/*
	(C) 2016 Gary Sinitsin. See LICENSE file (MIT license).
	Revised 2022 for SRI International research.

	InSys Intern: Amy Huang

*/
#include "Window.h"

namespace tincan {


Window* Window::sWindow = NULL;

void Window::registerClass()
{
	HINSTANCE hInstance = GetModuleHandle(NULL);

	WNDCLASSEX wc;
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = 0;
	wc.lpfnWndProc   = &Window::WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_3DFACE+1);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = L"tincanphoneWnd";
	wc.hIcon         = LoadIcon(hInstance, IDI_APPLICATION);
	wc.hIconSm       = 0;

	if (!RegisterClassEx(&wc))
		throw std::runtime_error("RegisterClassEx failed");
}

Window::Window(Phone* phone, int nCmdShow)
: phone(phone),
  handle(NULL),
  width(WIN_MIN_W),
  height(WIN_MIN_H),
  hlog(NULL),
  haddr(NULL)
{
	if (sWindow)
		throw std::runtime_error("Only one Window can be created");
	sWindow = this;

	CreateWindowEx(
		0,
		L"tincanphoneWnd",
		L"Tin Can Phone",
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME,
		GetSystemMetrics(SM_CXSCREEN)/2 - WIN_MIN_W/2, GetSystemMetrics(SM_CYSCREEN)/2 - WIN_MIN_H/2,
		WIN_MIN_W, WIN_MIN_H,
		NULL, NULL, GetModuleHandle(NULL), NULL);

	if (!handle)
		throw std::runtime_error("Could not create window");

	ShowWindow(handle, nCmdShow);
}

Window::~Window()
{
	phone->setUpdateHandler(NULL);
	destroy();
	sWindow = NULL;
}

LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Catch exceptions; they can't be thrown out of WndProc
	try
	{
		switch (msg)
		{
		case WM_CREATE:
			sWindow->handle = hWnd;
			sWindow->onCreate();
			return 0;

		case WM_SIZE:
			// Don't handle message if minimized
			if (wParam == SIZE_MINIMIZED)
				return DefWindowProc(hWnd, msg, wParam, lParam);
			sWindow->width  = LOWORD(lParam);
			sWindow->height = HIWORD(lParam);
			sWindow->onSize();
			// Fixes graphical glitches that happen when resizing
			//InvalidateRect(hWnd, NULL, TRUE);
			return 0;

		case WM_GETMINMAXINFO:
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = WIN_MIN_W;
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = WIN_MIN_H;
			return 0;

		case WM_PHONE_UPDATE:
			sWindow->onUpdate();
			return 0;

		case WM_COMMAND:
			sWindow->onCommand(LOWORD(wParam));
			return 0;

		case WM_CLOSE:
			sWindow->destroy();
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		}
	}
	catch (std::exception& ex)
	{
		errorMessage(ex.what());
		sWindow->destroy();
	}
	catch (...)
	{
		errorMessage("Unknown exception");
		sWindow->destroy();
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK Window::LogWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR)
{
	// Block all keyboard input
	if (msg == WM_KEYDOWN || msg == WM_CHAR || msg == WM_KEYUP)
		return 0;
	else if (msg == WM_NCDESTROY)
		RemoveWindowSubclass(hWnd, LogWndProc, uIdSubclass);
	
	return DefSubclassProc(hWnd, msg, wParam, lParam);
}

void Window::onCreate()
{
	SetTimer(handle, LOG_TIMER_ID, LOG_UPDATE_MS, &LogUpdateTimerProc);

	
	HINSTANCE hInstance = GetModuleHandle(NULL);

	int winh = WIN_MIN_H - 25;

	// Create log control
	hlog = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL,
		WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
		0, 0, WIN_MIN_W, WIN_MIN_H,
		handle, (HMENU)0, hInstance, NULL);
	setupControl(hlog);
	SendMessage(hlog, EM_SETMARGINS, EC_LEFTMARGIN, MARGIN);

	// Subclassing is the simplest way to get the behavior we want - no gray background or weird key/dialog msg handling
	SetWindowSubclass(hlog, &LogWndProc, 0, 0);

	/* MODIFIED: The controls will be created by for loops*/
	
	HWND hctl;

	wchar_t label_names [][23] =
	{
		L"IP address to call:", 	L"Set Bitrate (bits/s):",	L"Set Complexity (0-10):",  
		L"Set Bandwidth:", 			L"Toggle Console Debug:"
	};

	HMENU labels [] = 
	{
		(HMENU)IDC_ADDR_LABEL, 		(HMENU)BITRATE_LABEL, 		(HMENU)COMPLEX_LABEL,  		
		(HMENU)BANDWTH_LABEL, 		(HMENU)DEBUG_LABEL 
	};

	wchar_t button_names [][10] =
	{
		L"Call", 	L"Answer",		L"Set",		L"Set",
		L"4 kHz", 	L"6 kHz", 		L"8 kHz", 	L"12 kHz", 
		L"20 hHz",	L"DEBUG OFF"
	};

	HMENU commands [] = 
	{
		(HMENU)IDC_CALL, 	(HMENU)IDC_ANSWER_HANGUP,	(HMENU)IDC_BITRATE,	
		(HMENU)IDC_COMPLEX, (HMENU)IDC_NARROW, 			(HMENU)IDC_MEDIUM, 
		(HMENU)IDC_WIDE, 	(HMENU)IDC_SUPERWIDE,		(HMENU)IDC_FULL,	
		(HMENU)IDC_DEBUG
	};

	// for-loop to make labels
	for (int i = 0; i < 5; i++)
	{
		hctl = CreateWindow(L"STATIC", label_names[i], WS_VISIBLE | WS_CHILD | SS_RIGHT,
			0, 0, LABEL_W, TEXT_H,
			handle, labels[i], hInstance, NULL);
		setupControl(hctl);
	}

	// for-loop to make buttons
	for (int i = 0; i < 10; i++)
	{
		hctl = CreateWindow(L"BUTTON", button_names[i], WS_VISIBLE | WS_CHILD | WS_TABSTOP,
			0, 0, BUTTON_W, BUTTON_H,
			handle, commands[i], hInstance, NULL);
		setupControl(hctl);
	}

	// IP Address
	haddr = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_DISABLED,
		0, 0, ADDR_W, TEXT_H,
		handle, (HMENU)IDC_ADDR, hInstance, NULL);
	setupControl(haddr);
	SetWindowText(GetDlgItem(handle, IDC_ADDR), L"127.0.0.1");


	// Bitrate
	hbitrate = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_VISIBLE | WS_CHILD | WS_TABSTOP,
		0, 0, ADDR_W, TEXT_H,
		handle, (HMENU)BITRATE_TXT, hInstance, NULL);
	setupControl(hbitrate);
	SetWindowText(GetDlgItem(handle, BITRATE_TXT), L"8000");

	// Complexity (Might become an up-down button)
	hcomplex = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_VISIBLE | WS_CHILD | WS_TABSTOP,
		0, 0, ADDR_W, TEXT_H,
		handle, (HMENU)COMPLEX_TXT, hInstance, NULL);
	setupControl(hcomplex);
	SetWindowText(GetDlgItem(handle, COMPLEX_TXT), L"5");

	onSize();
}

void Window::onSize()
{
	// Fit log in window, leaving space at the bottom for other controls
	SetWindowPos(hlog, NULL,  2, 2,  width-2, height-MARGIN-(10 * BUTTON_H)-MARGIN, SWP_NOZORDER);

	// Keep bottom controls at bottom
	const uint nosize = SWP_NOSIZE|SWP_NOZORDER;
	HWND hctl;
	hctl = GetDlgItem(handle, IDC_ADDR_LABEL);
	SetWindowPos(hctl, NULL,  MARGIN, (height-ADDR_ROW), 0, 0, nosize);

	hctl = GetDlgItem(handle, IDC_ADDR);
	SetWindowPos(hctl, NULL,  MARGIN+LABEL_W+SPACE, (height-ADDR_ROW), 0, 0, nosize);

	hctl = GetDlgItem(handle, IDC_CALL);
	SetWindowPos(hctl, NULL,  MARGIN+LABEL_W+SPACE+ADDR_W+SPACE, height-ADDR_ROW - 3, 0, 0, nosize);

	hctl = GetDlgItem(handle, IDC_ANSWER_HANGUP);
	SetWindowPos(hctl, NULL,  MARGIN+LABEL_W+SPACE+ADDR_W+SPACE+BUTTON_W+MARGIN, height-ADDR_ROW - 3, 0, 0, nosize);

	/* BEGIN NEW */

	// Bitrate
	hctl = GetDlgItem(handle, BITRATE_LABEL);
	SetWindowPos(hctl, NULL,  MARGIN, (height - BITRATE_ROW), 0, 0, nosize);

	hctl = GetDlgItem(handle, BITRATE_TXT);
	SetWindowPos(hctl, NULL, MARGIN + LABEL_W + SPACE, (height - BITRATE_ROW), 0, 0, nosize);

	hctl = GetDlgItem(handle, IDC_BITRATE);
	SetWindowPos(hctl, NULL,  MARGIN+LABEL_W+SPACE+ADDR_W+SPACE, height-BITRATE_ROW - 3, 0, 0, nosize);

	// Complexity 
	hctl = GetDlgItem(handle, COMPLEX_LABEL);
	SetWindowPos(hctl, NULL, MARGIN, (height - COMPLEX_ROW), 0, 0, nosize);

	hctl = GetDlgItem(handle, COMPLEX_TXT);
	SetWindowPos(hctl, NULL, MARGIN + LABEL_W + SPACE, (height - COMPLEX_ROW), 0, 0, nosize);

	hctl = GetDlgItem(handle, IDC_COMPLEX);
	SetWindowPos(hctl, NULL,  MARGIN+LABEL_W+SPACE+ADDR_W+SPACE, height-COMPLEX_ROW - 3, 0, 0, nosize);

	// Bandwidth
	hctl = GetDlgItem(handle, BANDWTH_LABEL);
	SetWindowPos(hctl, NULL, MARGIN, (height - BANDWTH_ROW), 0, 0, nosize);

	int idc_bandwidths[5] = {(int)IDC_NARROW, (int)IDC_MEDIUM, (int)IDC_WIDE, (int)IDC_SUPERWIDE, (int)IDC_FULL};

	for(int i = 0; i < 5; i++)
	{
		hctl = GetDlgItem(handle, idc_bandwidths[i]);
		SetWindowPos(hctl, NULL,  MARGIN+LABEL_W+SPACE+(i * BUTTON_W)+SPACE, height-BANDWTH_ROW - 3, 0, 0, nosize);
	}
	

	// Debug
	hctl = GetDlgItem(handle, DEBUG_LABEL);
	SetWindowPos(hctl, NULL, MARGIN, (height - DEBUG_ROW), 0, 0, nosize);

	hctl = GetDlgItem(handle, IDC_DEBUG);
	SetWindowPos(hctl, NULL,  MARGIN+LABEL_W+SPACE+SPACE, height-DEBUG_ROW - 3, 0, 0, nosize);

}

void Window::onUpdate()
{
	switch (phone->getState())
	{
	case Phone::STARTING:
		break;

	case Phone::HUNGUP:
		EnableWindow(haddr, TRUE);
		SetFocus(haddr);
		EnableWindow(GetDlgItem(handle, IDC_CALL), TRUE);
		EnableWindow(GetDlgItem(handle, IDC_ANSWER_HANGUP), FALSE);
		break;
	
	case Phone::DIALING:
		EnableWindow(haddr, FALSE);
		EnableWindow(GetDlgItem(handle, IDC_CALL), FALSE);
		EnableWindow(GetDlgItem(handle, IDC_ANSWER_HANGUP), TRUE);
		SetWindowText(GetDlgItem(handle, IDC_ANSWER_HANGUP), L"Cancel call");
		SetFocus(GetDlgItem(handle, IDC_ANSWER_HANGUP));
		break;
	
	case Phone::RINGING:
		EnableWindow(haddr, FALSE);
		EnableWindow(GetDlgItem(handle, IDC_CALL), FALSE);
		EnableWindow(GetDlgItem(handle, IDC_ANSWER_HANGUP), TRUE);
		SetWindowText(GetDlgItem(handle, IDC_ANSWER_HANGUP), L"Answer");
		SetFocus(GetDlgItem(handle, IDC_ANSWER_HANGUP));
		break;
	
	case Phone::LIVE:
		EnableWindow(haddr, FALSE);
		EnableWindow(GetDlgItem(handle, IDC_CALL), FALSE);
		EnableWindow(GetDlgItem(handle, IDC_ANSWER_HANGUP), TRUE);
		SetWindowText(GetDlgItem(handle, IDC_ANSWER_HANGUP), L"Hang up");
		SetFocus(GetDlgItem(handle, IDC_ANSWER_HANGUP));
		break;
	
	case Phone::EXCEPTION:
		EnableWindow(haddr, FALSE);
		EnableWindow(GetDlgItem(handle, IDC_CALL), FALSE);
		EnableWindow(GetDlgItem(handle, IDC_ANSWER_HANGUP), FALSE);
		errorMessage(phone->getErrorMessage());
		destroy();
		return;
	
	case Phone::EXITED:
		return;
	}

	updateLog();
}

void Window::updateLog()
{
	// Pull log messages out of Phone
	string logs = phone->readLog();
	if (!logs.empty())
	{
		// Replace \n with \r\n
		size_t pos = 0;
		while ((pos = logs.find('\n', pos)) != string::npos) {
			logs.replace(pos, 1, "\r\n");
			pos += 2;
		}

		// Remove old logs if necessary
		size_t newloglen = logs.size() + (size_t)GetWindowTextLengthA(hlog);
		if (newloglen > LOG_MAX_SIZE)
		{
			SendMessageA(hlog, EM_SETSEL, 0, newloglen - LOG_MAX_SIZE);
			SendMessageA(hlog, EM_REPLACESEL, FALSE, (LPARAM)"");
		}

		// Append new logs
		SendMessageA(hlog, EM_SETSEL, LOG_MAX_SIZE, LOG_MAX_SIZE);
		SendMessageA(hlog, EM_REPLACESEL, FALSE, (LPARAM)logs.c_str());
	}
}

void Window::onCommand(const WORD id)
{
	if (id == IDC_CALL)
	{
		char addrText[256];
		GetWindowTextA(haddr, addrText, sizeof(addrText));
		phone->setCommand(Phone::CMD_CALL, addrText);
	}
	else if (id == IDC_ANSWER_HANGUP)
	{
		if (phone->getState() == Phone::RINGING)
			phone->setCommand(Phone::CMD_ANSWER);
		else
			phone->setCommand(Phone::CMD_HANGUP);
	}
	else if (id == IDC_BITRATE)
	{
		char bitrateText[256];
		GetWindowTextA(hbitrate, bitrateText, sizeof(bitrateText));
		phone->setBitrate(Phone::CMD_SETBITRATE, bitrateText);
	}
	else if (id == IDC_COMPLEX)
	{
		char complexText[256];
		GetWindowTextA(hcomplex, complexText, sizeof(complexText));
		phone->setComplexity(Phone::CMD_SETCOMPLEX, complexText);
	}
	else if (id == IDC_NARROW)	 { phone->setDebug(Phone::CMD_4K); }
	else if (id == IDC_MEDIUM)	 { phone->setDebug(Phone::CMD_6K); }
	else if (id == IDC_WIDE)  	 { phone->setDebug(Phone::CMD_8K); }
	else if (id == IDC_SUPERWIDE){ phone->setDebug(Phone::CMD_12K);}
	else if (id == IDC_FULL)	 { phone->setDebug(Phone::CMD_20K);}
	else if (id == IDC_DEBUG) 	 
	{ 
		phone->setDebug(Phone::CMD_DEBUG);

		if (!phone->getDebugStatus())
			SetWindowText(GetDlgItem(handle, IDC_DEBUG), L"DEBUG ON");
		else 
			SetWindowText(GetDlgItem(handle, IDC_DEBUG), L"DEBUG OFF");
	}


}

void Window::destroy()
{
	if (handle)
	{
		KillTimer(handle, LOG_TIMER_ID);
		DestroyWindow(handle);
		handle = NULL;
	}
}


}

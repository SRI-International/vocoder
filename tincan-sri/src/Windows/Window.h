/*
	(C) 2016 Gary Sinitsin. See LICENSE file (MIT license).
	Revised 2022 for SRI International research.

	InSys Intern: Amy Huang
*/
#pragma once

#include "../Phone.h"

#define UNICODE
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <CommCtrl.h> //For SetWindowSubclass
#include "Resource.h"
namespace tincan {


class Window : public UpdateHandler
{
public:
	static void errorMessage(const string& message)
	{
		HWND hWnd = (sWindow) ? sWindow->handle : NULL;
		MessageBoxA(hWnd, message.c_str(), "Tin Can Phone Error", MB_OK | MB_ICONERROR);
	}

	static void registerClass();

	HWND getHandle() const  {return handle;}
	
	Window(Phone* phone, int nCmdShow);
	~Window();
	
protected:
	enum {
		WM_PHONE_UPDATE   = WM_APP+101,
		IDC_ADDR_LABEL    = 101,
		IDC_ADDR          = 102,
		IDC_CALL          = IDOK,
		IDC_ANSWER_HANGUP = 200,

		/* BEGIN NEW */

		BITRATE_LABEL = 103,
		COMPLEX_LABEL = 104,
		BANDWTH_LABEL = 105,
		DEBUG_LABEL	  = 106, 

		BITRATE_TXT	  = 111,
		COMPLEX_TXT	  = 112,

		IDC_BITRATE	  = 121,
		IDC_COMPLEX	  = 122,

		IDC_NARROW	  = 123,
		IDC_MEDIUM	  = 124,
		IDC_WIDE 	  = 125,
		IDC_SUPERWIDE = 126,
		IDC_FULL	  = 127,

		IDC_DEBUG	  = 128,

		
		/* END NEW */

		LOG_TIMER_ID   = 1,
		LOG_UPDATE_MS  = 150,  //How often to pull log messages out of the Phone thread
		LOG_MAX_SIZE   = 2000, //How many characters to keep in the log text box

		WIN_MIN_W    = 600,
		WIN_MIN_H    = 500,
		MARGIN       = 10,
		SPACE        = 5,
		BUTTON_W     = 75,
		BUTTON_H     = 25,
		TEXT_H       = 20,
		LABEL_W		 = 150,
		ADDR_W       = 160,
		
		//ROWS FOR CONTROLS (subtracted from the height)

		ADDR_ROW	= (10 * BUTTON_H) - MARGIN,	// address, call, answer etc.
		BITRATE_ROW = (8 * BUTTON_H) - MARGIN,	// set bitrate 
		COMPLEX_ROW = (6 * BUTTON_H) - MARGIN,  // set complexity
		BANDWTH_ROW = (4 * BUTTON_H) - MARGIN,	// set bandwidth
		DEBUG_ROW	= (2 * BUTTON_H) - MARGIN	// turn on debug console and toggle mute

	};

	static Window* sWindow;
	Phone* phone;
	HWND   handle;
	int    width;
	int    height;
	HWND   hlog;
	HWND   haddr;

	/* BEGIN NEW */
	HWND   hbitrate;
	HWND   hcomplex;
	HWND   hbandwth;
	HWND   hdebug;

	// Implement UpdateHandler
	void sendUpdate()
	{
		if (handle)
			PostMessage(handle, WM_PHONE_UPDATE, 0, 0);
	}

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	static void CALLBACK LogUpdateTimerProc(HWND hWnd, UINT msg, UINT_PTR idEvent, DWORD time)
	{
		assert(sWindow);
		sWindow->updateLog();
	}

	static LRESULT CALLBACK LogWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR);

	void onCreate();

	void onSize();

	void onUpdate();

	void updateLog();

	void onCommand(const WORD id);

	void destroy();

	void setupControl(HWND hctl)
	{
		if (!hctl)
			throw std::runtime_error("Could not create control");

		static HFONT hfDefault = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		SendMessage(hctl, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
	}
};


}

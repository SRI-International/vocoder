/*
	(C) 2016 Gary Sinitsin. See LICENSE file (MIT license).
	Revised 2022 for SRI International research.
*/
#pragma once

#include "PhoneCommon.h"
#include "Mutex.h"
#include "Router.h"
#include "Socket.h"
#include <deque>
#include <iostream>
#include <string>
#include <opus/opus.h>
#include <portaudio.h>

namespace tincan {


enum Constants {
	PORT_DEFAULT         = 56780,
	PORT_MAX             = 56789,
	CHANNELS             = 1,    //1 channel (mono) audio
	SAMPLE_RATE          = 8000, //The number of 16-bit samples per second
	PACKET_MS            = 20,   //How long a single packet of samples is (20ms recommended by Opus)
	PACKET_SAMPLES       = 160,  //Samples per packet (SAMPLE_RATE * PACKET_MS = samples) 8000 * 0.020 = 160
	ENCODED_MAX_BYTES    = 24,   //Max size of a single packet's data once compressed (capacity of opus_encode buffer) 128, 64, 32
	BUFFERED_PACKETS_MIN = 2,    //How many packets to build up before we start playing audio
	BUFFERED_PACKETS_MAX = 5,    //When too many packets have built up and we start skipping them to speed up playback
	DISCONNNECT_TIMEOUT  = 5000, //How long to wait for valid AUDIO packets before we time out and disconnect
	RING_PACKET_INTERVAL = 500,  //How often to repeat RING packet
	UPNP_TIMEOUT_MS      = 8000  //Timeout to use when doing UPnP discovery
};


class UpdateHandler
{
public:
	// This method should send a message to the GUI thread (it is called in the Phone thread)
	virtual void sendUpdate() = 0;
	virtual ~UpdateHandler()  {}
};


class Phone
{
public:
	enum State { STARTING, HUNGUP, DIALING, RINGING, LIVE, EXITED, EXCEPTION };

	enum Command {
		CMD_NONE,
		CMD_CALL,   //Send outgoing call to addressIn when HUNGUP or RINGING
		CMD_ANSWER, //Answer incoming call when RINGING
		CMD_HANGUP, //End call when LIVE or DIALING
		CMD_EXIT,    //Exit the phone
		// NEW: Commnd to set the bitrte of hte program
		CMD_SETBITRATE,
		CMD_SETCOMPLEX,
		CMD_DEBUG,
		// Passband button commands
		CMD_4K,
		CMD_6K,
		CMD_8K,
		CMD_12K,
		CMD_20K
	};

	// These methods should only be called by the user thread, note that they lock Phone.mutex
	Phone::State getState() const
	{
		Scopelock lock(mutex);
		return stateOut;
	}
	
	void setCommand(Command cmd, const string& addr = "")
	{
		Scopelock lock(mutex);
		commandIn = cmd;
		addressIn = addr;
	}

	void setBitrate(Command cmd, const string& bitrate_str = "") 
	{
		Scopelock lock(mutex);
		commandIn = cmd;
		bitrateIn = bitrate_str;
	}

	void setComplexity(Command cmd, const string& bitrate_str = "")
	{
		Scopelock lock(mutex);
		commandIn = cmd;
		complexityIn = bitrate_str;
	}

	void setDebug(Command cmd)
	{
		Scopelock lock(mutex);
		commandIn = cmd;
		//consoleDebugActive = !consoleDebugActive;
	}
	
	string readLog()
	{
		Scopelock lock(mutex);
		string copy = logOut;
		logOut.clear();
		return copy;
	}
	
	string getErrorMessage() const
	{
		Scopelock lock(mutex);
		return errorMessage;
	}

	// This loop runs in its own thread
	int mainLoop() throw();

	// These are called before/after the Phone.mainLoop thread runs
	void setUpdateHandler(UpdateHandler* handler)  {updateHandler = handler;}

	bool getDebugStatus()
	{
		return consoleDebugActive;
	}
	
	Phone();
	~Phone();

protected:
	mutable Mutex mutex;

	// Must lock mutex before using these members!
	Command      commandIn;
	string       addressIn;
	State        stateOut;
	string       logOut;
	string       errorMessage;

	// New Gtk input variables
	string       bitrateIn;
	string       complexityIn;
	bool         consoleDebugActive = false;
	bool         isMuted = false;
	// END NEW

	// The rest do not have public accessors so no mutex requirement

	UpdateHandler*     updateHandler;
	std::ostringstream log;
	State              state;
	sockaddr_storage   address;

	struct Packet
	{
		enum Header { RING = 4000, BUSY, AUDIO, HANGUP }; // What kind of packet is it?
		uint32 header; // Header is one of 4 possible things.
		uint32 seq;    //AUDIO packet sequence number, 32 bits is enough for ~1000 days of 20ms packets
		byte   data[ENCODED_MAX_BYTES]; //AUDIO packet payload
	};

	struct AudioPacket
	{
		uint32 seq;
		byte   data[ENCODED_MAX_BYTES];
		uint   datasize;
		AudioPacket() : datasize(0)  {}
		bool operator == (uint32 rhs)  {return seq == rhs;} //For std::find
	};

	typedef std::deque<AudioPacket> AudioBuffer;
	AudioBuffer  audiobuf;
	uint32       sendseq;

	uint         ringToneTimer;
	uint         ringPacketTimer;
	uint         disconnectTimer;
	bool         increaseBuffering;
	uint         missedPackets;

	Router*      router;
	SOCKET       sock;
	OpusEncoder* encoder;
	OpusDecoder* decoder;
	PaStream*    stream;

	// Move parameters here so they can vary with input.
	opus_int32   bitrate;
	int          complexity;
	int          max_bandwidth;

	opus_int16   silence[PACKET_SAMPLES];
	opus_int16   ringToneIn[PACKET_SAMPLES];
	opus_int16   ringToneOut[PACKET_SAMPLES];

	void startup();
	/**
	 * Runs once the Phone program starts up.
	 * 
	 * Sets default values for the Opus encoder:
	 * - Bitrate    = 8 kbit/s
	 * - Complexity = 5
	 * - Bandwidth  = Fullband 20 kHz
	 */
	void start_settings();

	bool run();

	void hangup();
	void dial();
	void startRinging();
	void goLive();

	void receivePacket(const Packet& packet, uint packetSize, const sockaddr_storage& fromAddr);
	void bufferReceivedAudio(const Packet& packet, uint packetSize);

	void playReceivedAudio();
	void playRingtone();

	void sendPacket(Packet::Header header, const sockaddr_storage& to)
	{
		uint32 netheader = htonl(header);
		sendPacket((char*)&netheader, sizeof(netheader), to);
	}

	void sendPacket(char* buffer, int size, const sockaddr_storage& to);

	void beginAudioStream(bool input, bool output);
	void writeAudioStream(void* buffer, ulong samples);
	void endAudioStream();
};


}

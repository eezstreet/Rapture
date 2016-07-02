#include "sys_local.h"

/*
 * The Dispatch global object (ptDispatch) literally dispatches messages.
 * Each message sent to the dispatch system is logged to an external logfile named by the date and time the engine was started.
 * Each message is also given a priority.
 * The dispatch system has three bitmasks which do different behavior based on which type of messages come through:
 * 1. Any messages with priority which matches the Hidden bitmask are never logged nor acted upon.
 * 2. Any messages with priority which matches the Shutdown bitmask will cause the engine to shut down.
 * 3. Any messages with priority that matches the Message bitmask will cause a console message to appear.
 */

Dispatch* ptDispatch = nullptr;

// Initializes the dispatch system.
// This should be initialized first, so that we can catch any R_Messages that get sent.
Dispatch::Dispatch(const int _iHiddenMask, const int _iShutdownMask, const int _iMessageMask) :
iHiddenMask(_iHiddenMask),
iShutdownMask(_iShutdownMask),
iMessageMask(_iMessageMask),
ptLogFile(nullptr),
bSetup(false){
}

// Destroys the dispatch system.
Dispatch::~Dispatch() {
	if (!ptLogFile) {
		return;
	}
	File::CloseSync(ptLogFile);
	ptLogFile = nullptr;
}

// Since some messages may have been sent (but not properly dealt with), we should probably deal with those now.
// The setup function will simultaneously register the needed cvars and also dispatch any messages which are not handled yet.
void Dispatch::Setup() {
	Cvar* com_dispatchHiddenMask = Cvar::Get<int>("com_dispatchHiddenMask", "Hide messages with these priorities", (1 << CVAR_ARCHIVE), 
		(1 << PRIORITY_NONE) 
#ifndef _DEBUG 
		| (1 << PRIORITY_DEBUG)
#endif
		);
	Cvar* com_dispatchShutdownMask = Cvar::Get<int>("com_dispatchShutdownMask", "Shutdown when messages are dispatched with these priorities",
		(1 << CVAR_ARCHIVE), (1 << PRIORITY_ERRFATAL));
	Cvar* com_dispatchMessageMask = Cvar::Get<int>("com_dispatchMessageMask", "Show popup with these priorities", (1 << CVAR_ARCHIVE),
		(1 << PRIORITY_ERROR) | (1 << PRIORITY_ERRFATAL));

	com_dispatchHiddenMask->AddCallback((void*)Dispatch::ChangeHiddenMask);
	com_dispatchShutdownMask->AddCallback((void*)Dispatch::ChangeShutdownMask);
	com_dispatchMessageMask->AddCallback((void*)Dispatch::ChangeMessageMask);

	char fileName[MAX_HANDLE_STRING*2] = { 0 };
	time_t theTime = time(nullptr);

	strftime(fileName, sizeof(fileName), "logs/rapturelog_%Y-%m-%d_%H-%M-%S.log", localtime(&theTime));
	ptLogFile = File::OpenSync(fileName, "wb+");
	if(!ptLogFile) {
		return;
	}
	bSetup = true;

	for(auto it = vPreSetupMessages.begin(); it != vPreSetupMessages.end(); ++it) {
		PrintMessage(it->first, it->second.c_str());
	}
}

// This (dumbly-named) function closes the logfile.
void Dispatch::CatchError() {
	File::CloseSync(ptLogFile);
	ptLogFile = nullptr;
}

// This (dumbly-named) function dispatches a message with specified priority.
void Dispatch::PrintMessage(const int iPriority, const char* message) {
	static bool bLastHadNewline = true;
	bool bThisHasNewline = false;
	if(!bSetup) {
		vPreSetupMessages.push_back(make_pair(iPriority, message));
	}

	if(iPriority < 0 && iPriority >= PRIORITY_MAX) {
		return;
	}

	if(iHiddenMask & (1 << iPriority) && iPriority != PRIORITY_ERRFATAL) {
		// Fatal errors are NEVER hidden
		return;
	}

	if(ptLogFile != nullptr) {
		stringstream input;
		string in = message;
#ifdef WIN32
		size_t pos = 0;
		while((pos = in.find('\n', pos)) != string::npos) {
			in.replace(pos, 1, "\r\n");
			pos += 2;
			bThisHasNewline = true;
		}
#endif

		if(bLastHadNewline) {
			// Insert the time
			auto ticks = SDL_GetTicks();
			input << setfill('0') << setw(2) << floor(ticks / 3600000) << ":"; // Hours
			input << setfill('0') << setw(2) << floor(ticks / 60000) << ":"; // Minutes
			input << setfill('0') << setw(2) << floor(ticks / 1000) << " "; // Seconds

			// Insert the message type
			switch(iPriority) {
				default:
				case PRIORITY_NOTE:
					input << "[NOTE]\t\t";
					break;
				case PRIORITY_DEBUG:
					input << "[DEBUG]\t\t";
					break;
				case PRIORITY_MESSAGE:
					input << "[MESSAGE]\t";
					break;
				case PRIORITY_WARNING:
					input << "[WARNING]\t";
					break;
				case PRIORITY_ERROR:
					input << "[ERROR]\t";
					break;
				case PRIORITY_ERRFATAL:
					input << "[FATAL]\t\t";
					break;
			}
		}

		// Lastly, insert the message itself
		input << in;

		// Now write to log
		bLastHadNewline = bThisHasNewline;

		File::WriteSync(ptLogFile, (void*)input.str().c_str(), input.str().length());
	} else {
		printf(message);
	}

#ifdef WIN32
	if(iMessageMask & (1 << iPriority)) {
		switch(iPriority) {
			default:
				break;
			case PRIORITY_NOTE:
				MessageBox(nullptr, message, "Note", MB_OK | MB_SYSTEMMODAL | MB_SETFOREGROUND);
				break;
			case PRIORITY_DEBUG:
				MessageBox(nullptr, message, "Debug Message", MB_OK | MB_SYSTEMMODAL | MB_ICONINFORMATION | MB_SETFOREGROUND);
				break;
			case PRIORITY_MESSAGE:
				MessageBox(nullptr, message, "Message", MB_OK |  MB_SYSTEMMODAL | MB_ICONINFORMATION | MB_SETFOREGROUND);
				break;
			case PRIORITY_WARNING:
				MessageBox(nullptr, message, "Warning", MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL | MB_SETFOREGROUND);
				break;
			case PRIORITY_ERROR:
				MessageBox(nullptr, message, "Error", MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL | MB_SETFOREGROUND);
				break;
			case PRIORITY_ERRFATAL:
				MessageBox(nullptr, message, "Fatal Error", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL | MB_SETFOREGROUND);
				break;
		}
	}
#endif

	if(iShutdownMask & (1 << iPriority) || iPriority == PRIORITY_ERRFATAL) {
		// PRIORITY_ERRFATAL always shuts down the game
		// We might get multiple successive ERRFATALs in one frame, so we don't want to close the logfile just yet.
		setGameQuitting(true);
	}
}

// Changes the hidden bitmask
void Dispatch::ChangeHiddenMask(int newValue) {
	if(!ptDispatch) {
		return;
	}
	ptDispatch->iHiddenMask = newValue;
}

// Changes the shutdown bitmask
void Dispatch::ChangeShutdownMask(int newValue) {
	if(!ptDispatch) {
		return;
	}
	ptDispatch->iShutdownMask = newValue;
}

// Changes the message mask
void Dispatch::ChangeMessageMask(int newValue) {
	if(!ptDispatch) {
		return;
	}
	ptDispatch->iMessageMask = newValue;
}
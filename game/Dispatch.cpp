#include "sys_local.h"

Dispatch* ptDispatch = nullptr;

Dispatch::Dispatch(const int _iHiddenMask, const int _iShutdownMask, const int _iMessageMask) :
iHiddenMask(_iHiddenMask),
iShutdownMask(_iShutdownMask),
iMessageMask(_iMessageMask),
ptLogFile(nullptr),
bSetup(false){
}

Dispatch::~Dispatch() {
	if (!ptLogFile) {
		return;
	}
	ptLogFile->Close();
	ptLogFile = nullptr;
}

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
	ptLogFile = File::Open(fileName, "wb+");
	if(!ptLogFile) {
		return;
	}
	bSetup = true;

	for(auto it = vPreSetupMessages.begin(); it != vPreSetupMessages.end(); ++it) {
		PrintMessage(it->first, it->second.c_str());
	}
}

void Dispatch::CatchError() {
	ptLogFile->Close();
	ptLogFile = nullptr;
}

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
		ptLogFile->WritePlaintext(input.str());
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
		ptLogFile->Close();
		setGameQuitting(true);
		ptLogFile = nullptr;
	}
}

void Dispatch::ChangeHiddenMask(int newValue) {
	if(!ptDispatch) {
		return;
	}
	ptDispatch->iHiddenMask = newValue;
}

void Dispatch::ChangeShutdownMask(int newValue) {
	if(!ptDispatch) {
		return;
	}
	ptDispatch->iShutdownMask = newValue;
}

void Dispatch::ChangeMessageMask(int newValue) {
	if(!ptDispatch) {
		return;
	}
	ptDispatch->iMessageMask = newValue;
}
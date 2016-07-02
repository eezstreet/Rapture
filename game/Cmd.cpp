#include "sys_local.h"

template<typename T>
bool isType(string& s) {
	istringstream iss(s);
	T dummy;
	iss >> noskipws >> dummy;
	return iss.eof() && !iss.fail();
}

template<>
bool isType<bool>(string& s) {
	istringstream iss(s);
	bool dummy;
	iss >> boolalpha >> dummy;
	if(!dummy)
		iss >> dummy;
	return dummy;
}

static bool arch = false;

void Cmd_Set_f(vector<string>& args) {
	if(args.size() < 3)
		return;

	if(!Cvar::Exists(args[1]))
	{
		CvarSystem::CacheCvar(args[1], args[2]);
		return;
	}

	Cvar::cvarType_e eType = CvarSystem::GetCvarType(args[1]);
	switch(eType) {
		default:
		case Cvar::CV_STRING:
			CvarSystem::SetStringValue(args[1], (char*)args[2].c_str());
			break;
		case Cvar::CV_INTEGER:
			CvarSystem::SetIntegerValue(args[1], atoi(args[2].c_str()));
			break;
		case Cvar::CV_FLOAT:
			CvarSystem::SetFloatValue(args[1], (float)atof(args[2].c_str()));
			break;
		case Cvar::CV_BOOLEAN:
			CvarSystem::SetBooleanValue(args[1], atob(args[2].c_str()));
			break;
	}
}

void Cmd_Seta_f(vector<string>& args) {
	if(args.size() < 3)
		return;

	if(!Cvar::Exists(args[1])) {
		CvarSystem::CacheCvar(args[1], args[2], true);
		return;
	}

	Cmd_Set_f(args);
	int flags = CvarSystem::GetCvarFlags(args[1]);
	flags = flags | (1 << CVAR_ARCHIVE);
	CvarSystem::SetCvarFlags(args[1], flags);
}

void Cmd_Exec_f(vector<string>& args) {
	// Callbacks aren't acceptable here, we need to run their logic in the same thread.
	if(args.size() < 2) {
		R_Message(PRIORITY_MESSAGE, "usage: exec <filename.cfg>\n");
		return;
	}

	File* p = File::OpenSync(args[1].c_str());
	if (p == nullptr) {
		R_Message(PRIORITY_WARNING, "could not exec %s\n", args[1].c_str());
		return;
	}
	R_Message(PRIORITY_MESSAGE, "executing %s\n", args[1].c_str());

	char fileBuffer[32968];
	File::ReadSync(p, fileBuffer, sizeof(fileBuffer));
	File::CloseSync(p);
	
	string text = fileBuffer;
	if(text.length() > 0) {
		vector<string> lines;
		split(text, ';', lines);
		if(lines.size() > 0) {
			for(auto it = lines.begin(); it != lines.end(); ++it) {
				Cmd::ProcessCommand((*it).c_str());
			}
		}
	}
}

void Cmd_Quit_f(vector<string>& args) {
	SDL_Event e;
	e.type = SDL_QUIT;
	SDL_PushEvent(&e);
}

extern void ReturnToMain();
void Cmd_MainMenu_f(vector<string>& args) {
	ReturnToMain();
}

void Cmd_Cmdlist_f(vector<string>& args) {
	Cmd::ListCommands();
}

void Cmd_Cvarlist_f(vector<string>& args) {
	CvarSystem::ListCvars();
}

void Cmd_Zoneinfo_f(vector<string>& args) {
	Zone::MemoryUsage();
}

void Cmd_Screenshot_f(vector<string>& args) {
	if(args.size() >= 2) {
		Video::QueueScreenshot(args[1].c_str(), ".bmp");
	}
	else {
		Video::QueueScreenshot("", ".bmp");
	}
}

void Cmd_ScreenshotJPEG_f(vector<string>& args) {
	if(args.size() >= 2) {
		Video::QueueScreenshot(args[1].c_str(), ".jpg");
	}
	else {
		Video::QueueScreenshot("", ".jpg");
	}
}

void Cmd_ScreenshotPCX_f(vector<string>& args) {
	if(args.size() >= 2) {
		Video::QueueScreenshot(args[1].c_str(), ".pcx");
	}
	else {
		Video::QueueScreenshot("", ".pcx");
	}
}

void Cmd_Echo_f(vector<string>& args) {
	string text = "";
	for(auto it = args.begin()+1; it != args.end(); ++it) {
		text += *it;
		text += " ";
	}
	R_Message(PRIORITY_MESSAGE, "%s\n", text.c_str());
}

void Cmd_VidRestart_f(vector<string>& args) {
	Video::Restart();
}

extern bool bVMInputBlocked;
void Cmd_BlockVMInput_f(vector<string>& args) {
	bVMInputBlocked = !bVMInputBlocked;
}

extern void NewGame();
extern void StartEditor();
void Cmd_NewGameTest_f(vector<string>& args) {
	NewGame();
}

void Cmd_Editor_f(vector<string>& args) {
	StartEditor();
}

void Sys_InitCommands() {
	// Register commands from the engine
	Cmd::AddCommand("set", Cmd_Set_f);
	Cmd::AddCommand("seta", Cmd_Seta_f);
	Cmd::AddCommand("exec", Cmd_Exec_f);
	Cmd::AddCommand("quit", Cmd_Quit_f);
	Cmd::AddCommand("cmdlist", Cmd_Cmdlist_f);
	Cmd::AddCommand("cvarlist", Cmd_Cvarlist_f);
	Cmd::AddCommand("zoneinfo", Cmd_Zoneinfo_f);
	Cmd::AddCommand("echo", Cmd_Echo_f);
	Cmd::AddCommand("blockvminput", Cmd_BlockVMInput_f);

	Cmd::AddCommand("vid_restart", Cmd_VidRestart_f);
	Cmd::AddCommand("video_restart", Cmd_VidRestart_f);

	Cmd::AddCommand("screenshot", Cmd_Screenshot_f);
	Cmd::AddCommand("screenshotBMP", Cmd_Screenshot_f);
	Cmd::AddCommand("screenshotJPG", Cmd_ScreenshotJPEG_f);
	Cmd::AddCommand("screenshotJPEG", Cmd_ScreenshotJPEG_f);
	Cmd::AddCommand("screenshotPCX", Cmd_ScreenshotPCX_f);

	Cmd::AddCommand("newgametest", Cmd_NewGameTest_f);
	Cmd::AddCommand("editor", Cmd_Editor_f);
	Cmd::AddCommand("mainmenu", Cmd_MainMenu_f);
}
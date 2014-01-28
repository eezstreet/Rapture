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
			CvarSystem::SetFloatValue(args[1], atof(args[2].c_str()));
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
	flags = flags | Cvar::CVAR_ARCHIVE;
	CvarSystem::SetCvarFlags(args[1], flags);
}

void Cmd_Exec_f(vector<string>& args) {
	if(args.size() < 2) {
		R_Printf("usage: exec <filename.cfg>\n");
		return;
	}
	File* p = File::Open(args[1], "r");
	if(p == NULL) {
		R_Printf("could not exec %s\n", args[1].c_str());
		return;
	}
	R_Printf("executing %s\n", args[1].c_str());
	string text = p->ReadPlaintext();
	p->Close();
	Zone::FastFree(p, "files");

	vector<string> lines = split(text, ';');
	for(auto it = lines.begin(); it != lines.end(); ++it)
		Cmd::ProcessCommand((*it).c_str());
}

void Cmd_Quit_f(vector<string>& args) {
	SDL_Event e;
	e.type = SDL_QUIT;
	SDL_PushEvent(&e);
}

void Sys_InitCommands() {
	// Register commands from the engine
	Cmd::AddCommand("set", Cmd_Set_f);
	Cmd::AddCommand("seta", Cmd_Seta_f);
	Cmd::AddCommand("exec", Cmd_Exec_f);
	Cmd::AddCommand("quit", Cmd_Quit_f);
}
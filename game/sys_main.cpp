#include "sys_local.h"
#include <SDL.h>

/* main game initialization */

static RaptureGame *sys;

int main(int argc, char** argv) {
	sys = new RaptureGame(argc, argv);
	while(!sys->bHasFinished)
		sys->RunLoop();
	delete sys;
	return 0;
}

/* RaptureGame class, this does all the heavy lifting */
RaptureGame::RaptureGame(int argc, char **argv) : bHasFinished(false) {
	// init cmds
	Sys_InitCommands();

	// Init zone memory
	Zone::Init();

	// Init the cvar system (so we can assign preliminary cvars in the commandline)
	CvarSystem::Initialize();

	// Init filesystem
	FS::Init();

	// Read from the config
	Cmd::ProcessCommand("exec raptureconfig.cfg");

	// Actually read the arguments
	HandleCommandline(argc, argv);

	// Init hunk memory

	// Init the renderer
	RenderCode::Initialize();

	// Init rand (cuz we will DEFINITELY need it ;D)

	// Init UI
	//InitUI();
}

/* Called after the main loop has finished and we are ready to shut down */
RaptureGame::~RaptureGame() {
	RenderCode::Exit();
	FS::Shutdown();
	CvarSystem::Destroy();
	Zone::Shutdown();
}

/* Run every frame */
void RaptureGame::RunLoop() {
	SDL_PumpEvents();

	RenderCode::BlankFrame();
	RenderCode::Display();
}

/* Deal with the commandline arguments */
void RaptureGame::HandleCommandline(int argc, char** argv) {
	if(argc <= 1) return;
	string ss = "";
	for(int i = 1; i < argc; i++) {
		ss += argv[i];
	}
	vector<string> s = split(ss, '+');
	for(auto it = s.begin(); it != s.end(); ++it)
		Cmd::ProcessCommand(it->c_str());
}
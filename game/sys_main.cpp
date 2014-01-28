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

// This accounts for all input
int RaptureInputCallback(void *notUsed, SDL_Event* e) {
	switch(e->type) {
		case SDL_APP_TERMINATING:
		case SDL_QUIT:
			sys->PassQuitEvent();
			break;
		case SDL_KEYDOWN:
			Input->SendKeyDownEvent(e->key.keysym, e->text.text);
			break;
		case SDL_KEYUP:
			Input->SendKeyUpEvent(e->key.keysym, e->text.text);
			break;
		case SDL_MOUSEMOTION:
			Input->SendMouseMoveEvent(e->motion.x, e->motion.y);
			break;
		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEBUTTONDOWN:
			Input->SendMouseButtonEvent(e->button.button, e->button.state, e->button.x, e->button.y);
			break;
		case SDL_JOYAXISMOTION:
			break;
		case SDL_TEXTINPUT:
			Input->SendTextInputEvent(e->text.text);
			break;
		case SDL_TEXTEDITING:
			// Not used?
			break;
		case SDL_JOYBALLMOTION:
			break;
		case SDL_JOYHATMOTION:
			break;
		case SDL_JOYBUTTONDOWN:
			break;
		case SDL_JOYBUTTONUP:
			break;
		case SDL_CONTROLLERAXISMOTION:
			break;
		case SDL_CONTROLLERBUTTONDOWN:
			break;
		case SDL_CONTROLLERBUTTONUP:
			break;
	}
	return 1;
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

	// Init rand (cuz we will DEFINITELY need it ;D

	// Init input
	SDL_SetEventFilter(RaptureInputCallback, NULL);
	InitInput();

	// Init UI
	UI::Initialize();
}

/* Called after the main loop has finished and we are ready to shut down */
RaptureGame::~RaptureGame() {
	DeleteInput();
	RenderCode::Exit();
	CvarSystem::Destroy();
	FS::Shutdown();
	Zone::Shutdown();
}

/* Run every frame */
void RaptureGame::RunLoop() {
	// Do input
	Input->InputFrame();
	UI::Update();

	// Do rendering
	RenderCode::BlankFrame();
	UI::Render();
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

/* Quit the game. */
void RaptureGame::PassQuitEvent() {
	bHasFinished = true;
}

/* Some shared functions */
#include "ui_local.h"
void R_Printf(const char *fmt, ...) {
	va_list args;
	char str[1024];

	va_start(args, fmt);
	vsnprintf(str, 1024, fmt, args);
	va_end(args);

	Console::PushConsoleMessage(str);
}
#include "sys_local.h"

/* main game initialization */

static RaptureGame *sys = nullptr;
bool bStartupQuit = false;

void setGameQuitting(const bool b) { if(!sys) bStartupQuit = b; else sys->bHasFinished = true; }

int main(int argc, char** argv) {
	try {
		sys = new RaptureGame(argc, argv);
		FrameCapper fc;
		while(!sys->bHasFinished && !bStartupQuit) {
			fc.StartFrame();
			sys->RunLoop();
			fc.EndFrame();
		}
	}
	catch(const bool) {
		while((sys && !sys->bHasFinished) || (!bStartupQuit)) {
#ifdef _WIN32
			MSG        msg;
			if (!GetMessage (&msg, nullptr, 0, 0)) {
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
#endif
		}
	}
	if(sys) {
		delete sys;
	}
	return 0;
}

// This accounts for all input
int RaptureInputCallback(void *notUsed, SDL_Event* e) {
	if(sys->bHasFinished) {
		// DONT send input...
		return 1;
	}
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

// Print SDL version number
void Sys_PrintSDLVersion() {
	SDL_version ver;
	SDL_VERSION(&ver);

	R_Message(PRIORITY_NOTE, "using SDL %u.%u.%u\n", ver.major, ver.minor, ver.patch);
}

/* RaptureGame class, this does all the heavy lifting */
RaptureGame::RaptureGame(int argc, char **argv) : bHasFinished(false) {
	game = nullptr;
	editor = nullptr;

	ptDispatch = new Dispatch(0, 0, 0);

	Sys_PrintSDLVersion();

	// init cmds
	Sys_InitCommands();

	// Init zone memory
	Zone::Init();

	// Init the cvar system (so we can assign preliminary cvars in the commandline)
	CvarSystem::Initialize();

	// Init filesystem
	Filesystem::Init();

	// Setup the dispatch system
	ptDispatch->Setup();

	// Read from the config
	Cmd::ProcessCommand("exec raptureconfig.cfg");

	// Actually read the arguments
	HandleCommandline(argc, argv);

	// Init hunk memory

	// Init the renderer
	if (!Video::Init()) {
		return;
	}


	// Init rand (cuz we will DEFINITELY need it ;D

	// Init input
	SDL_SetEventFilter(RaptureInputCallback, nullptr);
	InitInput();

	// Init UI
	UI::Initialize();
}

/* Called after the main loop has finished and we are ready to shut down */
RaptureGame::~RaptureGame() {
	if(game) {
		trap->shutdown();
		delete game;
	}

	DeleteInput();
	CvarSystem::Destroy();
	delete ptDispatch;
	Filesystem::Exit();
	Zone::Shutdown();
	Video::Shutdown();
}

/* Run every frame */
void RaptureGame::RunLoop() {

	// Do input
	Input->InputFrame();
	UI::Update();

	Video::ClearFrame();

	// Do gamecode
	if(game || editor) {
		trap->runactiveframe();
	}

	// Do rendering
	UI::Render();
	Video::RenderFrame();
}

/* Deal with the commandline arguments */
void RaptureGame::HandleCommandline(int argc, char** argv) {
	if(argc <= 1) return;
	string ss = "";
	for(int i = 1; i < argc; i++) {
		ss += argv[i];
	}
	vector<string> s;
	split(ss, '+', s);
	for(auto it = s.begin(); it != s.end(); ++it) {
		Cmd::ProcessCommand(it->c_str());
	}
}

/* Quit the game. */
void RaptureGame::PassQuitEvent() {
	bHasFinished = true;
}

/* Set up common exports which are used in both the game and the editor */
extern bool IsConsoleOpen();
void RaptureGame::AssignExports(gameImports_s *imp) {
	imp->printf = R_Message;
	imp->error = R_Error;

	imp->GetTicks = reinterpret_cast<int(*)()>(SDL_GetTicks);

	imp->OpenFileSync = File::OpenSync;
	imp->ReadFileSync = File::ReadSync;
	imp->WriteFileSync = File::WriteSync;
	imp->CloseFileSync = File::CloseSync;
	imp->OpenFileAsync = File::OpenAsync;
	imp->ReadFileAsync = File::ReadAsync;
	imp->WriteFileAsync = File::WriteAsync;
	imp->CloseFileAsync = File::CloseAsync;
	imp->FileOpened = File::AsyncOpened;
	imp->FileRead = File::AsyncRead;
	imp->FileWritten = File::AsyncWritten;
	imp->FileClosed = File::AsyncClosed;
	imp->FileBad = File::AsyncBad;

	imp->ResourceAsync = Resource::ResourceAsync;
	imp->ResourceAsyncURI = Resource::ResourceAsyncURI;
	imp->ResourceSync = Resource::ResourceSync;
	imp->ResourceSyncURI = Resource::ResourceSyncURI;
	imp->FreeResource = Resource::FreeResource;
	imp->GetAssetComponent = Resource::GetAssetComponent;
	imp->ResourceRetrieved = Resource::ResourceRetrieved;
	imp->ResourceBad = Resource::ResourceBad;

	imp->RegisterMaterial = Video::RegisterMaterial;
	imp->DrawMaterial = Video::DrawMaterial;
	imp->DrawMaterialAspectCorrection = Video::DrawMaterialAspectCorrection;
	imp->DrawMaterialClipped = Video::DrawMaterialClipped;
	imp->DrawMaterialAbs = Video::DrawMaterialAbs;
	imp->DrawMaterialAbsClipped = Video::DrawMaterialAbsClipped;

	imp->RegisterStreamingTexture = Video::RegisterStreamingTexture;
	imp->LockStreamingTexture = Video::LockStreamingTexture;
	imp->UnlockStreamingTexture = Video::UnlockStreamingTexture;
	imp->DeleteStreamingTexture = Video::DeleteStreamingTexture;

	imp->CvarBoolVal = CvarSystem::EXPORT_BoolValue;
	imp->CvarIntVal = CvarSystem::EXPORT_IntValue;
	imp->CvarStrVal = CvarSystem::EXPORT_StrValue;
	imp->CvarValue = CvarSystem::EXPORT_Value;
	imp->RegisterCvarBool = static_cast<Cvar*(*)(const char*, const char*, int, bool)>(CvarSystem::RegisterCvar);
	imp->RegisterCvarFloat = static_cast<Cvar*(*)(const char*, const char*, int, float)>(CvarSystem::RegisterCvar);
	imp->RegisterCvarInt = static_cast<Cvar*(*)(const char*, const char*, int, int)>(CvarSystem::RegisterCvar);
	imp->RegisterCvarStr = static_cast<Cvar*(*)(const char*, const char*, int, char*)>(CvarSystem::RegisterCvar);
	imp->RegisterStaticMenu = UI::RegisterStaticMenu;
	imp->KillStaticMenu = UI::KillStaticMenu;

	imp->RunJavaScript = UI::RunJavaScript;
	imp->AddJSCallback = UI::AddJavaScriptCallback;
	imp->GetJSNumArgs = UI::GetJavaScriptNumArgs;
	imp->GetJSStringArg = UI::GetJavaScriptStringArgument;
	imp->GetJSIntArg = UI::GetJavaScriptIntArgument;
	imp->GetJSDoubleArg = UI::GetJavaScriptDoubleArgument;
	imp->GetJSBoolArg = UI::GetJavaScriptBoolArgument;

	imp->IsConsoleOpen = IsConsoleOpen;

	imp->Zone_Alloc = Zone::VMAlloc;
	imp->Zone_FastFree = Zone::VMFastFree;
	imp->Zone_Free = Zone::Free;
	imp->Zone_FreeAll = Zone::VMFreeAll;
	imp->Zone_NewTag = Zone::NewTag;
	imp->Zone_Realloc = Zone::Realloc;

	imp->FadeFromBlack = Video::FadeFromBlack;
}

/* Started a new game (probably from the menu) */
GameModule* RaptureGame::CreateGameModule(const char* bundle) {
	GameModule* game = new GameModule(bundle);
	static gameImports_s imp;
	AssignExports(&imp);
	
	trap = game->GetRefAPI(&imp);
	if(!trap) {
		return nullptr;
	}
	trap->init();
	return game;
}

/* Get the game module */
GameModule* RaptureGame::GetGameModule() {
	if(sys) {
		return sys->game;
	}
	return nullptr;
}

GameModule* RaptureGame::GetEditorModule() {
	if(sys) {
		return sys->editor;
	}
	return nullptr;
}

gameExports_s* RaptureGame::GetImport() {
	if(sys) {
		return sys->trap;
	}
	return nullptr;
}

/* Some shared functions */
#include "ui_local.h"
void R_Message(int iPriority, const char *fmt, ...) {
	va_list args;
	char str[1024];

	va_start(args, fmt);
	vsnprintf(str, 1024, fmt, args);
	va_end(args);

	ptDispatch->PrintMessage(iPriority, str);

	Console::PushConsoleMessage(str);
}

void NewGame() {
	MainMenu::DestroySingleton();
	if(Console::GetSingleton()->IsOpen()) {
		Console::GetSingleton()->Hide();
	}
	if(sys->game || sys->editor) {
		// Have to exit from these first
		return;
	}
	sys->game = sys->CreateGameModule("gamex86");
}

void StartEditor() {
	if(Console::GetSingleton()->IsOpen()) {
		Console::GetSingleton()->Hide();
	}
	MainMenu::DestroySingleton();
	if(sys->game || sys->editor) {
		// Have to exit from these first
		return;
	}
	sys->editor = sys->CreateGameModule("editorx86");
}

void ReturnToMain() {
	if(Console::GetSingleton()->IsOpen()) {
		Console::GetSingleton()->Hide();
	}
	if(sys->game) {
		sys->trap->shutdown();
		delete sys->game;
		sys->game = nullptr;
	} else if(sys->editor) {
		sys->trap->shutdown();
		delete sys->editor;
		sys->editor = nullptr;
	}
	R_Message(PRIORITY_MESSAGE, "creating main menu webview\n");
	MainMenu::GetSingleton();
}
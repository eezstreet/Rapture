#include "sys_local.h"
#include "ui_local.h"

// This accounts for all input
int RaptureGame::RaptureInputCallback(void *notUsed, SDL_Event* e) {
	RaptureGame* game = GetSingleton();
	if (!game || game->HasFlags(Rapture_FatalError) || game->HasFlags(Rapture_Quitting)) {
		// DONT send input...
		return 1;
	}
	switch(e->type) {
		case SDL_APP_TERMINATING:
		case SDL_QUIT:
			if (game) {
				game->uGameFlags |= (1 << Rapture_Quitting);
			}
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
RaptureGame::RaptureGame(int argc, char **argv) {
	game = nullptr;
	editor = nullptr;

	ptDispatch = new Dispatch(0, 0, 0);

	Sys_PrintSDLVersion();

	// init cmds
	Sys_InitCommands();

	// Init memory
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

	// Init the renderer
	if (!Video::Init()) {
		return;
	}

	// Init input
	SDL_SetEventFilter(RaptureInputCallback, nullptr);
	InitInput();

	// Init UI
	UI::Initialize();

	Network::Init();
}

/* Called after the main loop has finished and we are ready to shut down */
RaptureGame::~RaptureGame() {
	if(game) {
		trap->saveandexit();
		delete game;
	}
	if (editor) {
		trap->saveandexit();
		delete editor;
	}

	DeleteInput();
	CvarSystem::Destroy();
	delete ptDispatch;
	Filesystem::Exit();
	Zone::Shutdown();
	Video::Shutdown();
	Network::Shutdown();
}

/* Run every frame */
void RaptureGame::RunLoop() {

	// Do input
	Input->InputFrame();
	UI::Update();

	Video::ClearFrame();

	// Do gamecode
	Network::ServerFrame();
	Network::ClientFrame();

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

/* Started a new game (probably from the menu) */
GameModule* RaptureGame::CreateGameModule(const char* bundle) {
	GameModule* game = new GameModule(bundle);
	static gameImports_s imp;
	
	imp.printf = R_Message;
	imp.error = R_Error;

	imp.GetTicks = reinterpret_cast<int(*)()>(SDL_GetTicks);

	imp.OpenFileSync = File::OpenSync;
	imp.ReadFileSync = File::ReadSync;
	imp.WriteFileSync = File::WriteSync;
	imp.CloseFileSync = File::CloseSync;
	imp.OpenFileAsync = File::OpenAsync;
	imp.ReadFileAsync = File::ReadAsync;
	imp.WriteFileAsync = File::WriteAsync;
	imp.CloseFileAsync = File::CloseAsync;
	imp.FileOpened = File::AsyncOpened;
	imp.FileRead = File::AsyncRead;
	imp.FileWritten = File::AsyncWritten;
	imp.FileClosed = File::AsyncClosed;
	imp.FileBad = File::AsyncBad;

	imp.ResourceAsync = Resource::ResourceAsync;
	imp.ResourceAsyncURI = Resource::ResourceAsyncURI;
	imp.ResourceSync = Resource::ResourceSync;
	imp.ResourceSyncURI = Resource::ResourceSyncURI;
	imp.FreeResource = Resource::FreeResource;
	imp.GetAssetComponent = Resource::GetAssetComponent;
	imp.ResourceRetrieved = Resource::ResourceRetrieved;
	imp.ResourceBad = Resource::ResourceBad;

	imp.RegisterMaterial = Video::RegisterMaterial;
	imp.DrawMaterial = Video::DrawMaterial;
	imp.DrawMaterialAspectCorrection = Video::DrawMaterialAspectCorrection;
	imp.DrawMaterialClipped = Video::DrawMaterialClipped;
	imp.DrawMaterialAbs = Video::DrawMaterialAbs;
	imp.DrawMaterialAbsClipped = Video::DrawMaterialAbsClipped;

	imp.RegisterStreamingTexture = Video::RegisterStreamingTexture;
	imp.LockStreamingTexture = Video::LockStreamingTexture;
	imp.UnlockStreamingTexture = Video::UnlockStreamingTexture;
	imp.DeleteStreamingTexture = Video::DeleteStreamingTexture;
	imp.BlendTexture = Video::BlendTexture;

	imp.CvarBoolVal = CvarSystem::EXPORT_BoolValue;
	imp.CvarIntVal = CvarSystem::EXPORT_IntValue;
	imp.CvarStrVal = CvarSystem::EXPORT_StrValue;
	imp.CvarValue = CvarSystem::EXPORT_Value;
	imp.RegisterCvarBool = static_cast<Cvar*(*)(const char*, const char*, int, bool)>(CvarSystem::RegisterCvar);
	imp.RegisterCvarFloat = static_cast<Cvar*(*)(const char*, const char*, int, float)>(CvarSystem::RegisterCvar);
	imp.RegisterCvarInt = static_cast<Cvar*(*)(const char*, const char*, int, int)>(CvarSystem::RegisterCvar);
	imp.RegisterCvarStr = static_cast<Cvar*(*)(const char*, const char*, int, char*)>(CvarSystem::RegisterCvar);
	imp.RegisterStaticMenu = UI::RegisterStaticMenu;
	imp.KillStaticMenu = UI::KillStaticMenu;

	imp.SendServerPacket = Network::SendServerPacket;
	imp.SendClientPacket = Network::SendClientPacket;

	imp.RunJavaScript = UI::RunJavaScript;
	imp.AddJSCallback = UI::AddJavaScriptCallback;
	imp.GetJSNumArgs = UI::GetJavaScriptNumArgs;
	imp.GetJSStringArg = UI::GetJavaScriptStringArgument;
	imp.GetJSIntArg = UI::GetJavaScriptIntArgument;
	imp.GetJSDoubleArg = UI::GetJavaScriptDoubleArgument;
	imp.GetJSBoolArg = UI::GetJavaScriptBoolArgument;

	imp.IsConsoleOpen = UI::IsConsoleOpen;

	imp.Zone_Alloc = Zone::VMAlloc;
	imp.Zone_FastFree = Zone::VMFastFree;
	imp.Zone_Free = Zone::Free;
	imp.Zone_FreeAll = Zone::VMFreeAll;
	imp.Zone_NewTag = Zone::NewTag;
	imp.Zone_Realloc = Zone::Realloc;

	imp.FadeFromBlack = Video::FadeFromBlack;
	
	trap = game->GetRefAPI(&imp);
	if(!trap) {
		return nullptr;
	}
	return game;
}

/* Singleton ops */
RaptureGame* RaptureGame::singleton = nullptr;
bool noMoreSingleton = false;
RaptureGame* RaptureGame::GetSingleton(int argc, char** argv) {
	if (noMoreSingleton) {
		return nullptr;
	}
	if (singleton != nullptr) {
		return singleton;
	}
	singleton = new RaptureGame(argc, argv);
	return singleton;
}

void RaptureGame::DestroySingleton() {
	if (singleton != nullptr) {
		delete singleton;
	}
	noMoreSingleton = true;
}

/* Get the game module */
GameModule* RaptureGame::GetGameModule() {
	RaptureGame* singleton = RaptureGame::GetSingleton();
	if (singleton && singleton->HasFlags(Rapture_GameLoaded)) {
		return singleton->game;
	}
	return nullptr;
}

GameModule* RaptureGame::GetEditorModule() {
	RaptureGame* singleton = RaptureGame::GetSingleton();
	if (singleton && singleton->HasFlags(Rapture_EditorLoaded)) {
		return singleton->editor;
	}
	return nullptr;
}

gameExports_s* RaptureGame::GetImport() {
	RaptureGame* singleton = RaptureGame::GetSingleton();
	if (singleton) {
		if (singleton->HasFlags(Rapture_GameLoaded) || singleton->HasFlags(Rapture_EditorLoaded)) {
			return singleton->trap;
		}
	}
	return nullptr;
}

bool RaptureGame::AllowedToStartNewModules() {
	if (HasFlags(Rapture_EditorLoaded)) {
		R_Message(PRIORITY_MESSAGE, "Exit from the editor first before running this command.\n");
		return false;
	}

	if (HasFlags(Rapture_GameLoaded)) {
		R_Message(PRIORITY_MESSAGE, "Exit from the game first before running the command.\n");
		return false;
	}

	if (!HasFlags(Rapture_Initialized)) {
		R_Message(PRIORITY_MESSAGE, "The game needs to be fully initialized before this command can be run.\n");
		return false;
	}

	if (HasFlags(Rapture_Quitting) || HasFlags(Rapture_FatalError)) {
		R_Message(PRIORITY_MESSAGE, "An error occurred and the module could not be loaded.\n");
		return false;
	}
	return true;
}

void RaptureGame::StartGameFromFile(const char* szSaveGameName) {
	if (!AllowedToStartNewModules()) {
		return;
	}

	MainMenu::DestroySingleton();
	if (Console::GetSingleton()->IsOpen()) {
		Console::GetSingleton()->Hide();
	}

	game = CreateGameModule("gamex86");
	if (game == nullptr) {
		return;
	}
	Network::StartLocalServer();
	trap->startserverfromsave(szSaveGameName);
	trap->startclientfromsave(szSaveGameName);

	uGameFlags |= (1 << Rapture_GameLoaded);
}

void RaptureGame::JoinRemoteGame(const char* szFileName, const char* szHostName) {
	if (!AllowedToStartNewModules()) {
		return;
	}

	MainMenu::DestroySingleton();
	if (Console::GetSingleton()->IsOpen()) {
		Console::GetSingleton()->Hide();
	}

	game = CreateGameModule("gamex86");
	if (game == nullptr) {
		return;
	}

	if (!Network::JoinServer(szHostName)) {
		return;
	}
	trap->startclientfromsave(szFileName);

	uGameFlags |= (1 << Rapture_GameLoaded);
}

void RaptureGame::StartEditor() {
	if (!AllowedToStartNewModules()) {
		return;
	}

	if (Console::GetSingleton()->IsOpen()) {
		Console::GetSingleton()->Hide();
	}
	MainMenu::DestroySingleton();

	editor = CreateGameModule("editorx86");
	trap->startclientfromsave(nullptr);

	uGameFlags |= (1 << Rapture_EditorLoaded);
}

void RaptureGame::SaveAndExit() {
	if (Console::GetSingleton()->IsOpen()) {
		Console::GetSingleton()->Hide();
	}

	if (trap) {
		trap->saveandexit();
	}
	Network::DisconnectFromRemote();
	R_Message(PRIORITY_MESSAGE, "creating main menu webview\n");
	MainMenu::GetSingleton();

	uGameFlags = ~((1 << Rapture_GameLoaded) | (1 < Rapture_EditorLoaded));
}

bool RaptureGame::HasFlags(const RaptureGame::GameFlags flag) {
	return uGameFlags & (1 << flag);
}

void RaptureGame::AddFlag(const RaptureGame::GameFlags flag) {
	uGameFlags |= (1 << flag);
}
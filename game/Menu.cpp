#include "ui_local.h"

using namespace Awesomium;

void EXPORT_execCommand(const JSArray& args) {
	// TODO: print something on error?
	if(args.size() != 1)
		return;
	if(!args[0].IsString())
		return;
	Cmd::ProcessCommand(ToString(args[0].ToString()).c_str());
}

bool CvarGet_IsValid(const JSArray& args) {
	if(args.size() != 1) {
		R_Message(PRIORITY_WARNING, "JS warning: getCvar bad arguments\n");
		return false;
	}
	if(!args[0].IsString()) {
		R_Message(PRIORITY_WARNING, "JS warning: getCvar arg not string\n");
		return false;
	}
	return true;
}

JSValue EXPORT_getCvarString(const JSArray& args) {
	if(!CvarGet_IsValid(args))
		return JSValue::Null();
	string cvarName = ToString(args[0].ToString());
	Cvar::cvarType_e type = CvarSystem::GetCvarType(cvarName);
	if(type != Cvar::CV_STRING) {
		R_Message(PRIORITY_WARNING, "JS warning: getCvarString: cvar %s is not string type\n", cvarName.c_str());
		return JSValue::Null();
	}
	string value = CvarSystem::GetStringValue(cvarName.c_str());
	return JSValue(WSLit(value.c_str()));
}

JSValue EXPORT_getCvarInteger(const JSArray& args) {
	if(!CvarGet_IsValid(args))
		return JSValue::Null();
	string cvarName = ToString(args[0].ToString());
	Cvar::cvarType_e type = CvarSystem::GetCvarType(cvarName);
	if(type != Cvar::CV_INTEGER) {
		R_Message(PRIORITY_WARNING, "JS warning: getCvarInteger: cvar %s is not integer type\n", cvarName.c_str());
		return JSValue::Null();
	}
	return JSValue(CvarSystem::GetIntegerValue(cvarName.c_str()));
}

JSValue EXPORT_getCvarFloat(const JSArray& args) {
	if(!CvarGet_IsValid(args))
		return JSValue::Null();
	string cvarName = ToString(args[0].ToString());
	Cvar::cvarType_e type = CvarSystem::GetCvarType(cvarName);
	if(type != Cvar::CV_FLOAT) {
		R_Message(PRIORITY_WARNING, "JS warning: getCvarFloat: cvar %s is not float type\n", cvarName.c_str());
		return JSValue::Null();
	}
	return JSValue(CvarSystem::GetFloatValue(cvarName.c_str()));
}

JSValue EXPORT_getCvarBoolean(const JSArray& args) {
	if(!CvarGet_IsValid(args))
		return JSValue::Null();
	string cvarName = ToString(args[0].ToString());
	Cvar::cvarType_e type = CvarSystem::GetCvarType(cvarName);
	if(type != Cvar::CV_BOOLEAN) {
		R_Message(PRIORITY_WARNING, "JS warning: getCvarBoolean: cvar %s is not boolean type\n", cvarName.c_str());
		return JSValue::Null();
	}
	return JSValue(CvarSystem::GetBooleanValue(cvarName.c_str()));
}

bool CvarSet_IsValid(const JSArray& args) {
	if(args.size() != 2) {
		R_Message(PRIORITY_WARNING, "JS warning: setCvar using incorrect arguments\n");
		return false;
	}
	if(!args[0].IsString()) {
		R_Message(PRIORITY_WARNING, "JS warning: setCvar: first arg is not string\n");
		return false;
	}
	if(!Cvar::Exists(ToString(args[0].ToString()))) {
		R_Message(PRIORITY_WARNING, "JS warning: setCvar: cvar doesn't exist\n");
		return false;
	}
	return true;
}

void EXPORT_setCvarString(const JSArray& args) {
	if(!CvarSet_IsValid(args))
		return;
	if(!args[1].IsString()) {
		R_Message(PRIORITY_WARNING, "JS warning: setCvarString: second arg is not string\n");
		return;
	}
	CvarSystem::SetStringValue(ToString(args[0].ToString()), (char*)ToString(args[1].ToString()).c_str());
}

void EXPORT_setCvarInteger(const JSArray& args) {
	if(!CvarSet_IsValid(args))
		return;
	if(!args[1].IsInteger()) {
		R_Message(PRIORITY_WARNING, "JS warning: setCvarInteger: second arg is not integer\n");
		return;
	}
	CvarSystem::SetIntegerValue(ToString(args[0].ToString()), args[1].ToInteger());
}

void EXPORT_setCvarFloat(const JSArray& args) {
	if(!CvarSet_IsValid(args))
		return;
	if(!args[1].IsNumber()) {
		R_Message(PRIORITY_WARNING, "JS warning: setCvarFloat: second arg is not float\n");
		return;
	}
	CvarSystem::SetFloatValue(ToString(args[0].ToString()), args[1].ToDouble());
}

void EXPORT_setCvarBoolean(const JSArray& args) {
	if(!CvarSet_IsValid(args))
		return;
	if(!args[1].IsBoolean()) {
		R_Message(PRIORITY_WARNING, "JS warning: setCvarBoolean: second arg is not boolean\n");
		return;
	}
	CvarSystem::SetBooleanValue(ToString(args[0].ToString()), args[1].ToBoolean());
}

void EXPORT_echo(const JSArray& args) {
	R_Message(PRIORITY_MESSAGE, ToString(args[0].ToString()).c_str());
}

JSValue EXPORT_requestSaveInfo(const JSArray& args) {
	// First argument: true for multiplayer games, false otherwise
	if (args.size() != 1) {
		R_Message(PRIORITY_WARNING, "JS warning: requestSaveInfo called with incorrect number of arguments");
		return JSValue(WSLit(""));
	}
	if (!args[0].IsBoolean()) {
		R_Message(PRIORITY_WARNING, "JS warning: requestSaveInfo: argument is not boolean\n");
		return JSValue(WSLit(""));
	}
	const char* saveinfo = SaveGame::RequestSavegameInfo(args[0].ToBoolean());
	return JSValue(WSLit(saveinfo));
}

void EXPORT_deleteSaveFile(const JSArray& args) {
	// First argument: full path to save file
	if (args.size() != 1) {
		R_Message(PRIORITY_WARNING, "JS warning: deleteSaveFile called with incorrect number of arguments");
		return;
	}
	string szFile = ToString(args[0].ToString());
	SaveGame::DeleteSavegame(szFile.c_str());
}

void EXPORT_createSaveFile(const JSArray& args) {
	// First argument: entire string in JSON
	if (args.size() != 1) {
		R_Message(PRIORITY_WARNING, "JS warning: createSaveFile called with incorrect number of arguments");
		return;
	}
	string szJSON = ToString(args[0].ToString());
	SaveGame::CreateSavegame(szJSON.c_str());
}

/* End function definitions */
template<class T>
struct funcTable_t {
	string name;
	T func;
};

typedef funcTable_t<Menu::menuNonReturning> nonReturningTable_t;
typedef funcTable_t<Menu::menuReturning> returningTable_t;

nonReturningTable_t tbl_nonreturn [] = {
	{ "createSaveFile", EXPORT_createSaveFile },
	{ "deleteSaveFile", EXPORT_deleteSaveFile },
	{ "echo", EXPORT_echo },
	{ "execCommand", EXPORT_execCommand },
	{ "setCvarBoolean", EXPORT_setCvarBoolean },
	{ "setCvarFloat", EXPORT_setCvarFloat },
	{ "setCvarInteger", EXPORT_setCvarInteger },
	{ "setCvarString", EXPORT_setCvarString },
	{ "END", nullptr }
};

returningTable_t tbl_return [] = {
	{ "getCvarBoolean", EXPORT_getCvarBoolean },
	{ "getCvarFloat", EXPORT_getCvarFloat },
	{ "getCvarInteger", EXPORT_getCvarInteger },
	{ "getCvarString", EXPORT_getCvarString },
	{ "requestSaveInfo", EXPORT_requestSaveInfo },
	{ "END", nullptr }
};

Menu::Menu() {
	vmState = nullptr;
}

// ugly :s
Menu::Menu(const char *menuName) {
	int renderWidth = 0, renderHeight = 0;
	bool bFullscreen = false;
	Video::GetWindowInfo(&renderWidth, &renderHeight, &bFullscreen);

	R_Message(PRIORITY_NOTE, "Loading %s\n", menuName);
	wView = UI::wc->CreateWebView(renderWidth, renderHeight, UI::sess);
	wView->LoadURL(WebURL(WSLit(menuName)));
	wView->SetTransparent(true);
	while(wView->IsLoading())
		UI::wc->Update();
	UI::StartDrawingMenu(this);
	global = wView->CreateGlobalJavascriptObject(WSLit("Engine"));
	wView->set_js_method_handler(this);
	JSObject jObj = global.ToObject();
	gamemanager = jObj;
	SetupBaseCommands(&jObj);
}

Menu::~Menu() {
	if(!wView) {
		return; // don't.
	}
	UI::StopDrawingMenu(this);
	wView->Destroy();
	wView = nullptr;
}

void Menu::RunJavaScript(const char* sJS) {
	if(!wView) {
		return;
	}
	wView->ExecuteJavascript(WSLit(sJS), WSLit(""));
}

void Menu::SetupBaseCommands(JSObject* obj) {
	int i;

	for(i = 0; tbl_nonreturn[i].func; i++) {
		m_pfNonReturning.insert(make_pair(tbl_nonreturn[i].name, tbl_nonreturn[i].func));
		obj->SetCustomMethod(WSLit(tbl_nonreturn[i].name.c_str()), false);
	}

	for(i = 0; tbl_return[i].func; i++) {
		m_pfReturning.insert(make_pair(tbl_return[i].name, tbl_return[i].func));
		obj->SetCustomMethod(WSLit(tbl_return[i].name.c_str()), true);
	}
}

bool Menu::ExecuteBaseCommand(const string& command, const JSArray& args) {
	auto it = m_pfNonReturning.find(command);
	if(it == m_pfNonReturning.end())
		return false;
	it->second(args);
	return true;
}

pair<bool, JSValue> Menu::ExecuteBaseCommandWithReturn(const string& command, const JSArray& args) {
	JSValue returnValue;
	auto it = m_pfReturning.find(command);
	if(it == m_pfReturning.end())
		return make_pair(false, returnValue);
	returnValue = it->second(args);
	return make_pair(true, returnValue);
}

bool Menu::ExecuteVMCommand(const string& command, const JSArray& args) {
	for(auto it = m_pfVM.begin(); it != m_pfVM.end(); ++it) {
		if(it->first == command) {
			vmState = const_cast<JSArray*>(&args);
			it->second();
			return true;
		}
	}
	return false;
}

void Menu::OnMethodCall(WebView* caller, unsigned int remote_caller_id, const WebString& method_name, const JSArray& args) {
	if(ExecuteBaseCommand(ToString(method_name), args)) {
		return;
	} else if(ExecuteVMCommand(ToString(method_name), args)) {
		return;
	}
}

JSValue Menu::OnMethodCallWithReturnValue(WebView* caller, unsigned int remote_caller_id, const WebString& method_name, const JSArray& args) {
	pair<bool, JSValue> method = ExecuteBaseCommandWithReturn(ToString(method_name), args);
	if(method.first) {
		return method.second;
	}
	return JSValue(false);
}

void Menu::AssignCallback(const char* sCallbackName, menuVMCallback callback) {
	if(!callback) {
		R_Message(PRIORITY_WARNING, "WARNING: callback %s with no callback pointer!\n", sCallbackName);
		return;
	}
	m_pfVM[sCallbackName] = callback;
	gamemanager.SetCustomMethod(WebString(WSLit(sCallbackName)), false);
}

unsigned int Menu::GetVMArgCount() {
	if(vmState == nullptr) {
		R_Message(PRIORITY_ERROR, "Menu::GetVMArgCount(): null vmState\n");
		return -1;
	}
	return vmState->size();
}

void Menu::GetVMStringArg(unsigned int iArgNum, char* sBuffer, size_t numChars) {
	if(vmState == nullptr) {
		R_Message(PRIORITY_ERROR, "Menu::GetVMStringArg: null vmState\n");
		return;
	}
	if(iArgNum >= vmState->size()) {
		R_Message(PRIORITY_ERROR, "Menu::GetVMStringArg: iArgNum >= argCount\n");
		return;
	}
	if(sBuffer == nullptr) {
		R_Message(PRIORITY_ERROR, "Menu::GetVMStringArg: null buffer\n");
		return;
	}
	if(numChars <= 0) {
		R_Message(PRIORITY_ERROR, "Menu::GetVMStringArg: no size\n");
		return;
	}
	strncpy(sBuffer, ToString(vmState->At(iArgNum).ToString()).c_str(), numChars);
}

int Menu::GetVMIntArg(unsigned int iArgNum) {
	if(vmState == nullptr) {
		R_Message(PRIORITY_ERROR, "Menu::GetVMIntArg: null vmState\n");
		return -1;
	}
	if(iArgNum >= vmState->size()) {
		R_Message(PRIORITY_ERROR, "Menu::GetVMIntArg: iArgNum >= argCount\n");
		return -1;
	}
	return vmState->At(iArgNum).ToInteger();
}

double Menu::GetVMDoubleArg(unsigned int iArgNum) {
	if(vmState == nullptr) {
		R_Message(PRIORITY_ERROR, "Menu::GetVMDoubleArg: null vmState\n");
		return 0.0;
	}
	if(iArgNum >= vmState->size()) {
		R_Message(PRIORITY_ERROR, "Menu::GetVMDoubleArg: iArgNum >= argCount\n");
		return 0.0;
	}
	return vmState->At(iArgNum).ToDouble();
}

bool Menu::GetVMBoolArg(unsigned int iArgNum) {
	if(vmState == nullptr) {
		R_Message(PRIORITY_ERROR, "Menu::GetVMBoolArg: null vmState\n");
		return false;
	}
	if(iArgNum >= vmState->size()) {
		R_Message(PRIORITY_ERROR, "Menu::GetVMBoolArg: iArgNum >= argCount\n");
		return false;
	}
	return vmState->At(iArgNum).ToBoolean();
}
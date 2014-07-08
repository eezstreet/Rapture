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
		R_Printf("JS warning: getCvar bad arguments\n");
		return false;
	}
	if(!args[0].IsString()) {
		R_Printf("JS warning: getCvar arg not string\n");
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
		R_Printf("JS warning: getCvarString: cvar %s is not string type\n", cvarName.c_str());
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
		R_Printf("JS warning: getCvarInteger: cvar %s is not integer type\n", cvarName.c_str());
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
		R_Printf("JS warning: getCvarFloat: cvar %s is not float type\n", cvarName.c_str());
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
		R_Printf("JS warning: getCvarBoolean: cvar %s is not boolean type\n", cvarName.c_str());
		return JSValue::Null();
	}
	return JSValue(CvarSystem::GetBooleanValue(cvarName.c_str()));
}

bool CvarSet_IsValid(const JSArray& args) {
	if(args.size() != 2) {
		R_Printf("JS warning: setCvar using incorrect arguments\n");
		return false;
	}
	if(!args[0].IsString()) {
		R_Printf("JS warning: setCvar: first arg is not string\n");
		return false;
	}
	if(!Cvar::Exists(ToString(args[0].ToString()))) {
		R_Printf("JS warning: setCvar: cvar doesn't exist\n");
		return false;
	}
	return true;
}

void EXPORT_setCvarString(const JSArray& args) {
	if(!CvarSet_IsValid(args))
		return;
	if(!args[1].IsString()) {
		R_Printf("JS warning: setCvarString: second arg is not string\n");
		return;
	}
	CvarSystem::SetStringValue(ToString(args[0].ToString()), (char*)ToString(args[1].ToString()).c_str());
}

void EXPORT_setCvarInteger(const JSArray& args) {
	if(!CvarSet_IsValid(args))
		return;
	if(!args[1].IsInteger()) {
		R_Printf("JS warning: setCvarInteger: second arg is not integer\n");
		return;
	}
	CvarSystem::SetIntegerValue(ToString(args[0].ToString()), args[1].ToInteger());
}

void EXPORT_setCvarFloat(const JSArray& args) {
	if(!CvarSet_IsValid(args))
		return;
	if(!args[1].IsNumber()) {
		R_Printf("JS warning: setCvarFloat: second arg is not float\n");
		return;
	}
	CvarSystem::SetFloatValue(ToString(args[0].ToString()), args[1].ToDouble());
}

void EXPORT_setCvarBoolean(const JSArray& args) {
	if(!CvarSet_IsValid(args))
		return;
	if(!args[1].IsBoolean()) {
		R_Printf("JS warning: setCvarBoolean: second arg is not boolean\n");
		return;
	}
	CvarSystem::SetBooleanValue(ToString(args[0].ToString()), args[1].ToBoolean());
}

void EXPORT_echo(const JSArray& args) {
	R_Printf(ToString(args[0].ToString()).c_str());
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
	{ "execCommand", EXPORT_execCommand },
	{ "setCvarString", EXPORT_setCvarString },
	{ "setCvarInteger", EXPORT_setCvarInteger },
	{ "setCvarFloat", EXPORT_setCvarFloat },
	{ "setCvarBoolean", EXPORT_setCvarBoolean },
	{ "echo", EXPORT_echo },
	{ "END", NULL }
};

returningTable_t tbl_return [] = {
	{ "getCvarString", EXPORT_getCvarString },
	{ "getCvarInteger", EXPORT_getCvarInteger },
	{ "getCvarFloat", EXPORT_getCvarFloat },
	{ "getCvarBoolean", EXPORT_getCvarBoolean },
	{ "END", NULL }
};

Menu::Menu() {
}

Menu::Menu(const char *menuName) {
	R_Printf("Loading %s\n", menuName);
	string mainFileName = "file://" + File::GetFileSearchPath(menuName);
	wView = wc->CreateWebView(r_width->Integer(), r_height->Integer());
	wView->LoadURL(WebURL(WSLit(mainFileName.c_str())));
	wView->SetTransparent(true);
	while(wView->IsLoading())
		wc->Update();
	AddRenderable(wView);
	global = wView->CreateGlobalJavascriptObject(WSLit("GameManager"));
	wView->set_js_method_handler(this);
	JSObject jObj = global.ToObject();
	SetupBaseCommands(&jObj);
}

Menu::~Menu() {
	if(!wView) {
		return; // don't.
	}
	RemoveRenderable(wView);
	wView->Destroy();
	wView = NULL;
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

void Menu::OnMethodCall(WebView* caller, unsigned int remote_caller_id, const WebString& method_name, const JSArray& args) {
	if(ExecuteBaseCommand(ToString(method_name), args)) {
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
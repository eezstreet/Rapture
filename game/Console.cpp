#include "sys_local.h"
#include "ui_local.h"
#include "tr_shared.h"

using namespace Awesomium;

/* REFACTOR ME */

Console* Console::singleton = nullptr;
string Console::conLines = "";

void Console::Sizeup(vector<string>& args) {
	Console* con = Console::GetSingleton();
	string arg = "0";
	if (args.size() > 1) {
		arg = args[1];
	}
	JSArray outArgs = con->window.ToObject().GetMethodNames();
	for (unsigned int i = 0; i < outArgs.size(); i++) {
		JSValue val = outArgs[i];
		char text[1024] = { 0 };
		val.ToString().ToUTF8(text, sizeof(text));
		text[val.ToString().length()] = '\n';
		text[val.ToString().length() + 1] = '\0';
		R_Message(PRIORITY_DEBUG, text);
	}
}

void Console::Sizedn(vector<string>& args) {
	Console* con = Console::GetSingleton();
	string arg = "0";
	if (args.size() > 1) {
		arg = args[1];
	}
	JSArray outArgs;
	outArgs.Push(WSLit(arg.c_str()));
	con->window.ToObject().Invoke(WSLit("EXPORT_DecreaseTextSize"), outArgs);
}

const int Console::GetLineCount() {
	JSArray args; // don't care/garbage

	JSValue retVal = window.ToObject().Invoke(WSLit("EXPORT_NumDisplayedLines"), args);
	return retVal.ToInteger();
}

void Console::SendConsoleLines(string lines) {
	JSArray args;

	args.Push(WSLit(lines.c_str()));
	window.ToObject().Invoke(WSLit("EXPORT_SendNewLine"), args);
}

void Console::PushConsoleMessage(string message) {
	conLines += message;
	printf(message.c_str());

	if(SingletonExists())
		GetSingleton()->SendConsoleLines(message);
}

void Console::ReplaceConsoleContents() { // Gets called on opening console
	SendConsoleLines(conLines);
}

Console::Console() {
	int renderWidth = 0, renderHeight = 0;
	bool bFullscreen = false;
	bIsOpen = false;
	Video::GetWindowInfo(&renderWidth, &renderHeight, &bFullscreen);
	wView = UI::wc->CreateWebView(renderWidth, renderHeight, UI::sess);
	wView->SetTransparent(true);
	BlankConsole();
	while(wView->IsLoading())
		UI::wc->Update();
	global = wView->CreateGlobalJavascriptObject(WSLit(("ConsoleManager")));
	wView->set_js_method_handler(this);

	inputBuffer.push_back("");

	JSObject& obj = global.ToObject();
	SetupBaseCommands(&obj);
	obj.SetCustomMethod(WSLit("sendToClipboard"), false);
	obj.SetCustomMethod(WSLit("getClipboard"), true);
	obj.SetCustomMethod(WSLit("sendConsoleCommand"), false);
	obj.SetCustomMethod(WSLit("openSaysAMe"), false);
	obj.SetCustomMethod(WSLit("inputBufferUp"), false);
	obj.SetCustomMethod(WSLit("inputBufferDown"), false);
	obj.SetCustomMethod(WSLit("tabComplete"), false);

	window = wView->ExecuteJavascriptWithResult(WSLit("window"), WSLit(""));

	Cmd::AddCommand("sizeup", Console::Sizeup);
	Cmd::AddCommand("sizedn", Console::Sizedn);
}

Console::~Console() {
	UI::StopDrawingMenu(this);
	wView->Destroy();
	wView = nullptr;
	UI::currentFocus = prevFocus;
	bIsOpen = false;
}

Console::Console(Console& other) {
	bIsOpen = other.bIsOpen;
	m_pfNonReturning = other.m_pfNonReturning;
	m_pfReturning = other.m_pfReturning;
}

Console& Console::operator=(Console& other) {
	m_pfReturning = other.m_pfReturning;
	m_pfNonReturning = other.m_pfNonReturning;
	bIsOpen = other.bIsOpen;
	return *this;
}

void Console::Show(){
	bIsOpen = true;
	wView->LoadURL(WebURL(WSLit("asset://Rapture/menus/console")));
	UI::StartDrawingMenu(this);
	prevFocus = UI::currentFocus;
	UI::currentFocus = wView;
	rccb = /*FIXME*/ nullptr;
	// HACK: Sometimes doesn't work on first focus, so we need to do this twice
	wView->Focus();
	wView->Focus();
	ReplaceConsoleContents();
	itBufferPosition = inputBuffer.end()-1;
	UpdateInputBufferPosition();
}

void Console::Hide() {
	bIsOpen = false;
	// Blanking the console prevents the game from drawing the console while it's onscreen, but it doesn't kill the webview as we need it later
	BlankConsole();
	UI::StopDrawingMenu(this);
	UI::currentFocus = prevFocus;
	if(UI::currentFocus) {
		// HACK: Sometimes doesn't work on first focus, so we need to do this twice
		UI::currentFocus->Focus();
		UI::currentFocus->Focus();
	}
	rccb = nullptr;
}

void Console::BlankConsole() {
	wView->LoadURL(WebURL(WSLit("asset://Rapture/menus/null")));
	UI::wc->Update();
}

void Console::OnMethodCall(Awesomium::WebView* caller, unsigned int remote_caller_id, 
	const Awesomium::WebString& method_name, const Awesomium::JSArray& args) {
	JSArray jsFuncArgs;

	// Handle base commands
	if(ExecuteBaseCommand(ToString(method_name), args)) {
		return;
	}
	// Execute custom commands for the console class
	if(method_name == WSLit("sendToClipboard")) {
		size_t strLength = args[0].ToString().length();
		if(strLength <= 0)
			return;
		// Send the actual copy command
		wView->Copy();

		// Deselect the text
		if(!window.IsObject()) {
			return;
		}

		window.ToObject().Invoke(WSLit("UNIX_DeselectAll"), jsFuncArgs);
	}
	else if(method_name == WSLit("sendConsoleCommand")) {
		WebString buffer = args[0].ToString();
		if(buffer.length() < 1) return;
		inputBuffer.insert(inputBuffer.end()-1, 1, ToString(buffer));
		itBufferPosition = inputBuffer.end()-1;
		UpdateInputBufferPosition();
		PushConsoleMessage("] " + ToString(buffer) + '\n');
		Cmd::ProcessCommand(ToString(buffer).c_str());
	}
	else if(method_name == WSLit("inputBufferUp")) {
		if(itBufferPosition == inputBuffer.begin()) {
			return;
		}
		itBufferPosition--;
		UpdateInputBufferPosition();
	}
	else if(method_name == WSLit("inputBufferDown")) {
		if(itBufferPosition == inputBuffer.end()-1) {
			return;
		}
		itBufferPosition++;
		UpdateInputBufferPosition();
	}
	else if(method_name == WSLit("openSaysAMe")) {
		ReplaceConsoleContents();
	}
	else if(method_name == WSLit("tabComplete")) {
		if(args.size() <= 0) {
			return;
		}
		JSValue theArgument = args[0];
		string value = ToString(theArgument.ToString());
		string tabCompletion = Cmd::TabComplete(value);

		jsFuncArgs.Push(WSLit(tabCompletion.c_str()));
		window.ToObject().Invoke(WSLit("InputBufferUpdated"), jsFuncArgs);
	}
}

JSValue Console::OnMethodCallWithReturnValue(Awesomium::WebView* caller, unsigned int remote_caller_id, 
	const Awesomium::WebString& method_name, const Awesomium::JSArray& args) {
	auto x = ExecuteBaseCommandWithReturn(ToString(method_name), args);
	if(x.first)
		return x.second;
	if(method_name == WSLit("getClipboard")) {
		return JSValue(WSLit(Sys_GetClipboardContents().c_str()));
	}
	return JSValue(0);
}

void Console::UpdateInputBufferPosition() {
	JSArray args;
	args.Insert(JSValue(WSLit(itBufferPosition->c_str())), 0);
	window.ToObject().Invoke(WSLit("InputBufferUpdated"), args);
}
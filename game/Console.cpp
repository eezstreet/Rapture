#include "sys_local.h"
#include "ui_local.h"
#include "tr_shared.h"

using namespace Awesomium;

/* Manipulation of text box information */

Console* Console::singleton = nullptr;
string Console::conLines = "";

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
	bIsOpen = false;
	wView = wc->CreateWebView(r_width->Integer(), r_height->Integer());
	wView->SetTransparent(true);
	BlankConsole();
	while(wView->IsLoading())
		wc->Update();
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
}

Console::~Console() {
	RemoveRenderable(wView);
	wView->Destroy();
	wView = nullptr;
	currentFocus = prevFocus;
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
	string conFileName = "file://" + File::GetFileSearchPath("ui/console.html");
	wView->LoadURL(WebURL(WSLit(conFileName.c_str())));
	AddRenderable(wView);
	prevFocus = currentFocus;
	currentFocus = wView;
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
	RemoveRenderable(wView);
	currentFocus = prevFocus;
	if(currentFocus) {
		// HACK: Sometimes doesn't work on first focus, so we need to do this twice
		currentFocus->Focus();
		currentFocus->Focus();
	}
	rccb = nullptr;
}

void Console::BlankConsole() {
	string conFileName = "file://" + File::GetFileSearchPath("ui/null.html");
	wView->LoadURL(WebURL(WSLit(conFileName.c_str())));
	wc->Update();
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
}

void Console::UpdateInputBufferPosition() {
	JSArray args;
	args.Insert(JSValue(WSLit(itBufferPosition->c_str())), 0);
	window.ToObject().Invoke(WSLit("InputBufferUpdated"), args);
}
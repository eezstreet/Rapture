#pragma once
#include "tr_local.h"
#include "sys_local.h"

#include <Awesomium/WebCore.h>
#include <Awesomium/STLHelpers.h>
#include <Awesomium/BitmapSurface.h>

extern Awesomium::WebCore *wc;
extern Awesomium::WebView* currentFocus; // If this is non-null, then we only pipe input to that object, otherwise we do this for all renderables

void AddRenderable(Awesomium::WebView* wv);
void RemoveRenderable(Awesomium::WebView* wv);

class Menu : public Awesomium::JSMethodHandler {
public:
	typedef void (*menuNonReturning)(const Awesomium::JSArray&);
	typedef Awesomium::JSValue (*menuReturning)(const Awesomium::JSArray&);

	Awesomium::JSValue window;

	void RunJavaScript(const char* sJS);

	Menu();
	Menu(const char* menu);
	~Menu();
protected:
	Awesomium::WebView* wView;
	Awesomium::JSValue global;
	unordered_map<string, menuNonReturning> m_pfNonReturning;
	unordered_map<string, menuReturning> m_pfReturning;

	void OnMethodCall(Awesomium::WebView* caller, unsigned int remote_caller_id, const Awesomium::WebString& method_name, const Awesomium::JSArray& args);
	Awesomium::JSValue OnMethodCallWithReturnValue(Awesomium::WebView* caller, unsigned int remote_caller_id, const Awesomium::WebString& method_name, const Awesomium::JSArray& args);

	bool ExecuteBaseCommand(const string& command, const Awesomium::JSArray& args);
	pair<bool, Awesomium::JSValue> ExecuteBaseCommandWithReturn(const string& command, const Awesomium::JSArray& args);
	void SetupBaseCommands(Awesomium::JSObject *obj);
};

template<class T>
class SingletonMenu {
protected:
	static T* singleton;
public:
	static T* GetSingleton() { if(!singleton) return singleton = new T(); return singleton; }
	static void DestroySingleton() { if(!singleton) return; delete singleton; singleton = NULL; }
	static bool SingletonExists() { return (singleton != NULL); }
};

class Console : public SingletonMenu<Console>, public Menu {
private:
	void OnMethodCall(Awesomium::WebView* caller, 
		unsigned int remote_caller_id,
		const Awesomium::WebString& method_name,
		const Awesomium::JSArray& args);
	Awesomium::JSValue OnMethodCallWithReturnValue(
		Awesomium::WebView* caller,
		unsigned int remote_caller_id,
		const Awesomium::WebString& method_name,
		const Awesomium::JSArray& args);

	const int GetLineCount();
	void SendConsoleLines(string lines);
	static string conLines;
	void ReplaceConsoleContents();

	Console(Console& other);
	Console& operator= (Console& other);

	bool bIsOpen;
	Awesomium::WebView* prevFocus;

	vector<const string> inputBuffer;
	vector<const string>::iterator itBufferPosition;
	void UpdateInputBufferPosition();
public:
	Console();
	~Console();

	void Show();
	void Hide();
	bool IsOpen() const { return bIsOpen; }
	void BlankConsole();
	static void PushConsoleMessage(string message);
};

class MainMenu : public SingletonMenu<MainMenu>, public Menu {
	void OnMethodCall(Awesomium::WebView* caller,
			unsigned int remote_caller_id,
			const Awesomium::WebString& method_name,
			const Awesomium::JSArray& args);
		Awesomium::JSValue OnMethodCallWithReturnValue(
			Awesomium::WebView* caller,
			unsigned int remote_caller_id,
			const Awesomium::WebString& method_name,
			const Awesomium::JSArray& args);
public:
	MainMenu();
	~MainMenu();
	MainMenu(MainMenu& other);
	MainMenu& operator= (MainMenu& other);
};

typedef void (*RightClickCallback)(void); // HACK
extern RightClickCallback rccb;
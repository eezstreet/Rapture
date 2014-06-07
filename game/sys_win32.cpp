#include "win32_local.h"
#include <direct.h>
#include <process.h>

char* Sys_FS_GetHomepath() {
	// not sure
	return NULL;
}

char* Sys_FS_GetBasepath() {
	static char cwd[1024];
	_getcwd(cwd, 1023);
	cwd[1023] = '\0';
	return cwd;
}

void Sys_FS_MakeDirectory(const char* path) {
	CreateDirectory(path, NULL);
}

void Sys_RunThread(void (*threadRun)(void*), void* arg) {
	_beginthread(threadRun, 0, arg);
}

// http://stackoverflow.com/questions/14762456/getclipboarddatacf-text
string Sys_GetClipboardContents() {
	if(!OpenClipboard(nullptr))
		return "";

	// Get handle of clipboard object for ANSI text
	HANDLE hData = GetClipboardData(CF_TEXT);
	if (hData == nullptr)
		return ""; // error

	// Lock the handle to get the actual text pointer
	char * pszText = static_cast<char*>( GlobalLock(hData) );
	if (pszText == nullptr)
		return ""; // error

	// Save text in a string class instance
	std::string text( pszText );

	// Release the lock
	GlobalUnlock( hData );

	// Release the clipboard
	CloseClipboard();

	return text;
}

void Sys_SendToClipboard(string text) {
	if(text.length() <= 0) {
		OpenClipboard(0);
		EmptyClipboard();
		CloseClipboard();
		return;
	}
	HGLOBAL hMem =  GlobalAlloc(GMEM_MOVEABLE, text.length());
	if(hMem == 0) {
		R_Printf("Sys_SendToClipboard: couldn't GlobalAlloc (out of memory?)\n");
		return;
	}
	memcpy(GlobalLock(hMem), text.c_str(), text.length());
	GlobalUnlock(hMem);
	OpenClipboard(0);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, hMem);
	CloseClipboard();
}

ptModule Sys_LoadLibrary(string name) {
	name.append(".dll");
	string filepath = File::GetFileSearchPath(name);
	ptModule hLib = (ptModule)LoadLibrary(filepath.c_str());
	if(!hLib) {
		R_Printf("module load failure: %i\n", GetLastError());
	}
	return hLib;
}

ptModuleFunction Sys_GetFunctionAddress(ptModule module, string name) {
	return (ptModuleFunction)GetProcAddress((HMODULE)module, name.c_str());
}

bool Sys_Assertion(const char* msg, const char* file, const unsigned int line) {
	char text[256];
	sprintf(text, "Assertion Failure!\r\nFile: %s\r\nLine: %i\r\nExpression: %s", file, line, msg);
	int val = MessageBox(NULL, text, "Rapture Assertion Failure",  MB_ABORTRETRYIGNORE|MB_ICONWARNING|MB_TASKMODAL|MB_SETFOREGROUND|MB_TOPMOST);
	switch(val) {
		default:
		case IDABORT:
			Cmd::ProcessCommand("quit");
			return false;
		case IDRETRY:
			return true;
		case IDIGNORE:
			return false;
	}
}

void Sys_Error(const char* error, ...) {
	va_list		argptr;
	char		text[4096];
	va_start (argptr, error);
	vsnprintf(text, sizeof(text), error, argptr);
	va_end (argptr);
	R_Printf(text);
	R_Printf("\n");

	RenderCode::Exit(true);

	viewlog->SetErrorText(text);
	viewlog->Show();

	setGameQuitting(false);
	throw false;
}
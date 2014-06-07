#pragma once
#include "sys_shared.h"
#include "tr_shared.h"
#include "ui_shared.h"
#include <SDL.h>
#include <SDL_ttf.h>

#define R_Error Sys_Error

typedef void* ptModule;
typedef void* ptModuleFunction;

//
// sys_main.cpp
//
class GameModule {
	// modcode that gets loaded via shared objects
private:
	ptModule ptModuleHandle;
public:
	GameModule(string modulename);
	~GameModule();
	gameExports_s* GetRefAPI(gameImports_s* import);
};

class RaptureGame {
private:
	gameExports_s*	trap;

	void AssignExports(gameImports_s* in);
public:
	GameModule*		game;
	GameModule*		editor;

	RaptureGame(int argc, char **argv);
	~RaptureGame();

	RaptureGame(const RaptureGame& other) : bHasFinished(other.bHasFinished) {}
	RaptureGame& operator= (RaptureGame other) { bHasFinished = other.bHasFinished; return *this; }

	void HandleCommandline(int argc, char **argv);
	void PassQuitEvent();

	bool bHasFinished;
	void RunLoop();

	GameModule* CreateGameModule(const char* bundle);

	static GameModule* GetGameModule();
	static gameExports_s* GetImport();
};

//
// CvarSystem.cpp
//

template<typename T>
struct CvarValueSet {
	T defaultVal;
	T currentVal;

	void AssignBoth(T value) { defaultVal = currentVal = value; }
};

class Cvar {
	string name;
	string description;
	union {
		CvarValueSet<char*> s;
		CvarValueSet<int> i;
		CvarValueSet<float> v;
		CvarValueSet<bool> b;
	};
public:
	enum cvarType_e {
		CV_STRING,
		CV_INTEGER,
		CV_FLOAT,
		CV_BOOLEAN
	};
	enum cvarFlags_e {
		CVAR_ARCHIVE,
		CVAR_ROM,
		CVAR_ANNOUNCE,
	};
private:
	cvarType_e type;
	int flags;
	void AssignHardValue(char* value) { s.AssignBoth(value); }
	void AssignHardValue(int value) { i.AssignBoth(value); }
	void AssignHardValue(float value) { v.AssignBoth(value); }
	void AssignHardValue(bool value) { b.AssignBoth(value); }
	bool registered;
public:
	Cvar(Cvar&& other);
	Cvar(const string& sName, const string& sDesc, int iFlags, char* startValue);
	Cvar(const string& sName, const string& sDesc, int iFlags, int startValue);
	Cvar(const string& sName, const string& sDesc, int iFlags, float startValue);
	Cvar(const string& sName, const string& sDesc, int iFlags, bool startValue);
	Cvar();
	~Cvar();
	Cvar& operator= (char* value);
	Cvar& operator= (int value);
	Cvar& operator= (float value);
	Cvar& operator= (bool value);

	cvarType_e GetType() { return type; }
	int GetFlags() { return flags; }
	string GetName() { return name; }
	string GetDescription() { return description; }

	void SetValue(char* value) { if(type != CV_STRING) return; s.currentVal = value; if(flags & (1 << CVAR_ANNOUNCE)) R_Printf("%s changed to %s\n", name.c_str(), value); }
	void SetValue(int value) { if(type != CV_INTEGER) return; i.currentVal = value; if(flags & (1 << CVAR_ANNOUNCE)) R_Printf("%s changed to %i\n", name.c_str(), value); }
	void SetValue(float value) { if(type != CV_FLOAT) return; v.currentVal = value; if(flags & (1 << CVAR_ANNOUNCE)) R_Printf("%s changed to %f\n", name.c_str(), value); }
	void SetValue(bool value) { if(type != CV_BOOLEAN) return; b.currentVal = value; if(flags & (1 << CVAR_ANNOUNCE)) R_Printf("%s changed to %s\n", name.c_str(), btoa(value)); }
	inline char* String() { return s.currentVal; }
	inline int Integer() { return i.currentVal; }
	inline float Value() { return v.currentVal; }
	inline bool Bool() { return b.currentVal; }
	inline char* DefaultString() { return s.defaultVal; }
	inline int DefaultInteger() { return i.defaultVal; }
	inline float DefaultValue() { return v.defaultVal; }
	inline bool DefaultBool() { return b.defaultVal; }

	template<typename T>
	static Cvar* Get(const string& sName, const string& sDesc, int iFlags, T startValue) {
		try {
			auto it = CvarSystem::cvars.find(sName);
			if(it == CvarSystem::cvars.end())
				throw out_of_range("not registered");
			Cvar* cv = it->second;
			if(!cv->registered)
				throw out_of_range("not registered");
			return cv;
		}
		catch( out_of_range e ) {
			// register a new cvar
			return CvarSystem::RegisterCvar(sName, sDesc, iFlags, startValue);
		}
	}

	static bool Exists(const string& sName);
friend class CvarSystem;
friend class Cvar;
};

// Whenever we use /set or /seta on nonexisting cvars, those cvars are chucked into the cache
struct CvarCacheObject {
	string initvalue;
	bool archive;
};

class CvarSystem {
	static unordered_map<string, Cvar*> cvars;
	static unordered_map<string, CvarCacheObject*> cache;
	static bool init;
	static void Cache_Free(const string& sName);
	static void ArchiveCvars();
public:
	static string CvarSystem::GetNextCvar(const string& previous, bool& bFoundCommand);
	static string CvarSystem::GetFirstCvar(bool& bFoundCommand);

	static Cvar* RegisterCvar(Cvar *cvar);
	static Cvar* RegisterCvar(const string& sName, const string& sDesc, int iFlags, char* startValue);
	static Cvar* RegisterCvar(const string& sName, const string& sDesc, int iFlags, int startValue);
	static Cvar* RegisterCvar(const string& sName, const string& sDesc, int iFlags, float startValue);
	static Cvar* RegisterCvar(const string& sName, const string& sDesc, int iFlags, bool startValue);
	static void CacheCvar(const string& sName, const string& sValue, bool bArchive = false);
	static void Initialize();
	static void Destroy();

	static int GetCvarFlags(const string& sName) { return cvars[sName]->flags; }
	static void SetCvarFlags(const string& sName, int flags) { cvars[sName]->flags = flags; }
	static Cvar::cvarType_e GetCvarType(const string& sName) { return cvars[sName]->type; }

	static void SetStringValue(const string &sName, char* newValue) { cvars[sName]->s.currentVal = newValue; }
	static void SetIntegerValue(const string &sName, int newValue) { cvars[sName]->i.currentVal = newValue; }
	static void SetFloatValue(const string &sName, float newValue) { cvars[sName]->v.currentVal = newValue; }
	static void SetBooleanValue(const string &sName, bool newValue) { cvars[sName]->b.currentVal = newValue; }

	static string GetStringValue(const string& sName);
	static int GetIntegerValue(const string& sName);
	static float GetFloatValue(const string& sName);
	static bool GetBooleanValue(const string& sName);

	static void ListCvars();

	static bool ProcessCvarCommand(const string& sName, const vector<string>& VArguments);

	static void EXPORT_IntValue(const char* name, int* value);
	static void EXPORT_StrValue(const char* name, char* value);
	static void EXPORT_BoolValue(const char* name, bool* value);
	static void EXPORT_Value(const char* name, float* value);
friend class Cvar;
};

/* Memory */

//
// Zone.cpp
//

namespace Zone {
	enum zoneTags_e {
		TAG_NONE,
		TAG_CVAR,
		TAG_FILES,
		TAG_RENDERER,
		TAG_IMAGES,
		TAG_MATERIALS,
		TAG_FONT,
		TAG_CUSTOM, // should only be used by mods
		MAX_ENGINE_TAGS // last of the engine tags, but we can add more for vms
	};

	extern string tagNames[];

	struct ZoneChunk {
		size_t memInUse;
		bool isClassObject;

		ZoneChunk(size_t mem, bool bClass) : 
		memInUse(mem), isClassObject(bClass) {}
		ZoneChunk() { memInUse = 0; isClassObject = false; }
	};

	struct ZoneTag {
		size_t zoneInUse;
		size_t peakUsage;
		map<void*, ZoneChunk> zone;

		ZoneTag() : zoneInUse(0), peakUsage(0) { }
	};

	class MemoryManager {
	private:
		unordered_map<string, ZoneTag> zone;
	public:
		void* Allocate(int iSize, zoneTags_e tag);
		void* Allocate(int iSize, const string& tag);
		void Free(void* memory);
		void FastFree(void* memory, const string& tag);
		void FreeAll(const string& tag);
		void* Reallocate(void *memory, size_t iNewSize);
		void CreateZoneTag(const string& tag) { zone[tag] = ZoneTag(); }
		MemoryManager();
		~MemoryManager(); // deliberately ignoring rule of three
		void PrintMemUsage();

		template<typename T>
		T* AllocClass(zoneTags_e tag) {
			T* retVal = new T();
			ZoneChunk zc(sizeof(T), true);
			zone[tagNames[tag]].zoneInUse += sizeof(T);
			if(zone[tagNames[tag]].zoneInUse > zone[tagNames[tag]].peakUsage) {
				zone[tagNames[tag]].peakUsage = zone[tagNames[tag]].zoneInUse;
			}
			zone[tagNames[tag]].zone[retVal] = zc;
			return retVal;
		}
	};

	extern MemoryManager *mem;

	void Init();
	void Shutdown();

	void* Alloc(int iSize, zoneTags_e tag);
	void* Alloc(int iSize, const string& tag);
	void* VMAlloc(int iSize, const char* tag)/* { return Alloc(iSize, tag); }*/ ;
	void  Free(void* memory);
	void  FastFree(void *memory, const string& tag);
	void  VMFastFree(void* memory, const char* tag)/* { FastFree(memory, tag); }*/ ;
	void  FreeAll(const string& tag);
	void  VMFreeAll(const char* tag)/* { FreeAll(tag); }*/ ;
	void* Realloc(void *memory, size_t iNewSize);
	void  NewTag(const char* tag)/* { mem->CreateZoneTag(tag); }*/ ;
	template<typename T>
	T* New(zoneTags_e tag) { return mem->AllocClass<T>(tag); }
	void MemoryUsage();
};

//
// Hunk.cpp
//

namespace Hunk {
	class MemoryManager {
	public:
		void *Allocate(int iSize);
		void Free(void* memory);
	};

	void Init();
	void Shutdown();
};


//
// FileSystem.cpp
//

class File {
	FILE* handle;
	string searchpath;
public:
	static File* Open(const string &file, const string& mode);
	void Close();
	string ReadPlaintext(size_t numchar = 0);
	size_t ReadBinary(unsigned char* bytes, size_t numbytes, const bool bDontResetCursor);
	wstring ReadUnicode(size_t numchars = 0);
	size_t GetSize();
	inline size_t GetUnicodeSize() { return GetSize()/2; }
	size_t WritePlaintext(const string& text) { return fwrite(text.c_str(), sizeof(char), text.length(), handle); }
	size_t WriteUnicode(const wstring& text) { return fwrite(text.c_str(), sizeof(wchar_t), text.length(), handle); }
	size_t WriteBinary(void* bytes, size_t numbytes) { return fwrite(bytes, sizeof(unsigned char), numbytes, handle); }
	bool Seek(long offset, int origin) { if(fseek(handle, offset, origin)) return true; return false; }
	size_t Tell() { return ftell(handle); }
	static string GetFileSearchPath(const string& fileName);
	bool Exists(const string &file);
friend class File;
};

class FileSystem {
	///////////////
	// Class Properties
	// and methods
	///////////////
private:
	vector<string> searchpaths;
	unordered_map<string, File*> files;
	void RecursivelyTouch(const string& path);

public:
	void AddSearchPath(const string& searchpath) { string s = searchpath; stringreplace(s, "\\", "/"); searchpaths.push_back(s); }
	inline void AddCoreSearchPath(const string& basepath, const string& core) { AddSearchPath(basepath + '/' + core); }
	void CreateModSearchPaths(const string& basepath, const string& modlist);
	FileSystem();
	~FileSystem();
	inline vector<string>& GetSearchPaths() { return searchpaths; }
	void PrintSearchPaths();
	static int ListFiles(const string& dir, vector<string>& in, const string& extension="");

	////////////
	// Static Methods
	////////////
	static void Init();
	static void Shutdown();

	static File* EXPORT_OpenFile(const char* filename, const char* mode);
	static void EXPORT_Close(File* filehandle);
	static int EXPORT_ListFilesInDir(const char* filename, vector<string>& in, const char *extension);
	static size_t EXPORT_ReadBinary(File* filehandle, unsigned char* bytes, size_t numBytes, const bool bDontResetCursor);
	static string EXPORT_ReadPlaintext(File* filehandle, size_t numChars);
	static size_t EXPORT_GetFileSize(File* filehandle);

friend class File;
};

//
// CmdSystem.cpp
//

namespace Cmd {
	void ProcessCommand(const char *cmd);
	void AddCommand(const string& cmdName, conCmd_t cmd);
	void RemoveCommand(const string& cmdName);
	void ClearCommandList();
	void ListCommands();
	vector<string> Tokenize(const string &str);
	string TabComplete(const string& input);
};


//
// Input.cpp
//

class InputManager {
private:
	map<SDL_Scancode, string> binds;
	list<SDL_Scancode> thisFrameKeysDown;
	void BindCommandMouse(const string& keycodeArg, const string& commandArg);
	void (*Keycatcher)(SDL_Keycode key);
public:
	void SendKeyUpEvent(SDL_Keysym key, char* text);
	void SendKeyDownEvent(SDL_Keysym key, char* text);
	void InputFrame();
	void ExecuteBind(const string& bindName);
	void BindCommand(const string& keycodeArg, string commandArg);
	void SendMouseButtonEvent(unsigned int buttonId, unsigned char state, int x, int y);
	void SendMouseMoveEvent(int x, int y);
	void SendTextInputEvent(char* text);
	InputManager();
};

extern InputManager *Input;
void InitInput();
void DeleteInput();
extern const string keycodeNames[];


//
// FontManager.cpp
//

class Font {
private:
	TTF_Font* ptFont;

	void LoadFont(const char* sFontFile, int iPointSize);
public:
	Font();
	~Font();

	TTF_Font* GetFont() { return ptFont; }

	static Font* Register(const char* sFontFile, int iPointSize);
friend class FontManager;
};

class FontManager {
public:
	FontManager();
	~FontManager();

	Font* RegisterFont(const char* sFontFile, int iPointSize);
};
extern FontManager* FontMan;


//
// Timer.cpp
//
class Timer {
protected:
	unsigned long ulStartTicks;
	unsigned long ulPausedTicks;

	bool bIsStarted;
	bool bIsPaused;

	string sTimerName;
public:
	Timer();
	Timer(const string& sName);

	void Start();
	void Stop();
	void Pause();
	void Unpause();

	unsigned long GetTicks();

	bool IsStarted() { return bIsStarted; };
	bool IsPaused() { return bIsPaused; };
};


//
// FrameCapper.cpp
//
class FrameCapper {
protected:
	Timer capTimer;
	Cvar* capCvar;
public:
	void StartFrame();
	void EndFrame();
	FrameCapper();
};


//
// Viewlog (shared)
//
class Viewlog {
protected:
	Cvar* cvViewlog;
	bool bIsShown;
	bool bIsError;
	string errorText;
public:
	virtual void SetErrorText(const string& message) = 0;
	virtual void Show() = 0;
	virtual void Hide() = 0;
	virtual void TestViewlogShow() = 0;
};
extern Viewlog* viewlog;
void Sys_InitViewlog();

void setGameQuitting(const bool b);


//
// sys_cmds.cpp
//

void Sys_InitCommands();


//
// sys_<platform>.cpp
//
char* Sys_FS_GetHomepath();
char* Sys_FS_GetBasepath();
string Sys_GetClipboardContents();
void Sys_SendToClipboard(string text);
void Sys_FS_MakeDirectory(const char* path);
ptModule Sys_LoadLibrary(string name);
ptModuleFunction Sys_GetFunctionAddress(ptModule module, string name);
bool Sys_Assertion(const char* msg, const char* file, const unsigned int line);
void Sys_Error(const char* error, ...);
void Sys_PassToViewlog(const char* text);
#pragma once
#include "sys_shared.h"
#include "tr_shared.h"

class RaptureGame {
public:
	RaptureGame(int argc, char **argv);
	~RaptureGame();

	RaptureGame(const RaptureGame& other) : bHasFinished(other.bHasFinished) {}
	RaptureGame& operator= (RaptureGame other) { bHasFinished = other.bHasFinished; return *this; }

	void HandleCommandline(int argc, char **argv);

	bool bHasFinished;
	void RunLoop();
};

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

	void SetValue(char* value) { if(type != CV_STRING) return; s.currentVal = value; }
	void SetValue(int value) { if(type != CV_INTEGER) return; i.currentVal = value; }
	void SetValue(float value) { if(type != CV_FLOAT) return; v.currentVal = value; }
	void SetValue(bool value) { if(type != CV_BOOLEAN) return; b.currentVal = value; }
	inline char* String() { return s.currentVal; }
	inline int Integer() { return i.currentVal; }
	inline float Value() { return v.currentVal; }
	inline bool Bool() { return b.currentVal; }

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
	static Cvar* RegisterCvar(Cvar *cvar);
	static Cvar* RegisterCvar(const string& sName, const string& sDesc, int iFlags, char* startValue);
	static Cvar* RegisterCvar(const string& sName, const string& sDesc, int iFlags, int startValue);
	static Cvar* RegisterCvar(const string& sName, const string& sDesc, int iFlags, float startValue);
	static Cvar* RegisterCvar(const string& sName, const string& sDesc, int iFlags, bool startValue);
	static void CacheCvar(const string& sName, const string& sValue, bool bArchive = false);
	static void Initialize();
	static void Destroy();

	static int GetCvarFlags(string& sName) { return cvars[sName]->flags; }
	static void SetCvarFlags(string& sName, int flags) { cvars[sName]->flags = flags; }
	static Cvar::cvarType_e GetCvarType(string& sName) { return cvars[sName]->type; }

	static void SetStringValue(string &sName, char* newValue) { cvars[sName]->s.currentVal = newValue; }
	static void SetIntegerValue(string &sName, int newValue) { cvars[sName]->i.currentVal = newValue; }
	static void SetFloatValue(string &sName, float newValue) { cvars[sName]->v.currentVal = newValue; }
	static void SetBooleanValue(string &sName, bool newValue) { cvars[sName]->b.currentVal = newValue; }
friend class Cvar;
};

/* Memory */
namespace Zone {
	enum zoneTags_e {
		TAG_NONE,
		TAG_CVAR,
		TAG_FILES,
		TAG_CUSTOM, // should only be used by mods
		TAG_MAX,
	};

	extern string tagNames[];

	struct ZoneTag {
		size_t zoneInUse;
		unordered_map<void*, pair<size_t, bool>> zone;

		ZoneTag() : zoneInUse(0) { }
	};

	class MemoryManager {
	private:
		unordered_map<string, ZoneTag> zone;
	public:
		void* Allocate(int iSize, zoneTags_e tag);
		void Free(void* memory);
		void FastFree(void* memory, const string tag);
		void FreeAll(const string tag);
		void* Reallocate(void *memory, size_t iNewSize);
		void CreateZoneTag(string& tag) { zone[tag] = ZoneTag(); }
		MemoryManager();
		~MemoryManager(); // deliberately ignoring rule of three

		template<typename T>
		T* AllocClass(zoneTags_e tag) {
			T* retVal = new T();
			zone[tagNames[tag]].zoneInUse += sizeof(T);
			zone[tagNames[tag]].zone[retVal] = make_pair(sizeof(T), true);
			return retVal;
		}
	};

	extern MemoryManager *mem;

	void Init();
	void Shutdown();

	void* Alloc(int iSize, zoneTags_e tag);
	void  Free(void* memory);
	void  FastFree(void *memory, const string tag);
	void  FreeAll(const string tag);
	void* Realloc(void *memory, size_t iNewSize);
	template<typename T>
	T* New(zoneTags_e tag) { return mem->AllocClass<T>(tag); }
};

namespace Hunk {
	class MemoryManager {
	public:
		void *Allocate(int iSize);
		void Free(void* memory);
	};

	void Init();
	void Shutdown();
};

/* Filesystem */
class File {
	FILE* handle;
	string searchpath;
public:
	static pair<bool, File&> Open(const string &file, const string& mode);
	void Close();
	string ReadPlaintext(size_t numchar = 0);
	unsigned char* ReadBinary(size_t numbytes = 0);
	wstring ReadUnicode(size_t numchars = 0);
	size_t GetSize();
	inline size_t GetUnicodeSize() { return GetSize()/2; }
	size_t WritePlaintext(const string& text) { return fwrite(text.c_str(), sizeof(char), text.length(), handle); }
	size_t WriteUnicode(const wstring& text) { return fwrite(text.c_str(), sizeof(wchar_t), text.length(), handle); }
	size_t WriteBinary(void* bytes, size_t numbytes) { return fwrite(bytes, sizeof(unsigned char), numbytes, handle); }
friend class File;
};

namespace FS {
	class FileSystem {
		vector<string> searchpaths;
		unordered_map<string, File*> files;
	public:
		void AddSearchPath(string searchpath) { searchpaths.push_back(searchpath); }
		inline void AddCoreSearchPath(string basepath, string core) { AddSearchPath(basepath + '/' + core); }
		void CreateModSearchPaths(string basepath, string modlist);
		FileSystem();
		~FileSystem();
		inline vector<string>& GetSearchPaths() { return searchpaths; }
	friend class File;
	};

	void Init();
	void Shutdown();
};

/* CmdSystem.cpp */
namespace Cmd {
	void ProcessCommand(const char *cmd);
	void AddCommand(string cmdName, conCmd_t cmd);
	void RemoveCommand(string cmdName);
	void ClearCommandList();
	vector<string> Tokenize(const string &str);
};

// sys_cmds.cpp
void Sys_InitCommands();

// sys_<platform>.cpp
char* Sys_FS_GetHomepath();
char* Sys_FS_GetBasepath();
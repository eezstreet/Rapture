#include "sys_local.h"

namespace FS {
	FileSystem::FileSystem() {}
	FileSystem::~FileSystem() {
		Zone::FreeAll("files");
		files.clear();
	}

	Cvar* fs_core;
	Cvar* fs_homepath;
	Cvar* fs_modlist;
	Cvar* fs_basepath;

	static FileSystem* fs;

	void Init() {
		fs_core = CvarSystem::RegisterCvar("fs_core", "Core directory; contains all essential game data.", Cvar::CVAR_ROM, "core");
		fs_homepath = CvarSystem::RegisterCvar("fs_homepath", "homepath", Cvar::CVAR_ROM, Sys_FS_GetBasepath());
		fs_modlist = CvarSystem::RegisterCvar("fs_modlist", "List of active mods", Cvar::CVAR_ROM, "");
		fs_basepath = CvarSystem::RegisterCvar("fs_basepath", "Directory to gamedata", Cvar::CVAR_ROM, Sys_FS_GetBasepath());

		fs = new FileSystem(); // TODO: use smart pointer
		// the order of these searchpaths matters!
		fs->AddSearchPath(fs_homepath->String());
		fs->AddSearchPath(fs_basepath->String());
		fs->AddCoreSearchPath(fs_basepath->String(), fs_core->String());
		fs->CreateModSearchPaths(fs_basepath->String(), fs_modlist->String());
	}

	void Shutdown() {
		delete fs;
	}

	void FileSystem::CreateModSearchPaths(string basepath, string modlist) {
		if(modlist.length() < 1)
			return; // blank modlist
		vector<string> mods = split(modlist, ';');
		for(auto it = mods.begin(); it != mods.end(); ++it)
			fs->AddCoreSearchPath(fs_basepath->String(), *it);
	}

	inline vector<string> GetSearchPaths() {
		return fs->GetSearchPaths();
	}
};

pair<bool, File&> File::Open(const string& fileName, const string& mode) {
	// If a file has been opened before, we can open it again using the same search path as we did previously.
	unordered_map<string, File*>::iterator it = FS::fs->files.find(fileName);
	if(it != FS::fs->files.end()) {
		string path = it->second->searchpath + fileName;
		if(it->second->handle) return make_pair(false, File());
		it->second->handle = fopen(path.c_str(), mode.c_str());
		return make_pair(true, *it->second);
	}

	// If we are reading, we need to reverse the searchpath list (so we read from homepath last)
	vector<string> searchpaths = FS::GetSearchPaths();
	if(mode.find('w') == string::npos && mode.find('a') == string::npos)
		reverse(searchpaths.begin(), searchpaths.end());

	for(auto it = searchpaths.begin(); it != searchpaths.end(); ++it) {
		// if file found in this search path, good to go
		string path = *it + fileName;
		FILE* file = fopen(path.c_str(), mode.c_str());
		if(file) {
			File *F = (File*)Zone::Alloc(sizeof(File), Zone::TAG_FILES);
			F->searchpath = *it;
			F->handle = file;
			FS::fs->files[fileName] = F;
			return make_pair(true, *F);
		}
	}

	return make_pair(false, File());
}

void File::Close() {
	if(!handle) return;
	fclose(handle);
	handle = NULL;
}

string File::ReadPlaintext(size_t numChars) {
	if(!handle) return "";
	if(numChars == 0) { // reset the cursor to beginning of file and read whole thing
		fseek(handle, 0L, SEEK_SET);
		numChars = GetSize()/sizeof(char);
	}
	char* buf = (char*)Zone::Alloc(sizeof(char)*numChars, Zone::TAG_FILES);
	fread(buf, sizeof(char), numChars, handle);
	string retval = buf;
	Zone::Free(buf);
	return retval;
}

unsigned char* File::ReadBinary(size_t numBytes) {
	if(!handle) return NULL;
	if(numBytes == 0) { // reset cursor to beginning of file and read whole thing
		fseek(handle, 0L, SEEK_SET);
		numBytes = GetSize()/sizeof(unsigned char);
	}
	unsigned char* buf = (unsigned char*)Zone::Alloc(sizeof(unsigned char)*numBytes, Zone::TAG_FILES);
	fread(buf, sizeof(unsigned char), numBytes, handle);
	// FIXME: dangling memory
	return buf;
}

wstring File::ReadUnicode(size_t numChars) {
	if(!handle) return L"";
	if(numChars == 0) { // reset cursor to beginning of file and read whole string
		fseek(handle, 0L, SEEK_SET);
		numChars = GetSize()/sizeof(wchar_t);
	}
	wchar_t *buf = (wchar_t*)Zone::Alloc(sizeof(wchar_t)*numChars, Zone::TAG_FILES);
	fread(buf, sizeof(wchar_t), numChars, handle);
	wstring retval = buf;
	Zone::Free(buf);
	return retval;
}

size_t File::GetSize() {
	size_t currentPos = ftell(handle);
	fseek(handle, 0L, SEEK_END);
	size_t size = ftell(handle);
	fseek(handle, currentPos, SEEK_SET);
	return size;
}
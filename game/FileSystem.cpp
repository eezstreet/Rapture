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
		R_Printf("fs_init\n");
		fs_core = CvarSystem::RegisterCvar("fs_core", "Core directory; contains all essential game data.", Cvar::CVAR_ROM, "core");
		fs_homepath = CvarSystem::RegisterCvar("fs_homepath", "homepath", Cvar::CVAR_ROM, Sys_FS_GetBasepath());
		fs_modlist = CvarSystem::RegisterCvar("fs_modlist", "List of active mods", Cvar::CVAR_ROM, "");
		fs_basepath = CvarSystem::RegisterCvar("fs_basepath", "Directory to gamedata", Cvar::CVAR_ROM, Sys_FS_GetBasepath());

		fs = new FileSystem();
		// the order of these searchpaths matters!
		// These were originally reversed, but reading is done more often than writing, and it resulted in a lot of reversing
		// I figured this would be a nice little optimization
		fs->CreateModSearchPaths(fs_basepath->String(), fs_modlist->String());
		fs->AddCoreSearchPath(fs_basepath->String(), fs_core->String());
		fs->AddSearchPath(fs_basepath->String());
		fs->AddSearchPath(fs_homepath->String());
		fs->PrintSearchPaths();
	}

	void Shutdown() {
		delete fs;
	}

	void FileSystem::CreateModSearchPaths(const string& basepath, const string& modlist) {
		if(modlist.length() < 1)
			return; // blank modlist
		vector<string> mods = split(modlist, ';');
		for(auto it = mods.begin(); it != mods.end(); ++it)
			fs->AddCoreSearchPath(fs_basepath->String(), *it);
	}

	inline vector<string>& GetSearchPaths() {
		return fs->GetSearchPaths();
	}

	void FileSystem::PrintSearchPaths() {
		R_Printf("All search paths:\n");
		for_each(searchpaths.begin(), searchpaths.end(), [](string& s) {
			R_Printf("%s\n", s.c_str());
		});
	}

	int FileSystem::ListFiles(const string& dir, vector<string>& in, const string& extension) {
		DIR* xdir;
		struct dirent *ent;
		int numFiles = 0;
		string fixedDir;
		if(dir.at(dir.length()-1) == '\\' || dir.at(dir.length()-1) == '/') {
			fixedDir = dir;
		}
		else {
			fixedDir = dir + '/';
		}
		auto x = FS::fs->GetSearchPaths();
		for(auto it = x.begin(); it != x.end(); ++it) {
			string compString = *it + '/' + dir; // hm, this'll be searchpath + search dir
			if((xdir = opendir(compString.c_str())) != NULL) {
				// loop through all files
				while((ent = readdir(xdir)) != NULL) {
					string fName = ent->d_name;
					if(extension.length() != 0 && !checkExtension(fName, extension))
						continue;
					in.push_back(fixedDir + fName);
					numFiles++;
				}
				closedir(xdir);
			}
		}
		return numFiles;
	}

	void FileSystem::RecursivelyTouch(const string& path) {
		size_t lastPos = 0;
		size_t nextSlash = 0;
		while((nextSlash = path.find_first_of('/', lastPos)) != path.npos) {
			const string s = path.substr(0, nextSlash+1);
			Sys_FS_MakeDirectory(s.c_str());
			lastPos = nextSlash+1;
		}
	}

	void* EXPORT_OpenFile(const char* filename, const char* mode) {
		return (void*)File::Open(filename, mode);
	}

	void EXPORT_CloseFile(void* filehandle) {
		File* f = (File*)filehandle;
		f->Close();
	}

	int EXPORT_ListFilesInDir(const char* filename, vector<string>& in, const char* ext) {
		return fs->ListFiles(filename, in, ext);
	}

	string EXPORT_ReadPlaintext(void* filehandle, size_t numChars) {
		File* f = (File*)filehandle;
		return f->ReadPlaintext(numChars);
	}

	size_t EXPORT_GetFileSize(void* filehandle) {
		File* f = (File*)filehandle;
		return f->GetSize();
	}

	void EXPORT_Close(void* filehandle) {
		File* f = (File*)filehandle;
		f->Close();
	}
};

File* File::Open(const string& fileName, const string& mode) {
	// Trim off any leading (or trailing) whitespace
	string fixedName = trim(fileName);
	// Replace all instances of \\ with / (Windows fix)
	stringreplace(fixedName, "\\", "/");
	// Now make sure we start with a '/'
	if(fixedName[0] != '/')
		fixedName = '/' + fixedName;

	// If a file has been opened before, we can open it again using the same search path as we did previously.
	bool bWeAreReading = false;
	if(mode.find('w') == string::npos && mode.find('a') == string::npos) {
		// We don't do this when writing (because we always write to homepath)
		auto it = FS::fs->files.find(fixedName);
		bWeAreReading = true;
		if(it != FS::fs->files.end()) {
			string path = it->second->searchpath + fixedName;
			if(it->second->handle) return NULL;
			it->second->handle = fopen(path.c_str(), mode.c_str());
			return it->second;
		}
	}

	// If we are writing, we need to reverse the searchpath list (so we write to homepath first)
	vector<string> searchpaths = FS::GetSearchPaths();
	if(!bWeAreReading)
		reverse(searchpaths.begin(), searchpaths.end());

	for(auto it = searchpaths.begin(); it != searchpaths.end(); ++it) {
		// if file found in this search path, good to go
		string path = *it + fixedName;
		if(!bWeAreReading) {
			// If we are writing, make sure that the folder exists
			FS::fs->RecursivelyTouch(path);
		}
		FILE* file = fopen(path.c_str(), mode.c_str());
		if(file) {
			File *F = (File*)Zone::New<File>(Zone::TAG_FILES);
			F->searchpath = *it;
			F->handle = file;
			FS::fs->files[fixedName] = F;
			return F;
		}
	}

	return NULL;
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
		numChars = GetSize();
	}
	if(numChars == 0) { // blank!
		return "";
	}
	char* buf = (char*)Zone::Alloc(sizeof(char)*numChars+1, Zone::TAG_FILES);
	fread(buf, sizeof(char), numChars, handle);
	buf[numChars] = '\0';
	string retval = buf;
	Zone::FastFree(buf, "files");
	return retval;
}

size_t File::ReadBinary(unsigned char* bytes, size_t numBytes) {
	if(!handle) return NULL;
	if(numBytes == 0) { // reset cursor to beginning of file and read whole thing
		fseek(handle, 0L, SEEK_SET);
		numBytes = GetSize()/sizeof(unsigned char);
	}
	return fread(bytes, sizeof(unsigned char), numBytes, handle);
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
	Zone::FastFree(buf, "files");
	return retval;
}

size_t File::GetSize() {
	size_t currentPos = ftell(handle);
	fseek(handle, 0L, SEEK_END);
	size_t size = ftell(handle);
	fseek(handle, currentPos, SEEK_SET);
	return size;
}

string File::GetFileSearchPath(const string& fileName) {
	// Get a file's search path without opening the file itself.
	// This is extremely handy in the case of Awesomium, where it handles files on its own.

	// Trim off any leading (or trailing) whitespace
	string fixedName = trim(fileName);
	// Now make sure we start with a '/'
	if(fixedName[0] != '/')
		fixedName = '/' + fixedName;
	reverse(FS::fs->searchpaths.begin(), FS::fs->searchpaths.end());
	for(auto it = FS::fs->searchpaths.begin(); it != FS::fs->searchpaths.end(); ++it) {
		string path = *it + fixedName;
		FILE* f = fopen(path.c_str(), "r");
		if(f) {
			fclose(f);
			reverse(FS::fs->searchpaths.begin(), FS::fs->searchpaths.end());
			return path;
		}
	}
	reverse(FS::fs->searchpaths.begin(), FS::fs->searchpaths.end());
	return "";
}
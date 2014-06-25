#include "sys_local.h"

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

void FileSystem::Init() {
	R_Printf("fs_init\n");
	fs_core = CvarSystem::RegisterCvar("fs_core", "Core directory; contains all essential game data.", (1 << Cvar::CVAR_ROM), "core");
	fs_homepath = CvarSystem::RegisterCvar("fs_homepath", "homepath", (1 << Cvar::CVAR_ROM), Sys_FS_GetBasepath());
	fs_modlist = CvarSystem::RegisterCvar("fs_modlist", "List of active mods", (1 << Cvar::CVAR_ROM), "");
	fs_basepath = CvarSystem::RegisterCvar("fs_basepath", "Directory to gamedata", (1 << Cvar::CVAR_ROM), Sys_FS_GetBasepath());

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

void FileSystem::Shutdown() {
	delete fs;
}

void FileSystem::CreateModSearchPaths(const string& basepath, const string& modlist) {
	if(modlist.length() < 1) {
		return; // blank modlist
	}
	vector<string> mods;
	split(modlist, ';', mods);
	for(auto it = mods.begin(); it != mods.end(); ++it) {
		fs->AddCoreSearchPath(fs_basepath->String(), *it);
	}
}

void FileSystem::PrintSearchPaths() {
	R_Printf("All search paths:\n");
	for_each(searchpaths.begin(), searchpaths.end(), [](string& s) {
		R_Printf("%s\n", s.c_str());
	});
}

// Make sure to free mem after this call!
char** FileSystem::ListFiles(const string& dir, const char* extension, int* iNumFiles) {
	DIR* xdir;
	struct dirent *ent;
	string fixedDir;
	vector<string> fileNames;

	if(!iNumFiles) {
		return NULL;
	}
	*iNumFiles = 0;
	if(strlen(extension) <= 0) {
		return NULL;
	}

	if(dir.back() == '\\' || dir.back() == '/') {
		fixedDir = dir;
	} else {
		fixedDir = dir + '/';
	}
	auto x = fs->GetSearchPaths();
	for(auto it = x.begin(); it != x.end(); ++it) {
		string compString = *it + '/' + dir; // hm, this'll be searchpath + search dir
		if((xdir = opendir(compString.c_str())) != NULL) {
			// loop through all files
			while(1) {
				ent = readdir(xdir);
				if(ent == NULL) {
					break;
				}
				if(!checkExtension(ent->d_name, extension)) {
					continue;
				}
				stringstream fName;
				fName << fixedDir << ent->d_name;
				fName << '\0';
				fileNames.push_back(fName.str());
				(*iNumFiles)++;
			}
			closedir(xdir);
		}
	}

	if(*iNumFiles == 0) {
		return NULL;
	}

	char** ptStrList = (char**)Zone::Alloc(sizeof(char*) * (*iNumFiles), Zone::TAG_FILES);
	int i = 0;
	for(auto it = fileNames.begin(); it != fileNames.end(); ++it) {
		ptStrList[i] = (char*)Zone::Alloc(it->length(), Zone::TAG_FILES);
		strcpy(ptStrList[i], it->c_str());
		i++;
	}
	return ptStrList;
}

void FileSystem::FreeFileList(char** ptFileList, int iNumItems) {
	if(ptFileList == NULL) {
		return;
	}
	for(int i = 0; i < iNumItems; i++) {
		Zone::FastFree(ptFileList[i], "files");
	}
	Zone::FastFree(ptFileList, "files");
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

File* FileSystem::EXPORT_OpenFile(const char* filename, const char* mode) {
	return File::Open(filename, mode);
}

void FileSystem::EXPORT_Close(File* filehandle) {
	filehandle->Close();
}

char** FileSystem::EXPORT_ListFilesInDir(const char* filename, const char* ext, int *iNumFiles) {
	return fs->ListFiles(filename, ext, iNumFiles);
}

string FileSystem::EXPORT_ReadPlaintext(File* f, size_t numChars) {
	return f->ReadPlaintext(numChars);
}

size_t FileSystem::EXPORT_ReadBinary(File* f, unsigned char* bytes, size_t numBytes, const bool bDontResetCursor) {
	return f->ReadBinary(bytes, numBytes, bDontResetCursor);
}

size_t FileSystem::EXPORT_GetFileSize(File* f) {
	return f->GetSize();
}

File* File::Open(const string& fileName, const string& mode) {
	// Trim off any leading (or trailing) whitespace
	string fixedName = trim(fileName);
	// Replace all instances of \\ with / (Windows fix)
	stringreplace(fixedName, "\\", "/");
	// Now make sure we start with a '/'
	if(fixedName[0] != '/') {
		fixedName = '/' + fixedName;
	}

	// If a file has been opened before, we can open it again using the same search path as we did previously.
	bool bWeAreReading = false;
	if(mode.find('w') == string::npos && mode.find('a') == string::npos) {
		// We don't do this when writing (because we always write to homepath)
		auto it = fs->files.find(fixedName);
		bWeAreReading = true;
		if(it != fs->files.end()) {
			string path = it->second->searchpath + fixedName;
			if(it->second->handle) return NULL;
			it->second->handle = fopen(path.c_str(), mode.c_str());
			return it->second;
		}
	}

	// If we are writing, we need to reverse the searchpath list (so we write to homepath first)
	vector<string> searchpaths = fs->GetSearchPaths();
	if(!bWeAreReading) {
		reverse(searchpaths.begin(), searchpaths.end());
	}

	for(auto it = searchpaths.begin(); it != searchpaths.end(); ++it) {
		// if file found in this search path, good to go
		string path = *it + fixedName;
		if(!bWeAreReading) {
			// If we are writing, make sure that the folder exists
			fs->RecursivelyTouch(path);
		}
		FILE* file = fopen(path.c_str(), mode.c_str());
		if(file) {
			File *F = (File*)Zone::New<File>(Zone::TAG_FILES);
			F->searchpath = *it;
			F->handle = file;
			fs->files[fixedName] = F;
			return F;
		}
	}

	return NULL;
}

void File::Close() {
	if(!handle) { return; }
	fclose(handle);
	handle = NULL;
}

string File::ReadPlaintext(size_t numChars) {
	if(!handle) { return ""; }
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

size_t File::ReadBinary(unsigned char* bytes, size_t numBytes, const bool bDontResetCursor) {
	if(!handle) return NULL;
	if(numBytes == 0 && !bDontResetCursor) { // reset cursor to beginning of file and read whole thing
		fseek(handle, 0L, SEEK_SET);
		numBytes = GetSize()/sizeof(unsigned char);
	} else if(numBytes == 0) { // DONT reset cursor, but read the whole thing
		int s = ftell(handle);
		fseek(handle, 0L, SEEK_SET);
		size_t totalChars = GetSize();
		fseek(handle, s, SEEK_SET);
		numBytes = totalChars - s;
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
	reverse(fs->searchpaths.begin(), fs->searchpaths.end());
	for(auto it = fs->searchpaths.begin(); it != fs->searchpaths.end(); ++it) {
		string path = *it + fixedName;
		FILE* f = fopen(path.c_str(), "r");
		if(f) {
			fclose(f);
			reverse(fs->searchpaths.begin(), fs->searchpaths.end());
			return path;
		}
	}
	reverse(fs->searchpaths.begin(), fs->searchpaths.end());
	return "";
}
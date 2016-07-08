#include "sys_local.h"

File::File() {
	flags = 0;
	path = "";
	mode = "";
	fp = nullptr;
}

File::File(const File& other) {
	flags = other.flags;
	fp = other.fp;
	path = other.path;
	mode = other.mode;
}

File* File::OpenAsync(const char* file, const char* mode, fileOpenedCallback callback) {
	File* pFile = new File();
	pFile->mode = mode;
	Filesystem::ResolveFilePath(pFile->path, file, mode);

	Filesystem::QueueFileOpen(pFile, callback);
	return pFile;
}

void File::ReadAsync(File* pFile, void* data, size_t dataSize, fileReadCallback callback) {
	if (pFile == nullptr) {
		return;
	}
	Filesystem::QueueFileRead(pFile, data, dataSize, callback);
}

void File::WriteAsync(File* pFile, void* data, size_t dataSize, fileWrittenCallback callback) {
	if (pFile == nullptr) {
		return;
	}
	Filesystem::QueueFileWrite(pFile, data, dataSize, callback);
}

void File::CloseAsync(File* pFile, fileClosedCallback callback) {
	if (pFile == nullptr) {
		return;
	}
	Filesystem::QueueFileClose(pFile, callback);
}

bool File::AsyncOpened(File* pFile) {
	if (!(pFile->flags & File_Opened)) {
		return false;
	}
	pFile->flags &= ~File_Opened;
	return true;
}

bool File::AsyncRead(File* pFile) {
	if (!(pFile->flags & File_Read)) {
		return false;
	}
	pFile->flags &= ~File_Read;
	return true;
}

bool File::AsyncWritten(File* pFile) {
	if (!(pFile->flags & File_Written)) {
		return false;
	}
	pFile->flags &= ~File_Written;
	return true;
}

bool File::AsyncClosed(File* pFile) {
	if (!(pFile->flags & File_Closed)) {
		return false;
	}
	pFile->flags &= ~File_Closed;
	return true;
}

bool File::AsyncBad(File* pFile) {
	if (!(pFile->flags & File_Bad)) {
		return false;
	}
	pFile->flags &= ~File_Bad;
	return true;
}

File* File::OpenSync(const char* file, const char* mode) {
	File* pFile = new File();
	if (pFile == nullptr) {
		return pFile;
	}
	Filesystem::ResolveFilePath(pFile->path, file, mode);
	pFile->mode = mode;
	pFile->fp = fopen(pFile->path.c_str(), mode);
	if (pFile->fp == nullptr) {
		delete pFile;
		return nullptr;
	}
	pFile->flags |= File_Opened;
	return pFile;
}

bool File::ReadSync(File* pFile, void* data, size_t dataSize) {
	if (pFile == nullptr || pFile->fp == nullptr) {
		return false;
	}
	memset(data, 0, dataSize);	// Make it null-terminated by default.
	size_t read = fread(data, 1, dataSize, pFile->fp);
	if (read <= 0) {
		pFile->flags |= File_Bad;
		return false;
	}
	pFile->flags |= File_Read;
	return true;
}

bool File::WriteSync(File* pFile, void* data, size_t dataSize) {
	if (pFile == nullptr || pFile->fp == nullptr) {
		return false;
	}
	size_t written = fwrite(data, 1, dataSize, pFile->fp);
	if (written <= 0) {
		pFile->flags |= File_Bad;
		return false;
	}
	pFile->flags |= File_Written;
	return true;
}

bool File::CloseSync(File* pFile) {
	if (pFile == nullptr || pFile->fp == nullptr) {
		return false;
	}
	fclose(pFile->fp);
	pFile->fp = nullptr;
	pFile->flags |= File_Closed;
	pFile->flags &= ~(File_Read | File_Written | File_Opened);
	delete pFile;
	return true;
}

/*
Gets run whenever the filesystem deques an open command on this file.
The callback is run after the open command has successfully completed.
*/
void File::DequeOpen(fileOpenedCallback callback) {
	this->fp = fopen(this->path.c_str(), this->mode.c_str());
	if (this->fp == nullptr) {
		this->flags |= File_Bad;
		return;	// Don't run the callback if we failed
	}
	if (callback) {
		callback(this);
	}
	this->flags |= File_Opened;
}

/*
Gets run whenever the filesystem deques a read command on this file.
The callback is run after the read command has completed successfully.
*/
void File::DequeRead(void* data, size_t dataSize, fileReadCallback callback) {
	if (this->fp == nullptr) {
		this->flags |= File_Bad;
		return;
	}
	memset(data, 0, dataSize);	// Null-terminate it by default
	size_t read = fread(data, 1, dataSize, this->fp);
	if (read == 0) {
		this->flags |= File_Bad;
		return;
	}
	if (callback) {
		callback(this, data, dataSize);
	}
	this->flags |= File_Read;
}

/*
Gets run whenever the filesystem deques a write command on this file.
The callback is run after the write command is complete.
*/
void File::DequeWrite(void* data, size_t dataSize, fileWrittenCallback callback) {
	if (this->fp == nullptr) {
		this->flags |= File_Bad;
		return;
	}
	size_t written = fwrite(data, 1, dataSize, this->fp);
	if (written == 0) {
		this->flags |= File_Bad;
		return;
	}
	if (callback) {
		callback(this, data, dataSize);
	}
	this->flags |= File_Written;
}

/*
Gets run whenever the filesystem deques a close command on this file.
The callback is run after the close command is complete.
*/
void File::DequeClose(fileClosedCallback callback) {
	fclose(this->fp);
	if (callback) {
		callback(this);
	}
	flags &= ~(File_Read | File_Written);
	flags |= File_Closed;
}
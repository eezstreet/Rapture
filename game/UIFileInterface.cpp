#include "ui_local.h"

using namespace Rocket::Core;

FileHandle UIFileInterface::Open(const String& path) {
	File* ff = File::Open(path.CString(), "r");
	if(!ff) return NULL;
	return (FileHandle)ff;
}

void UIFileInterface::Close(FileHandle file) {
	File* f = (File*)file;
	f->Close();
}

size_t UIFileInterface::Read(void* buffer, size_t size, FileHandle file) {
	File* f = (File*)file;
	return f->ReadBinary((unsigned char*)buffer, size);
}

bool UIFileInterface::Seek(FileHandle file, long offset, int origin) {
	File* f = (File*)file;
	return f->Seek(offset, origin);
}

size_t UIFileInterface::Tell(FileHandle file) {
	File* f = (File*)file;
	return f->Tell();
}


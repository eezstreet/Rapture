#include "tr_local.h"

ImageClass::ImageClass() {
}

ImageClass::ImageClass(unsigned char* pixels, unsigned int numPixels) :
	pixBuffer(pixels),
	numPixInBuffer(numPixels) {
}

bool ImageClass::WriteToFile(const string& filename) {
	FREE_IMAGE_FORMAT f = DetermineFileFormat(filename);
	FIMEMORY *fMem = FreeImage_OpenMemory(pixBuffer, numPixInBuffer);
	auto inf = FreeImage_GetFileTypeFromMemory(fMem, 0);
	auto img = FreeImage_LoadFromMemory(inf, fMem);
	if(!img) {
		return false;
	}
	DWORD byteSize = 0;
	BYTE* memBuffer = NULL;
	auto fMem2 = FreeImage_OpenMemory();
	FreeImage_SaveToMemory(f, img, fMem2);
	FreeImage_Unload(img);
	FreeImage_AcquireMemory(fMem2, &memBuffer, &byteSize);
	File* file = File::Open(filename, "wb+");
	if(!file) {
		return false;
	}
	file->WriteBinary(memBuffer, byteSize);
	file->Close();
	FreeImage_CloseMemory(fMem);
	FreeImage_CloseMemory(fMem2);
	return true;
}

FREE_IMAGE_FORMAT ImageClass::DetermineFileFormat(const string& filename, bool bJustExtension /* = false */) {
	string sExt = filename;
	if(!bJustExtension) {
		sExt = filename.substr(filename.find_last_of('.'));
	}
	if(!sExt.compare(".bmp")) {
		return FIF_BMP;
	}
	else if(!sExt.compare(".dds")) {
		return FIF_DDS;
	}
	else if(!sExt.compare(".gif")) {
		return FIF_GIF;
	}
	else if(!sExt.compare(".ico") || !sExt.compare(".icon")) {
		return FIF_ICO;
	}
	else if(!sExt.compare(".jpg") || !sExt.compare(".jpeg")) {
		return FIF_JPEG;
	}
	else if(!sExt.compare(".pcx")) {
		return FIF_PCX;
	}
	else if(!sExt.compare(".png")) {
		return FIF_PNG;
	}
	else if(!sExt.compare(".psd")) {
		return FIF_PSD;
	}
	else if(!sExt.compare(".raw")) {
		return FIF_RAW;
	}
	else if(!sExt.compare(".tga") || !sExt.compare(".targa")) {
		return FIF_TARGA;
	}
	else if(!sExt.compare(".tiff") || !sExt.compare(".tif")) {
		return FIF_TIFF;
	}
	else {
		return FIF_UNKNOWN;
	}
}

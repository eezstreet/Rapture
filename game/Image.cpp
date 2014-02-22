#include "tr_local.h"

unordered_map<string, ImageClass*> ImageClass::registeredImages;
unordered_map<string, FREE_IMAGE_FORMAT> ImageClass::formats;

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

void ImageClass::InitImages() {
	formats.insert(make_pair(".bmp", FIF_BMP));
	formats.insert(make_pair(".dds", FIF_DDS));
	formats.insert(make_pair(".gif", FIF_GIF));
	formats.insert(make_pair(".ico", FIF_ICO));
	formats.insert(make_pair(".icon", FIF_ICO));
	formats.insert(make_pair(".jpg", FIF_JPEG));
	formats.insert(make_pair(".jpeg", FIF_JPEG));
	formats.insert(make_pair(".pcx", FIF_PCX));
	formats.insert(make_pair(".png", FIF_PNG));
	formats.insert(make_pair(".psd", FIF_PSD));
	formats.insert(make_pair(".raw", FIF_RAW));
	formats.insert(make_pair(".tga", FIF_TARGA));
	formats.insert(make_pair(".targa", FIF_TARGA));
	formats.insert(make_pair(".tiff", FIF_TIFF));
	formats.insert(make_pair(".tif", FIF_TIFF));
}

void ImageClass::ShutdownImages() {
	formats.clear();
	// I was originally thinking about clearing the zone tag for images at this point,
	// but it makes more sense to clear it at program termination because that saves us
	// from having to reload the images if we do a vid_restart.
}

void* ImageClass::RegisterImage(const string& filename) {
	ImageClass* img = Zone::New<ImageClass>(Zone::TAG_IMAGES);
	string filepath = stripextension(filename);
	if(registeredImages.find(filepath) != registeredImages.end()) {
		return registeredImages[filepath];
	}
	bool bWeLoadedSuccessfully = false;
	for(auto it = ImageClass::formats.begin(); it != ImageClass::formats.end(); ++it) {
		string teststr = filepath;
		teststr.append(it->first);
		string path = File::GetFileSearchPath(teststr);
		if(!path.compare("")) {
			// Didn't find a file with this extension, try the next one
			continue;
		}
		img->format = it->second;
		if(img->ParseImage(path)) {
			bWeLoadedSuccessfully = true;
			break;
		}
	}
	if(!bWeLoadedSuccessfully) {
		Zone::FastFree(img, "images");
		return NULL;
	}
	else {
		registeredImages[filepath] = img;
		return img;
	}

}

bool ImageClass::ParseImage(const string& filename) {
	image = FreeImage_Load(format, filename.c_str());
	if(!image) {
		return false;
	}
	image = FreeImage_ConvertTo32Bits(image);
	if(!image) {
		return false;
	}
	return true;
}

void ImageClass::RenderImage(void* image, float xPct, float yPct, float wPct, float hPct) {
	assert(image);
	if(!image) {
		return;
	}
	ImageClass* ri = (ImageClass*)image;
	SDL_Surface* surf = SDL_CreateRGBSurfaceFrom(FreeImage_GetBits(ri->image), FreeImage_GetWidth(ri->image), FreeImage_GetHeight(ri->image),
		32, FreeImage_GetPitch(ri->image), FreeImage_GetRedMask(ri->image), FreeImage_GetGreenMask(ri->image), FreeImage_GetBlueMask(ri->image),
		0x000000FF);
	SDL_Rect rect;
	rect.w = wPct*r_width->Integer();
	rect.h = hPct*r_height->Integer();
	rect.x = xPct*r_width->Integer();
	rect.y = yPct*r_height->Integer();
	SDL_SetClipRect(surf, &rect);
	RenderCode::AddSurface((void*)surf);
}
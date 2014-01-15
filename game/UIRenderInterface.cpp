#include "ui_local.h"

using namespace Rocket::Core;

void UIRenderInterface::RenderGeometry(Vertex* vertices, int num_vertices, int* indices, int num_indices, 
		TextureHandle texture, const Vector2f& translation) {

	SDL_Renderer* r = (SDL_Renderer*)RenderCode::GetRenderer();
	SDL_RenderGeometry(r, (SDL_Texture*)texture, (SDL_Vertex*)vertices, num_vertices, indices, num_indices, (SDL_Vector2f*)&translation);
}

void UIRenderInterface::EnableScissorRegion(bool enable) {
	SDL_Renderer* r = (SDL_Renderer*)RenderCode::GetRenderer();
	if(enable)
		SDL_EnableScissor(r);
	else
		SDL_DisableScissor(r);
}

void UIRenderInterface::SetScissorRegion(int x, int y, int width, int height) {
	SDL_Renderer* r = (SDL_Renderer*)RenderCode::GetRenderer();
	SDL_Rect rect;
	rect.x = x; rect.y = y; rect.w = width; rect.h = height;
	SDL_ScissorRegion(r, &rect);
}

bool UIRenderInterface::LoadTexture(TextureHandle& handle, Vector2i& dimensions, const String& source) {
	File* f = File::Open(source.CString(), "r");
	if(!f) return false;

	Rocket::Core::byte* src = (Rocket::Core::byte*)Zone::Alloc(f->GetSize(), Zone::TAG_FILES);
	size_t read = f->ReadBinary(src);
	SDL_RWops* rw = SDL_RWFromMem(src, read);
	SDL_Surface* surf = IMG_Load_RW(rw, 1);
	if(!surf) {
		printf(SDL_GetError());
		return false;
	}
	int w = surf->w;
	int h = surf->h;
	SDL_Texture* tex = (SDL_Texture*)RenderCode::TexFromSurface(surf);

	handle = (TextureHandle)tex;
	dimensions = Vector2i(w, h);

	return true;
}

bool UIRenderInterface::GenerateTexture(TextureHandle& texture_handle, const Rocket::Core::byte* source, const Vector2i& source_dimensions) {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    Uint32 iRMask = 0xff000000;
    Uint32 iGMask = 0x00ff0000;
    Uint32 iBMask = 0x0000ff00;
    Uint32 iAMask = 0x000000ff;
#else
    Uint32 iRMask = 0x000000ff;
    Uint32 iGMask = 0x0000ff00;
    Uint32 iBMask = 0x00ff0000;
    Uint32 iAMask = 0xff000000;
#endif
	SDL_Surface* surf = SDL_CreateRGBSurfaceFrom((void*)source, source_dimensions.x, source_dimensions.y, 32, source_dimensions.x*4,
		iRMask, iGMask, iBMask, iAMask);
	SDL_Texture* tex = (SDL_Texture*)RenderCode::TexFromSurface(surf);
	SDL_FreeSurface(surf);
	texture_handle = (TextureHandle)tex;
	return true;
}

void UIRenderInterface::ReleaseTexture(TextureHandle handle) {
	SDL_DestroyTexture((SDL_Texture*)handle);
}
#include "ui_local.h"

using namespace Rocket::Core;

void UIRenderInterface::RenderGeometry(Vertex* vertices, int num_vertices, int* indices, int num_indices, 
		TextureHandle texture, const Vector2f& translation) {
	glPushMatrix();
	glTranslatef(translation.x, translation.y, 0);

	vector<Vector2f> p(num_vertices);
	vector<Colourb> c(num_vertices);
	vector<Vector2f> t(num_vertices);

	for(int i = 0; i < num_vertices; i++) {
		p[i] = vertices[i].position;
		c[i] = vertices[i].colour;
		t[i] = vertices[i].tex_coord;
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(2, GL_FLOAT, 0, &p[0]);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, &c[0]);
	glTexCoordPointer(2, GL_FLOAT, 0, &t[0]);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	SDL_Texture* tex = (SDL_Texture*)texture;
	if(tex)
		SDL_GL_BindTexture(tex, NULL, NULL);
	else {
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, indices);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glColor4f(1, 1, 1, 1);

	glPopMatrix();
}

void UIRenderInterface::EnableScissorRegion(bool enable) {
	//if(enable)
	//	glEnable(GL_SCISSOR_TEST);
	//else
	//	glDisable(GL_SCISSOR_TEST);
}

void UIRenderInterface::SetScissorRegion(int x, int y, int width, int height) {
	//int iWindowWidth, iWindowHeight;
	//RenderCode::GetWindowSize(&iWindowWidth, &iWindowHeight);
	//glScissor(x, iWindowHeight - (y + iWindowHeight), width, height);
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
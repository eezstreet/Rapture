#pragma once
#include "tr_local.h"
#include "sys_local.h"

#include <Rocket/Core.h>
#include <Rocket/Controls.h>

class UIFileInterface : public Rocket::Core::FileInterface {
	virtual Rocket::Core::FileHandle Open(const Rocket::Core::String& path);
	virtual void Close(Rocket::Core::FileHandle file);
	virtual size_t Read(void* buffer, size_t size, Rocket::Core::FileHandle file);
	virtual bool Seek(Rocket::Core::FileHandle file, long offset, int origin);
	virtual size_t Tell(Rocket::Core::FileHandle file);
};

class UISystemInterface : public Rocket::Core::SystemInterface {
	virtual float GetElapsedTime();
	// TODO: write functions for TranslateString() and LogMessage()
};

class UIRenderInterface : public Rocket::Core::RenderInterface {
	virtual void RenderGeometry(Rocket::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, 
		Rocket::Core::TextureHandle texture, const Rocket::Core::Vector2f& translation);
	virtual void EnableScissorRegion(bool enable);
	virtual void SetScissorRegion(int x, int y, int width, int height);
	virtual bool LoadTexture(Rocket::Core::TextureHandle& texture_handle, Rocket::Core::Vector2i& texture_dimensions, 
		const Rocket::Core::String& source);
	virtual bool GenerateTexture(Rocket::Core::TextureHandle& texture_handle, const Rocket::Core::byte* source,
		const Rocket::Core::Vector2i& source_dimensions);
	virtual void ReleaseTexture(Rocket::Core::TextureHandle texture_handle);
};
#include "tr_local.h"

// The Renderer is never directly accessed from any gamecode, instead use a Video:: call to access it.
static renderImports_s refExports = {
	&R_Message,
	&File::OpenAsync,
	&File::ReadAsync,
	&File::WriteAsync,
	&File::CloseAsync,
	&File::AsyncOpened,
	&File::AsyncRead,
	&File::AsyncWritten,
	&File::AsyncClosed,
	&File::AsyncBad,
	&File::OpenSync,
	&File::ReadSync,
	&File::WriteSync,
	&File::CloseSync,
	&Resource::ResourceAsync,
	&Resource::ResourceAsyncURI,
	&Resource::ResourceSync,
	&Resource::ResourceSyncURI,
	&Resource::FreeResource,
	&Resource::GetAssetComponent,
	&Filesystem::ResolveFilePath,
	&Filesystem::ResolveAssetPath,
	&Video::GetWindowInfo,
	&Video::GetRaptureWindow,
	&CvarSystem::RegisterStringVar,
	&CvarSystem::RegisterIntegerVar,
	&CvarSystem::RegisterFloatVar,
	&CvarSystem::RegisterBooleanVar,
	&CvarSystem::GetStringValue,
	&CvarSystem::GetIntegerValue,
	&CvarSystem::GetFloatValue,
	&CvarSystem::GetBooleanValue,
};

typedef renderExports_s* (*refapi_t)(renderImports_s*);
Renderer::Renderer(const char* renderName) : bExportsValid(false) {
	string fullRendName = "rend_";
	fullRendName += renderName;

	strcpy(szRenderName, renderName);

	R_Message(PRIORITY_MESSAGE, "Loading renderer '%s'\n", fullRendName.c_str());
	renderModule = Sys_LoadLibrary(fullRendName);
	if (renderModule == NULL) {
		R_Message(PRIORITY_ERRFATAL, "Failed to load renderer\n");
		return;
	}

	ptModuleFunction ptGetRefAPI = Sys_GetFunctionAddress(renderModule, "GetRefAPI");
	if (ptGetRefAPI == nullptr) {
		R_Message(PRIORITY_ERRFATAL, "GetRefAPI failed on renderer %s\n", fullRendName.c_str());
		return;
	}

	refapi_t entry = (refapi_t)ptGetRefAPI;
	pExports = entry(&refExports);
	bExportsValid = true;
}
#include "tr_local.h"

// The Renderer is never directly accessed from any gamecode, instead use a Video:: call to access it.
static renderImports_s refExports = {
	&R_Message,
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
	&File::Open,
	&File::CloseFile,
	&File::WriteFilePlaintext,
	&File::GetFileSearchPathISO,
	&FontManager::GetFontTTF
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
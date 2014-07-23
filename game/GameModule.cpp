#include "sys_local.h"

GameModule::GameModule(string module) {
	ptModuleHandle = Sys_LoadLibrary(module);
	if(ptModuleHandle == nullptr) {
		R_Message(PRIORITY_ERRFATAL, "FATAL: GameModule failed to load\n");
	}
}

GameModule::~GameModule() {
	Sys_FreeLibrary(ptModuleHandle);
}

typedef gameExports_s* (*refapi_t)(gameImports_s*);
gameExports_s* GameModule::GetRefAPI(gameImports_s* import) {
	ptModuleFunction call = Sys_GetFunctionAddress(ptModuleHandle, "GetRefAPI");
	if(!call) {
		return nullptr;
	}
	refapi_t gamecall = (refapi_t)call;
	return gamecall(import);
}
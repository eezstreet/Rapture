#include "sys_local.h"

GameModule::GameModule(string module) {
	ptModuleHandle = Sys_LoadLibrary(module);
	if(ptModuleHandle == NULL) {
		R_Printf("FATAL: GameModule failed to load\n");
	}
}

GameModule::~GameModule() {
}

typedef gameExports_s* (*refapi_t)(gameImports_s*);
gameExports_s* GameModule::GetRefAPI(gameImports_s* import) {
	ptModuleFunction call = Sys_GetFunctionAddress(ptModuleHandle, "GetRefAPI");
	if(!call) {
		return NULL;
	}
	refapi_t gamecall = (refapi_t)call;
	return gamecall(import);
}
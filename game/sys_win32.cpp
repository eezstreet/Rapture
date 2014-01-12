#include "sys_local.h"
#include <direct.h>
#include <process.h>

char* Sys_FS_GetHomepath() {
	// not sure
	return NULL;
}

char* Sys_FS_GetBasepath() {
	static char cwd[1024];
	_getcwd(cwd, 1023);
	cwd[1023] = '\0';
	return cwd;
}

void Sys_RunThread(void (*threadRun)(void*), void* arg) {
	_beginthread(threadRun, 0, arg);
}


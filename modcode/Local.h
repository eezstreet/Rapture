#pragma once
#include <sys_shared.h>

extern gameImports_s* trap;
#define R_Message trap->printf
#define R_Error trap->error

void eswap(unsigned short &x);
void eswap(unsigned int &x);
string genuuid();
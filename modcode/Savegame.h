#pragma once
#include "sys_shared.h"

#define SAVEGAME_HEAD	"RSAV"
#define SAVEGAME_VER	1
#define SAVE_DIRECTORY	"save"
#define MAX_SAVE_PATH	64

struct Savegame : Rapture_Savegame {

	// Interface
	static Savegame Retrieve(const char* path);
	static void Save(Savegame& out, const char* path);
};
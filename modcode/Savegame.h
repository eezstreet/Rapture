#pragma once
#include "sys_shared.h"

#define SAVEGAME_HEAD	"RSAV"
#define SAVEGAME_VER	1
#define SAVE_DIRECTORY	"save"
#define MAX_SAVE_PATH	64

struct Savegame {
	struct Savegame_Header {
		char		header[5];
		uint8_t		version;

		Rapture_TimeDate	lastUseTime;	// Last time this character was used
		Rapture_TimeDate	creationTime;	// When this character was created
		Rapture_TimeDate	playTime;		// How long this character has been played for
	};

	Savegame_Header		head;			// Head of the savegame

	// Interface
	static Savegame Retrieve(const char* path);
	static void Save(Savegame& out, const char* path);
};
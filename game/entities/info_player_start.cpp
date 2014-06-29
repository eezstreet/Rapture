#include "g_local.h"

#define SPAWNFLAG_VIS0				1
#define SPAWNFLAG_VIS1				2
#define SPAWNFLAG_VIS2				4
#define SPAWNFLAG_VIS3				8
#define SPAWNFLAG_VIS4				16
#define SPAWNFLAG_VIS5				32
#define SPAWNFLAG_VIS6				64
#define SPAWNFLAG_VIS7				128
#define SPAWNFLAG_PORTAL			256
#define SPAWNFLAG_PLAYSPAWN			512

void info_player_start::spawn() {
	// Don't need to worry about spawning, these are actually pretty passive
}

void info_player_start::think() {
	// These don't really need to think.
}

Entity* info_player_start::spawnme(float x, float y, int spawnflags, int act) {
	info_player_start* ent = new info_player_start();
	ent->x = x;
	ent->y = y;
	ent->spawnflags = spawnflags;
	ent->uuid = genuuid();
	ent->classname = "info_player_start";

	ent->bShouldWeCollide = false;
	ent->bShouldWeRender = false;
	ent->bShouldWeThink = false;
	ent->iAct = act;

	return ent;
}
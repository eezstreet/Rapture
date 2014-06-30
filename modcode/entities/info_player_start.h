#pragma once
#include "Entity.h"
#include "Local.h"

// Other entities..
class info_player_start : public Entity {
public:
	enum {
		SPAWNFLAG_VIS0 = 1,
		SPAWNFLAG_VIS1 = 2,
		SPAWNFLAG_VIS2 = 4,
		SPAWNFLAG_VIS3 = 8,
		SPAWNFLAG_VIS4 = 16,
		SPAWNFLAG_VIS5 = 32,
		SPAWNFLAG_VIS6 = 64,
		SPAWNFLAG_VIS7 = 128,
		SPAWNFLAG_PORTAL = 256,
		SPAWNFLAG_PLAYSPAWN = 512
	};

	virtual void render() { }
	virtual void think();
	virtual void spawn();

	static Entity* spawnme(float x, float y, int spawnflags, int act);
};
#include "contain_chest.h"

void contain_chest::spawn() {
}

void contain_chest::think() {
}

void contain_chest::render() {
}

void contain_chest::Open() {
}

Entity* contain_chest::spawnme(float x, float y, int spawnflags, int act) {
	contain_chest* ent = new contain_chest();
	ent->x = x;
	ent->y = y;
	ent->spawnflags = spawnflags;
	ent->uuid = genuuid();
	ent->classname = "contain_chest";

	ent->bShouldWeCollide = true;	// We can't collide with a chest once it's open, though.
	ent->bShouldWeRender = true;
	ent->bShouldWeThink = false;	// Chests don't need to think
	ent->iAct = act;
	return ent;
}
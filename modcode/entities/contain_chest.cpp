#include "contain_chest.h"
#include "Server.h"
#include "Worldspace.h"
#include "DungeonManager.h"

void contain_chest::spawn() {
	int originX = Worldspace::WorldPlaceToScreenSpaceIX(x, y);
	int originY = Worldspace::WorldPlaceToScreenSpaceIY(x, y);
	bIsOpen = false;
}

void contain_chest::think() {
	if(bIsOpen) {
		return;
	}
}

bool contain_chest::mouseover() {
	if(!bIsOpen) {
		ptServer->GetClient()->StartLabelDraw("Chest");
		ptServer->GetClient()->ptFocusEnt = this;
		return true;
	} else {
		return false;
	}
}

void contain_chest::render() {
	int renderPosX = Entity::GetDrawX(this);
	int renderPosY = Entity::GetDrawY(this);
}

void contain_chest::interact(Entity* interacter) { 
	RVec2<float> ourOrigin(x, y);
	RVec2<float> interacterOrigin(interacter->x, interacter->y);
	if(interacterOrigin.Within(ourOrigin, 1.0f)) {
		// Don't allow use when interacter is stupidly far away..one tile sounds good
		Open();
	}
}

void contain_chest::Open() {
	bIsOpen = true;

	// Kill collision detection
	w = 0;
	h = 0;

	// TODO: evaluate and drop treasure
}

Entity* contain_chest::spawnme(float x, float y, int spawnflags, int act) {
	contain_chest* ent = new contain_chest();
	ent->x = x;
	ent->y = y;
	ent->w = 0.3f;
	ent->h = 0.3f;
	ent->spawnflags = spawnflags;
	ent->uuid = genuuid();
	ent->classname = "contain_chest";

	ent->bShouldWeCollide = true;	// We can't collide with a chest once it's open, though.
	ent->bShouldWeRender = true;
	ent->bShouldWeThink = true;
	ent->iAct = act;
	return ent;
}
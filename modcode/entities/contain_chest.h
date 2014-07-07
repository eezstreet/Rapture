#pragma once
#include "Entity.h"
#include "Container.h"

class contain_chest : public Container {
public:
	virtual void render();
	virtual void think();
	virtual void spawn();
	virtual void interact(Entity* interacter);
	virtual bool mouseover();

	virtual void Open();

	static Entity* spawnme(float x, float y, int spawnflags, int act);
};
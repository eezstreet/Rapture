#pragma once
#include "Entity.h"
#include "Local.h"

// Actors have visible representations, whereas regular entities do not.
class Actor : public Entity {
public:
	float GetPreviousX() const { return pX; }
	float GetPreviousY() const { return pY; }
protected:
	Material* materialHandle;
	float pX, pY;
friend struct Worldspace;
};
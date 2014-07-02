#pragma once
#include "Entity.h"
#include "Local.h"
#include "RVector.h"

// Actors have visible representations, whereas regular entities do not.
class Actor : public Entity {
public:
	enum pathFind_e {
		PATHFIND_NONE,		// Dumb, mob-like enemies won't even attempt to pathfind, they'll just move in a direction
		PATHFIND_ASTAR,
	};
	float GetPreviousX() const { return pX; }
	float GetPreviousY() const { return pY; }
protected:
	float pX, pY;
	float fSpeed;

	bool bIsPlayer;

	pathFind_e pathfinding;
	bool Move();

	RVec2<float> dir;
private:
	bool Move_NoPathfinding();
	bool Move_AStarPathfinding();
friend struct Worldspace;
};
#pragma once
#include "Entity.h"
#include "Actor.h"

// Players are...players.
class Player : public Actor {
	void MoveToScreenspace(int sx, int sy, bool bStopAtDestination);
public:
	virtual void render();
	virtual void think();
	virtual void spawn();
	Player(float x, float y);

	void MouseUpEvent(int sX, int sY);
	void MouseDownEvent(int sX, int sY);
	void MouseMoveEvent(int sX, int sY);

	void SignalZoneChange(int nX, int nY, const char* newZone);
	void MoveToNextVis();

	static Entity* spawnme(float x, float y, int spawnflags, int act);
	unsigned char playerNum;
};
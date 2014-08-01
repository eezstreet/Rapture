#include "Player.h"
#include "RVector.h"
#include "Local.h"
#include "DungeonManager.h"
#include "Server.h"

static RVec2<float> dest(0,0);
static bool bShouldWeBeMoving = false;
static bool bDoWeHaveADestination = false;
static bool bMouseDown = false;
int iMovingToVis = -1;

extern int visTouching;

#define MAX_PLAYER_TOUCHME_DIS 0.035f
#define MIN_PLAYER_MOVE_DIS 0.25f

Player::Player(float _x, float _y) {
	this->x = _x;	// being weird
	this->y = _y;
	uuid = genuuid();
	fSpeed = 0.003f;
	bIsPlayer = true;
}

void Player::think() {
	RVec2<float> origin(x, y);
	pX = x;
	pY = y;

	if(bShouldWeBeMoving && bDoWeHaveADestination && origin.Within(dest, MAX_PLAYER_TOUCHME_DIS)) {
		bShouldWeBeMoving = false;
	}

	if(!bShouldWeBeMoving && origin.Within(dest, MIN_PLAYER_MOVE_DIS) && ptDestinationEnt) {
		ptDestinationEnt->interact(this);
		ptDestinationEnt = nullptr;
	}

	if(bShouldWeBeMoving) {
		bShouldWeBeMoving = Move();
	}
}

void Player::spawn() {
	Worldspace* ptWorld = ptServer->ptDungeonManager->GetWorld(this->iAct);
	materialHandle = trap->RegisterMaterial("TestCharacter");
	pX = x;
	pY = y;
	pathfinding = PATHFIND_NONE;	// FIXME
	ptWorld->AddPlayer(this);
}

void Player::render() {
	int screenWidth = 0;
	int screenHeight = 0;

	trap->CvarIntVal("r_width", &screenWidth);
	trap->CvarIntVal("r_height", &screenHeight);

	trap->RenderMaterial(materialHandle, (screenWidth / 2) - 32, (screenHeight / 2) - 64);
}

void Player::interact(Entity* interacter) {
}

bool Player::mouseover() {
	return false;
}

void Player::MoveToNextVis() {
	ptServer->ptDungeonManager->MovePlayerToVis(iAct, playerNum, iMovingToVis);
}

void Player::MoveToScreenspace(int sx, int sy, bool bStopAtDestination) {
	float clickedX = Worldspace::ScreenSpaceToWorldPlaceX(sx, sy, this);
	float clickedY = Worldspace::ScreenSpaceToWorldPlaceY(sx, sy, this);

	RVec2<float> origin(x, y);
	RVec2<float> destination(clickedX, clickedY);
	dir = destination - origin;
	dir.Normalize();
	dest = destination;

	if(origin.Within(dest, MIN_PLAYER_MOVE_DIS)) {
		bShouldWeBeMoving = false;
		if(ptServer->GetClient()->ptFocusEnt != nullptr) {
			ptDestinationEnt = ptServer->GetClient()->ptFocusEnt;
		}
		// FIXME: use whatever is in our crosshair
		return;
	}

	bShouldWeBeMoving = true;
	bDoWeHaveADestination = bStopAtDestination;
}

void Player::MouseMoveEvent(int sx, int sy) {
	// If we're moving, we need to update our position
	if(bMouseDown && bShouldWeBeMoving) {
		MoveToScreenspace(sx, sy, false);
	}
}

void Player::MouseDownEvent(int sx, int sy) {
	ptDestinationEnt = nullptr;
	iMovingToVis = visTouching;
	MoveToScreenspace(sx, sy, false);
	bMouseDown = true;
}

void Player::MouseUpEvent(int sx, int sy) {
	bMouseDown = false;
	bDoWeHaveADestination = true;
}

Entity* Player::spawnme(float x, float y, int spawnflags, int act) {
	Player* ent = new Player(x, y);
	ent->x = x;
	ent->y = y;
	ent->spawnflags = spawnflags;
	ent->uuid = genuuid();
	ent->classname = "Player";

	ent->bShouldWeCollide = true;
	ent->bShouldWeRender = true;
	ent->bShouldWeThink = true;
	ent->iAct = act;

	return ent;
}

void Player::SignalZoneChange(int nX, int nY, const char* newZone) {
	Worldspace* ptWorld = ptServer->ptDungeonManager->GetWorld(this->iAct);
	trap->FadeFromBlack(500);
	x = nX;
	y = nY;
	bShouldWeBeMoving = bDoWeHaveADestination = false;
	ptServer->GetClient()->EnteredArea(newZone);
	ptWorld->ActorMoved(this);
}
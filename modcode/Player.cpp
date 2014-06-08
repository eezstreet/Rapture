#include "g_local.h"
#include "RVector.h"

const static float speed = 0.02;
static RVec2<float> dir(0,0);
static RVec2<float> dest(0,0);
static bool bShouldWeBeMoving = false;
static bool bDoWeHaveADestination = false;
static bool bMouseDown = false;

Player::Player(float _x, float _y) {
	this->x = _x;	// being weird
	this->y = _y;
	uuid = genuuid();
}

void Player::think() {
	RVec2<float> origin(x, y);
	if(bShouldWeBeMoving && origin.Within(dest, 0.025)) {
		bShouldWeBeMoving = false;
	}
	else if(bShouldWeBeMoving) {
		x += dir.GetX() * speed;
		y += dir.GetY() * speed;
	}
	render();
}

void Player::spawn() {
	materialHandle = trap->RegisterMaterial("TestCharacter");
	world.AddPlayer(this);
}

void Player::render() {
	int screenWidth = 0;
	int screenHeight = 0;

	trap->CvarIntVal("r_width", &screenWidth);
	trap->CvarIntVal("r_height", &screenHeight);

	trap->RenderMaterial(materialHandle, (screenWidth / 2) - 32, (screenHeight / 2) - 64);
}

void Player::MoveToScreenspace(int sx, int sy, bool bStopAtDestination) {
	float clickedX = Worldspace::ScreenSpaceToWorldPlaceX(sx, sy);
	float clickedY = Worldspace::ScreenSpaceToWorldPlaceY(sx, sy);

	RVec2<float> origin(x, y);
	RVec2<float> destination(clickedX, clickedY);
	dir = destination - origin;
	dir.Normalize();
	dest = destination;

	bShouldWeBeMoving = true;
	bDoWeHaveADestination = bStopAtDestination;
}

void Player::MouseMoveEvent(int sx, int sy) {
	// Change direction
	if(bMouseDown && bShouldWeBeMoving) {
		MoveToScreenspace(sx, sy, true);
	}
}

void Player::MouseDownEvent(int sx, int sy) {
	MoveToScreenspace(sx, sy, true);
	bMouseDown = true;
}

void Player::MouseUpEvent(int sx, int sy) {
	bMouseDown = false;
}
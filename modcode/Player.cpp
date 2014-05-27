#include "g_local.h"

Player::Player(float _x, float _y) {
	this->x = _x;	// being weird
	this->y = _y;
	uuid = genuuid();
}

void Player::think() {
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
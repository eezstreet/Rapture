#include "g_local.h"
#include "RVector.h"

const static float speed = 0.003;
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
	pX = x;
	pY = y;

	if(bShouldWeBeMoving && bDoWeHaveADestination && origin.Within(dest, 0.035)) {
		bShouldWeBeMoving = false;
	}
	else if(bShouldWeBeMoving) {
		// TODO: run collision detection
		RVec2<float> nextFramePosition((dir.GetX() * speed * GetGameFrametime())+x, (dir.GetY() * speed * GetGameFrametime())+y);
		if(nextFramePosition.GetX() < 0 && nextFramePosition.GetY() < 0) {
			// No moving into negative coords
			bShouldWeBeMoving = false;
			return;
		}

		int bottomedOutX = (int)floor(nextFramePosition.GetX());
		int bottomedOutY = (int)floor(nextFramePosition.GetY());
		int bottomedOutSubtileX = (int)floor((nextFramePosition.GetX()-bottomedOutX)*4);
		int bottomedOutSubtileY = (int)floor((nextFramePosition.GetY()-bottomedOutY)*4);
		auto nodes = world.qtTileTree->NodesAt(bottomedOutX+1, bottomedOutY+1);
		if(nodes.size() <= 0) {
			// No nodes in this sector = no movement
			bShouldWeBeMoving = false;
			return;
		}
		
		bool bDoWeHaveNodeHere = false;
		vector<TileNode*> nodesOnThis;
		for(auto it = nodes.begin(); it != nodes.end(); ++it) {
			auto node = (*it);
			if(node->x == bottomedOutX && node->y == bottomedOutY) {
				nodesOnThis.push_back(node);
			}
		}
		if(nodesOnThis.size() <= 0) {
			// No tiles here? NO CAN PASS.
			bShouldWeBeMoving = false;
			return;
		}
		for(auto it = nodesOnThis.begin(); it != nodesOnThis.end(); ++it) {
			auto node = (*it);
			if(node->ptTile->lowmask == 0) {
				// No mask = no worries
				continue;
			}
			if(node->ptTile->lowmask & (1 << (bottomedOutSubtileX * 4)+bottomedOutSubtileY)) {
				// Does this subtile specifically block movement? Don't allow it.
				bShouldWeBeMoving = false;
				return;
			}
		}
		x = nextFramePosition.GetX();
		y = nextFramePosition.GetY();
		world.ActorMoved(this);
	}
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
		MoveToScreenspace(sx, sy, false);
	}
}

void Player::MouseDownEvent(int sx, int sy) {
	MoveToScreenspace(sx, sy, false);
	bMouseDown = true;
}

void Player::MouseUpEvent(int sx, int sy) {
	bMouseDown = false;
	bDoWeHaveADestination = true;
}
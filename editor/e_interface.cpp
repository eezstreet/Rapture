#include "e_local.h"

float fCameraOffsetX = 0.0f;
float fCameraOffsetY = 0.0f;

float WorldPlaceToScreenSpaceFX(float x, float y) {
	return (96.0 * x) + (96.0 * y);
}

float WorldPlaceToScreenSpaceFY(float x, float y) {
	return (48.0 * y) - (48.0 * x);
}

int WorldPlaceToScreenSpaceIX(int x, int y) {
	return (96 * x) + (96 * y);
}

int WorldPlaceToScreenSpaceIY(int x, int y) {
	return (48 * y) - (48 * x);
}

float CameraOffsetX() {
	int screenWidth = 0;

	trap->CvarIntVal("r_width", &screenWidth);

	return (screenWidth / 2) - (96.0 * fCameraOffsetX) - (96.0 * fCameraOffsetY);
}

float CameraOffsetY() {
	int screenHeight = 0;

	trap->CvarIntVal("r_height", &screenHeight);

	return (screenHeight / 2) - (48.0 * fCameraOffsetY) + (48.0 * fCameraOffsetX);
}
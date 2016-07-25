#include "e_local.h"

float fCameraOffsetX = 0.0f;
float fCameraOffsetY = 0.0f;

float WorldPlaceToScreenSpaceFX(float x, float y) {
	return (96.0f * x) + (96.0f * y);
}

float WorldPlaceToScreenSpaceFY(float x, float y) {
	return (48.0f * y) - (48.0f * x);
}

int WorldPlaceToScreenSpaceIX(int x, int y) {
	return (96 * x) + (96 * y);
}

int WorldPlaceToScreenSpaceIY(int x, int y) {
	return (48 * y) - (48 * x);
}

float CameraOffsetX() {
	int screenWidth = 0;

	//trap->CvarIntVal("vid_width", &screenWidth);

	return (screenWidth / 2) - (96.0f * fCameraOffsetX) - (96.0f * fCameraOffsetY);
}

float CameraOffsetY() {
	int screenHeight = 0;

	//trap->CvarIntVal("vid_height", &screenHeight);

	return (screenHeight / 2) - (48.0f * fCameraOffsetY) + (48.0f * fCameraOffsetX);
}
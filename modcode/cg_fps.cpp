#include "g_local.h"

#define FRAME_CAPTURE	20				// Capture 20 frames worth of activity for FPS average
static unsigned int frametimes[FRAME_CAPTURE];
static unsigned int framecount = 0;
static unsigned int framelast = 0;
static float fps;
static unsigned int frametime = 0;

void InitFPS() {
	memset(frametimes, 0, sizeof(frametimes));
	framecount = 0;
	framelast = trap->GetTicks();
	fps = 0;
}

void FPSFrame() {
	unsigned int index = framecount % FRAME_CAPTURE;
	unsigned int ticks = trap->GetTicks();
	unsigned int count = 0;

	frametimes[index] = ticks - framelast;
	framecount++;
	
	if(framecount < FRAME_CAPTURE) {
		count = framecount;
	} else {
		count = FRAME_CAPTURE;
	}

	framelast = ticks;

	fps = 0;
	for(int i = 0; i < count; i++) {
		fps += frametimes[i];
	}

	fps /= count;
	fps = 1000 / fps;

	frametime = frametimes[index];
}

float GetGameFPS() {
	return fps;
}

unsigned int GetGameFrametime() {
	return frametime;
}
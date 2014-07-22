#include "sys_local.h"

// Based off of LazyFoo' LTimer implementation

Timer::Timer() {
	sTimerName = "<unnamed timer>";
	Stop();
}

Timer::Timer(const string& sName) {
	Timer();
	sTimerName = sName;
}

void Timer::Start() {
	bIsStarted = true;
	bIsPaused = false;

	ulStartTicks = SDL_GetTicks();
	ulPausedTicks = 0;
}

void Timer::Stop() {
	bIsStarted = bIsPaused = false;
	ulStartTicks = ulPausedTicks = 0;
}

void Timer::Pause() {
	if(bIsPaused) {
		R_Message(PRIORITY_WARNING, "WARNING: Timer '%s' attempted to re-pause! (@ ticks: %i)\n", sTimerName.c_str(), SDL_GetTicks());
		return;
	} else if(!bIsStarted) {
		R_Message(PRIORITY_WARNING, "WARNING: Attempted to pause stopped timer '%s'!\n", sTimerName.c_str());
		return;
	}

	bIsPaused = true;
	ulPausedTicks = SDL_GetTicks() - ulStartTicks;
	ulStartTicks = 0;
}

void Timer::Unpause() {
	if(!bIsPaused) {
		R_Message(PRIORITY_WARNING, "WARNING: Attempted to unpause running timer '%s'\n", sTimerName.c_str());
		return;
	} else if(!bIsStarted) {
		R_Message(PRIORITY_WARNING, "WARNING: Attempted to unpause stopped timer '%s'\n", sTimerName.c_str());
		return;
	}

	bIsPaused = false;
	ulStartTicks = SDL_GetTicks() - ulPausedTicks;
	ulPausedTicks = 0;
}

unsigned long Timer::GetTicks() {
	if(bIsStarted) {
		if(bIsPaused) {
			return ulPausedTicks;
		} else {
			return SDL_GetTicks() - ulStartTicks;
		}
	}
	return 0;
}
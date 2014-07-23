#include "sys_local.h"

FrameCapper::FrameCapper() {
	capCvar = Cvar::Get<int>("com_maxfps", "Maximum allowed FPS", (1 << CVAR_ARCHIVE), 180);
	hitchWarningCvar = Cvar::Get<int>("com_hitch", "Hitch warning", (1 << CVAR_ARCHIVE), 100);
}

void FrameCapper::StartFrame() {
	capTimer.Start();
}

void FrameCapper::EndFrame() {
	unsigned long ulFrameTicks = capTimer.GetTicks();
	unsigned long ulCap = (1000 / capCvar->Integer());
	unsigned long ulHitchWarning = hitchWarningCvar->Integer();
	if(ulFrameTicks < ulCap) {
		SDL_Delay( ulCap - ulFrameTicks );
	}
	if(ulFrameTicks >= ulHitchWarning && ulHitchWarning > 0) {
		R_Message(PRIORITY_WARNING, "hitch warning: %i ms frame time\n", ulFrameTicks);
	}
}
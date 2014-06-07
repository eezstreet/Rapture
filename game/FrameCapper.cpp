#include "sys_local.h"

FrameCapper::FrameCapper() {
	capCvar = Cvar::Get<int>("com_maxfps", "Maximum allowed FPS", (1 << Cvar::CVAR_ARCHIVE), 180);
}

void FrameCapper::StartFrame() {
	capTimer.Start();
}

void FrameCapper::EndFrame() {
	unsigned long ulFrameTicks = capTimer.GetTicks();
	unsigned long ulCap = (1000 / capCvar->Integer());
	if(ulFrameTicks < ulCap) {
		SDL_Delay( ulCap - ulFrameTicks );
	}
}
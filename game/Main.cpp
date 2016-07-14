#include "sys_local.h"

int main(int argc, char** argv) {
	RaptureGame* game = RaptureGame::GetSingleton();
	if (game->HasFlags(RaptureGame::Rapture_Initialized)) {
		FrameCapper fc;
		while (!game->HasFlags(RaptureGame::Rapture_FatalError) && !game->HasFlags(RaptureGame::Rapture_Quitting)) {
			fc.StartFrame();
			game->RunLoop();
			fc.EndFrame();
		}
	}
	RaptureGame::DestroySingleton();
	return 0;
}
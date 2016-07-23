#include "Simulation.h"
#include "Savegame.h"

#define TEMP_SAVE_PATH	"temp.rsav"

namespace Simulation {
	static bool bIsInitialized = false;
	static char szSave[MAX_SAVE_PATH] {0};
	static Savegame savegame;

	void Initialize(const char *szSavePath) {
		if (bIsInitialized) {
			return;
		}

		if (szSavePath == nullptr) {
			strcpy(szSave, TEMP_SAVE_PATH);
		}
		else {
			strcpy(szSave, szSavePath);
		}
		savegame = Savegame::Retrieve(szSavePath);
		bIsInitialized = true;
	}

	void Shutdown() {
		if (!bIsInitialized) {
			return;
		}

		Savegame::Save(savegame, szSave);
		bIsInitialized = false;
	}
}
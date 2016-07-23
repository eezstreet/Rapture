#include "Savegame.h"
#include "Modcode.h"

#define SAVE_READ_MODE	"rb+"
#define SAVE_WRITE_MODE	"wb+"

// TODO: serialization

Savegame Savegame::Retrieve(const char* path) {
	Savegame out;

	if(path != nullptr) {
		char pathBuf[MAX_SAVE_PATH]{0};
		sprintf(pathBuf, "%s/%s", SAVE_DIRECTORY, path);
		File* inFile = trap->OpenFileSync(pathBuf, SAVE_READ_MODE);
		if (inFile != nullptr) {
			// Read each field individually
			trap->ReadFileSync(inFile, &out.head, sizeof(out.head));
			trap->CloseFileAsync(inFile, nullptr);

			// We should also modify the last use time
			out.head.lastUseTime = trap->GetCurrentTimeDate();
			return out;
		}
	}

	Rapture_TimeDate curTime = trap->GetCurrentTimeDate();
	Rapture_TimeDate playTime{ 0 };
	Savegame_Header head{ SAVEGAME_HEAD, SAVEGAME_VER, curTime, curTime, playTime };
	out.head = head;

	return out;
}

#define TEMP_SAVE_PATH "temp.rsav"
void Savegame::Save(Savegame& out, const char* path) {
	char pathBuf[MAX_SAVE_PATH]{0};

	sprintf(pathBuf, "%s/%s", SAVE_DIRECTORY, path);
	File* outFile = trap->OpenFileSync(pathBuf, SAVE_WRITE_MODE);
	if (outFile == nullptr) {
		trap->printf(PRIORITY_WARNING, "Couldn't open %s for saving\n", pathBuf);
		return;
	}

	// Add on to the play time
	Rapture_TimeDate currentTime = trap->GetCurrentTimeDate();
	Rapture_TimeDate diffTime{ 0 };
	trap->SubtractTimeDate(currentTime, out.head.lastUseTime, &diffTime);
	trap->AddTimeDate(diffTime, out.head.playTime, &out.head.playTime);

	// Save the file
	trap->WriteFileSync(outFile, &out.head, sizeof(out.head));
	trap->CloseFileAsync(outFile, nullptr);
}
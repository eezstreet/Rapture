#include "Savegame.h"
#include "Modcode.h"

#define SAVE_READ_MODE	"rb+"
#define SAVE_WRITE_MODE	"wb+"

// TODO: serialization

Savegame Savegame::Retrieve(const char* path) {
	Savegame out;
	memset(&out, 0, sizeof(Savegame));

	if (path != nullptr) {
		File* infile = trap->OpenFileSync(path, SAVE_READ_MODE);
		if (infile == nullptr) {
			trap->printf(PRIORITY_WARNING, "Couldn't open %s for reading\n", path);
		}
		
		// <---------- SAVEGAME READING ROUTINE ------------>
		trap->ReadFileSync(infile, &out.head, sizeof(out.head));

		trap->CloseFileSync(infile);
	}

	return out;
}

void Savegame::Save(Savegame& out, const char* path) {
	// Add on to play time
	Rapture_TimeDate currentTime = trap->GetCurrentTimeDate();
	Rapture_TimeDate diffTime{ 0 };
	trap->SubtractTimeDate(currentTime, out.head.time.lastUseTime, &diffTime);
	trap->AddTimeDate(diffTime, out.head.time.playTime, &out.head.time.playTime);

	// Actually save the file
	if (path != nullptr && stricmp(path, "null")) {
		File* outfile = trap->OpenFileSync(path, SAVE_WRITE_MODE);
		if (outfile == nullptr) {
			trap->printf(PRIORITY_WARNING, "Couldn't open %s for saving\n", path);
			return;
		}

		// <---------- SAVEGAME WRITING ROUTINE ------------>
		trap->WriteFileSync(outfile, &out.head, sizeof(out.head));

		trap->CloseFileSync(outfile);
	}
}
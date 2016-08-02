#include "sys_local.h"
#include "../json/cJSON.h"

#define MAX_SAVEGAME_DEPTH	8
#define SAVEGAME_INITIAL	128
#define SAVEGAME_BLOCK		128
#define SAVEGAME_EXTENSION	".rsav"
#define SAVEGAME_FOLDER		"save"

namespace SaveGame {
	const char* RequestSavegameInfo(bool bMultiplayer) {
		cJSONStream* json = cJSON_Stream_New(MAX_SAVEGAME_DEPTH, 0, SAVEGAME_INITIAL, SAVEGAME_BLOCK);
		vector<string> vSaveFiles;

		Filesystem::ListAllFilesInPath(vSaveFiles, SAVEGAME_EXTENSION, SAVEGAME_FOLDER);
		cJSON_Stream_BeginObject(json, "saves");
		for (auto& fileName : vSaveFiles) {
			const char* path = fileName.c_str();
			File* pFile = File::OpenSync(path);
			if (!pFile) {
				continue;
			}
			Rapture_Savegame::Rapture_SaveHeader head;
			File::ReadSync(pFile, &head, sizeof(Rapture_Savegame::Rapture_SaveHeader));
			File::CloseSync(pFile);

			if (!(bMultiplayer ^ (bool)head.meta.multiplayer)) {
				Rapture_CharacterMeta meta = head.meta;
				cJSON_Stream_BeginObject(json, path);
				cJSON_Stream_WriteString(json, "name", meta.charName);
				cJSON_Stream_WriteInteger(json, "class", meta.charClass);
				cJSON_Stream_WriteInteger(json, "league", meta.charLeague);
				cJSON_Stream_WriteInteger(json, "difficulty", meta.difficulty);
				cJSON_Stream_EndBlock(json);
			}
		}

		// test stuff, don't actually use
#if 0
		cJSON_Stream_BeginObject(json, "C:/Rapture/save/test.rsav");
		cJSON_Stream_WriteString(json, "name", "Test Character");
		cJSON_Stream_WriteInteger(json, "class", 0);
		cJSON_Stream_WriteInteger(json, "league", 0);
		cJSON_Stream_WriteInteger(json, "difficulty", 0);
		cJSON_Stream_EndBlock(json);
#endif
		cJSON_Stream_EndBlock(json);
		
		return cJSON_Stream_Finalize(json);
	}

	void DeleteSavegame(const char* path) {
		string szPath = path;
		string ext = SAVEGAME_EXTENSION;

		// Don't remove files unless they have the .rsav extension (in order to prevent a potential exploit)
		if (szPath.compare(szPath.length() - ext.length(), ext.length(), ext)) {
			R_Message(PRIORITY_WARNING, "Tried to delete a savegame with invalid extension: %s\n", path);
			return;
		}
		remove(path);
	}

	void CreateSavegame(const char* charCreateJSON) {
		char jsonError[SAVEGAME_BLOCK] {0};
		cJSON* json = cJSON_ParsePooled(charCreateJSON, jsonError, sizeof(jsonError));
		if (json == nullptr) {
			R_Message(PRIORITY_WARNING, "Failed to process input from createSaveGame command. (%s)\n", jsonError);
			return;
		}

		// Character class
		cJSON* child = cJSON_GetObjectItem(json, "class");
		const int nCharClass = cJSON_ToInteger(child);

		// Bandana color
		child = cJSON_GetObjectItem(json, "bandana");
		const int nCharColor = cJSON_ToInteger(child);

		// Character name
		child = cJSON_GetObjectItem(json, "name");
		const char* szCharName = cJSON_ToString(child);

		// Starting skill
		child = cJSON_GetObjectItem(json, "skill");
		const int nCharSkill = cJSON_ToInteger(child);

		// League
		child = cJSON_GetObjectItem(json, "league");
		const int nCharLeague = cJSON_ToInteger(child);

		// Multiplayer
		child = cJSON_GetObjectItem(json, "multiplayer");
		const uint8_t multiplayer = cJSON_ToBoolean(child);

		Rapture_Savegame save = {
			{
				// Header
				{ 'R', 'S', 'A', 'V' },
				// Meta
				{
					{ 0 },
					nCharClass,
					nCharColor,
					nCharLeague,
					0,
					nCharSkill,
					multiplayer
				},
				// Time
				{
					{ 0 },
					TimeDate::GetCurrent(),
					{ 0 }
				}
			}
		};
		strcpy(save.head.meta.charName, szCharName);

		// Find the first available save slot and write to it
		char saveBuffer[SAVEGAME_BLOCK] {0};
		int number = -1;
		File* pFile = nullptr;
		sprintf(saveBuffer, "save/%s.rsav", szCharName);
		do {
			number++;
			pFile = File::OpenSync(saveBuffer, "wb+");
			sprintf(saveBuffer, "save/%s-%i.rsav", szCharName, number);
		} while (pFile == nullptr);
		File::WriteSync(pFile, &save, sizeof(save));
		File::CloseSync(pFile);

		cJSON_Delete(json);
	}
}
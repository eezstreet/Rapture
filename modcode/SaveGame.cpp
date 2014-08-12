#include "SaveGame.h"
#include <json/cJSON.h>

SaveGame* SaveGame::NewSingleplayerSave(const Server* ptServer, const Client* ptClient) {
	SaveGame* s = new SaveGame(ptServer, ptClient, ptClient->ptPlayer);

	return s;
}

#define MAX_SAVE_DEPTH 10
#define MAX_SAVE_SIZE 10240 // 10kb ought to do
SaveGame::SaveGame(const Server* ptServer, const Client* ptClient, const Player* ptPlayer) {
	if(ptServer == nullptr) {
		// Multiplayer game saved onto local disk
		return; // TODO
	}
	// Save all files as "test.sav" for now.
	File* ptFile = trap->OpenFile("save/test.sav", "wb");
	if(ptFile == nullptr) {
		R_Error("Couldn't open save file \"save/test.sav\"\n");
		return;
	}

	// FIXME: Need to write more code involving towns...
	string sTown = "Survivor's Camp";
	cJSONStream* ptStream = cJSON_Stream_New(MAX_SAVE_DEPTH, true, MAX_SAVE_SIZE, MAX_SAVE_SIZE);

	{
		// --- Save Start ---
		cJSON_Stream_BeginObject(ptStream, "SAVE");

		{
			// --- Header Data ---
			cJSON_Stream_BeginObject(ptStream, "HEADER");

			cJSON_Stream_WriteString(ptStream, "savename", "Test Save");
			cJSON_Stream_WriteString(ptStream, "spawntown", sTown.c_str());

			// --- Header Data ---
			cJSON_Stream_EndBlock(ptStream);
		}

		{
			// --- Quest Data ---
			cJSON_Stream_BeginObject(ptStream, "QUEST");

			for(auto it = ptServer->ptQuestManager->GetStateStartIterator();
				it != ptServer->ptQuestManager->GetStateEndIterator();
				++it) {
				if(it->second == CQS_QUEST_COMPLETE) { // Mark the quest as being complete in a previous game
					cJSON_Stream_WriteInteger(ptStream, it->first.c_str(), CQS_QUEST_COMPLETEPREVIOUS);
				} else {
					cJSON_Stream_WriteInteger(ptStream, it->first.c_str(), it->second);
				}
			}

			// --- Quest Data ---
			cJSON_Stream_EndBlock(ptStream);
		}

		// --- Save Start ---
		cJSON_Stream_EndBlock(ptStream);
	}

	const char* sFileContents = cJSON_Stream_Finalize(ptStream);
	trap->WriteFile(ptFile, sFileContents);
	trap->CloseFile(ptFile);
}
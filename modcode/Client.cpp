#include "Client.h"
#include "Server.h"
#include "Worldspace.h"
#include "DungeonManager.h"
#include "NPC.h"

Client::Client() {
	ptConsolasFont = trap->RegisterFont("fonts/consola.ttf", 18);
	framecount = 0;
	framelast = 0;
	frametime = 0;

	memset(frametimes, 0, sizeof(frametimes));
	framecount = 0;
	framelast = trap->GetTicks();
	fps = 0;

	ptHUD = trap->RegisterStaticMenu("ui/hud.html");
	bShouldDrawLabels = false;
	bStoppedDrawingLabels = false;
	bEntDrawingLabels = false;

	trap->AddJSCallback(ptHUD, "signalNPCMenuClosed", Client::NPCMenuClosed);
	trap->AddJSCallback(ptHUD, "npcMenuCallback", Client::NPCPickOption);
	trap->AddJSCallback(ptHUD, "defineClickZone", Client::DefineClickZone);
	trap->AddJSCallback(ptHUD, "undefineClickZone", Client::UndefineClickZone);
	trap->AddJSCallback(ptHUD, "changedSelectedQuest", Client::ChangedQuestLogSelectedQuest);

	trap->CvarIntVal("vid_width", &screenWidth);
	trap->CvarIntVal("vid_height", &screenHeight);
}

Client::~Client() {
	trap->KillStaticMenu(ptHUD);
	m_clickZones.clear();
}

void Client::RunFPS() {
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

	fps /= (float)count;
	fps = 1000 / fps;

	frametime = frametimes[index];
}

void Client::DrawViewportInfo() {
	int drawFPS = 0;
	bool drawXY = false;
	bool drawWorldXY = false;
	stringstream ss;

	trap->CvarIntVal("cg_drawfps", &drawFPS);
	trap->CvarBoolVal("cg_drawxy", &drawXY);
	trap->CvarBoolVal("cg_drawworldxy", &drawWorldXY);

	if(drawFPS >= 1) {
		if(drawFPS == 1 || drawFPS == 3) {
			ss << "FPS: ";
			ss << fps;
			ss << "      ";
		}
		if(drawFPS == 2 || drawFPS == 3) {
			ss << "ms: ";
			ss << frametime;
			ss << "      ";
		}
	}

	if(drawXY) {
		ss << "Screen Space: ";
		ss << cursorX;
		ss << "X / ";
		ss << cursorY;
		ss << "Y      ";
	}

	if(drawWorldXY) {
		ss << "World Space: ";
		ss << (Worldspace::ScreenSpaceToWorldPlaceX(cursorX, cursorY, ptPlayer));
		ss << "X / ";
		ss << (Worldspace::ScreenSpaceToWorldPlaceY(cursorX, cursorY, ptPlayer));
		ss << "Y      ";
	}
	if(ss.str().length() > 0) {
		trap->RenderTextShaded(ptConsolasFont, ss.str().c_str(), 255, 255, 255, 127, 127, 127);
	}
}

void Client::Preframe() {
	bEntDrawingLabels = false;
	ptFocusEnt = nullptr;
}

void Client::Frame() {
	vector<Entity*> vEnts = ptServer->ptDungeonManager->GetEntsAt(Worldspace::ScreenSpaceToWorldPlaceX(cursorX, cursorY, ptPlayer),
		Worldspace::ScreenSpaceToWorldPlaceY(cursorX, cursorY, ptPlayer), ptPlayer->iAct);
	if(vEnts.empty()) {
		bShouldDrawLabels = false;
	} else {
		for(auto it = vEnts.begin(); it != vEnts.end(); ++it) {
			Entity* ent = (*it);
			if(ent->mouseover()) {
				bShouldDrawLabels = true;
				break;
			}
		}
	}

	RunFPS();

	ptServer->ptDungeonManager->GetWorld(ptPlayer->iAct)->Render(this); // FIXME
	if(!bShouldDrawLabels && !bStoppedDrawingLabels) {
		HideLabels();
	}

	DrawViewportInfo();
}

void Client::HideLabels() {
	trap->RunJavaScript(ptHUD, "stopLabelDraw();");
	bStoppedDrawingLabels = true;
}

void Client::StartLabelDraw(const char* sLabel) {
	stringstream fullName;
	fullName << "startLabelDraw(\"" << sLabel << "\", " << cursorX+15 << ", " << cursorY-30 << ");";
	trap->RunJavaScript(ptHUD, fullName.str().c_str());
	bStoppedDrawingLabels = false;
}

void Client::EnteredArea(const char* sArea) {
	stringstream fullName;
	fullName << "changeDivHTML('ID_zone', \"" << sArea << "\");";
	trap->RunJavaScript(ptHUD, fullName.str().c_str());
}

static bool bNPCMenuUp = false;
static bool bPreventMouseInteraction = false;
void Client::PassMouseDown(int x, int y) {
	cursorX = x;
	cursorY = y;

	if(bNPCMenuUp == true) {
		return;
	}

	// Check and see if we have any HUD-defined areas which we are not allowed to click
	for(auto it = m_clickZones.begin(); it != m_clickZones.end(); ++it) {
		ClickZone_t* ptCZ = &it->second;
		if(x >= ptCZ->x && x <= ptCZ->x + ptCZ->w) {
			if(y >= ptCZ->y && y <= ptCZ->y + ptCZ->h) {
				return;
			}
		}
	}

	if(!trap->IsConsoleOpen()) {
		ptPlayer->MouseDownEvent(x, y);
		if(ptFocusEnt) {
			ptPlayer->SetDestinationEnt(ptFocusEnt);
		}
	}
}

void Client::PassMouseUp(int x, int y) {
	cursorX = x;
	cursorY = y;
	ptPlayer->MouseUpEvent(x, y);
}

void Client::PassMouseMove(int x, int y) {
	cursorX = x;
	cursorY = y;
	if(!trap->IsConsoleOpen()) {
		ptPlayer->MouseMoveEvent(x, y);
	}
}

void Client::NPCStartInteraction(Entity* ptNPC, OptionList& rtOptionList) {
	stringstream jsString;

	if(rtOptionList.size() <= 0) {
		return;
	}

	NPC* ptNpc = (NPC*)ptNPC;
	ptInteractingEnt = ptNPC;

	jsString << "startedNPCInteraction(" << rtOptionList.size() << ", '" << ptNpc->getname() << "'";
	jsString << ", " << cursorX << ", " << cursorY;
	for(auto it = rtOptionList.begin(); it != rtOptionList.end(); ++it) {
		jsString << ", '" << it->first << "'";
	}
	jsString << ");";
	ptCurrentOptionList = &rtOptionList;
	trap->RunJavaScript(ptHUD, jsString.str().c_str());

	bNPCMenuUp = true;
}

void Client::NPCChangeMenu(OptionList& rtOptionList) {
	stringstream jsString;

	if(rtOptionList.size() <= 0) {
		return;
	}

	jsString << "changedNPCMenu(" << rtOptionList.size();
	for(auto it = rtOptionList.begin(); it != rtOptionList.end(); ++it) {
		jsString << ", '" << it->first << "'";
	}
	jsString << ");";
	ptCurrentOptionList = &rtOptionList;
	trap->RunJavaScript(ptHUD, jsString.str().c_str());
}

void Client::NPCPickMenu(int iWhichOption, bool bClosedMenu) {
	if(!bNPCMenuUp) {
		return;
	}

	if(ptInteractingEnt == nullptr) {
#ifdef WIN32
		R_Message(PRIORITY_ERROR, "%s %i: !ptInteractingEnt\n", __FILE__, __LINE__);
#else
		R_Message(PRIORITY_ERROR, "Lost valid ptInteractingEnt ptr!\n");
#endif
		return;
	}

	if(ptCurrentOptionList == nullptr) {
#ifdef WIN32
		R_Message(PRIORITY_ERROR, "%s %i: !ptCurrentOptionList", __FILE__, __LINE__);
#else
		R_Message(PRIORITY_ERROR, "Lost valid ptCurrentOptionList ptr!\n");
#endif
		return;
	}

	NPC* ptNPC = (NPC*)ptInteractingEnt;
	if(bClosedMenu) {
		// Close the menu
		NPC::StopInteraction(ptNPC, ptPlayer);
		ptInteractingEnt = nullptr;
		trap->RunJavaScript(ptHUD, "closeNPCMenu();");
		bNPCMenuUp = false;
		return;
	}
	// NETWORKING FIXME
	assert(iWhichOption < ptCurrentOptionList->size());
	Option o = ptCurrentOptionList->at(iWhichOption);
	o.second(ptNPC, ptPlayer);
}

void Client::NPCMenuClosed() {
	ptClient->NPCPickMenu(-1, true);
}

void Client::NPCPickOption() {
	int iWhichOption = trap->GetJSIntArg(ptClient->ptHUD, 0);

	ptClient->NPCPickMenu(iWhichOption, false);
}

// Quest log
void Client::NewQuest(const string& sQuestName, const string& sQuestID, Client::SetDescriptionCallback pfCallback) {
	stringstream ss;
	ss << "newQuest('" << sQuestName << "', '" << sQuestID << "');";
	m_pfQuestDescriptions[sQuestID] = pfCallback;

	trap->RunJavaScript(ptHUD, ss.str().c_str());
}

void Client::SetQuestLogDescription(const string& sQuestDesc) {
	stringstream ss;
	ss << "setQuestLogDescriptionText('" << sQuestDesc << "');" << '\0';
	trap->RunJavaScript(ptHUD, ss.str().c_str());
}

void Client::ChangedQuestLogSelectedQuest() {
	char questID[MAX_HANDLE_STRING];
	trap->GetJSStringArg(ptClient->ptHUD, 0, questID, sizeof(questID));
	auto it = ptClient->m_pfQuestDescriptions.find(questID);
	if(it == ptClient->m_pfQuestDescriptions.end()) {
		trap->printf(PRIORITY_WARNING, "WARNING: attempted to change to invalid quest ID %s in quest log...\n", questID);
		return;
	}
	it->second(ptClient);
}

void Client::DefineClickZone() {
	char sZoneName[MAX_HANDLE_STRING];
	trap->GetJSStringArg(ptClient->ptHUD, 0, sZoneName, sizeof(sZoneName));
	int x = trap->GetJSIntArg(ptClient->ptHUD, 1);
	int y = trap->GetJSIntArg(ptClient->ptHUD, 2);
	int w = trap->GetJSIntArg(ptClient->ptHUD, 3);
	int h = trap->GetJSIntArg(ptClient->ptHUD, 4);
	ClickZone_t cz; cz.x = x; cz.y = y; cz.w = w; cz.h = h;
	ptClient->m_clickZones[sZoneName] = cz;
}

void Client::UndefineClickZone() {
	char sZoneName[MAX_HANDLE_STRING];
	trap->GetJSStringArg(ptClient->ptHUD, 0, sZoneName, sizeof(sZoneName));
	ptClient->m_clickZones.erase(sZoneName);
}
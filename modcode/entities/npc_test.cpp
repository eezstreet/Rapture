#include "npc_test.h"
#include "Server.h"
#include "OptionList.h"

static OptionList test;
static OptionList testSubmenu;
void TestInteraction(Entity* a1, Entity* a2) {
	R_Message(PRIORITY_DEBUG, "Test Interaction Successful\n");
	NPC::StopClientFromInteracting(a1, a2);
}

void TestSubmenu(Entity* a1, Entity* a2) {
	NPC::OpenSubmenu(testSubmenu);
}

void TestSubmenuBack(Entity* a1, Entity* a2) {
	NPC::OpenSubmenu(test);
}

void TestQuestInit(Entity* a1, Entity* a2) {
	ptServer->ptQuestManager->ChangeQuestState("Test Quest", 1);
	RemoveOption(test, "Test Quest Init");
	NPC::OpenSubmenu(test);
}

void npc_test::render() {
	int renderPosX = Entity::GetDrawX(this);
	int renderPosY = Entity::GetDrawY(this);

	//trap->RenderMaterial(materialHandle, renderPosX + 35, renderPosY - 25);
}

void npc_test::think() {
	return; // Currently do nothing? Future NPCs might patrol areas or do random things
}

void npc_test::spawn() {
	ptInteractingWith = nullptr;
	test.clear();
	InsertOption(test, "Test Interaction", TestInteraction);
	InsertOption(test, "Test Quest Init", TestQuestInit);
	InsertOption(test, "Test Submenu", TestSubmenu);
	InsertOption(test, "Cancel", NPC::StopClientFromInteracting);
	InsertOption(testSubmenu, "Test Interaction 2", TestInteraction);
	InsertOption(testSubmenu, "Return", TestSubmenuBack);
}

OptionList* npc_test::getcurrentoptions() {
	return &test;
}

Entity* npc_test::spawnme(float x, float y, int act, int spawnflags) {
	npc_test* ent = new npc_test();
	NPC* npc = (NPC*)ent;
	npc->ptInteractingWith = nullptr;
	ent->x = x;
	ent->y = y;
	ent->w = 0.6f;
	ent->h = 0.6f;
	ent->spawnflags = spawnflags;
	ent->uuid = genuuid();
	ent->classname = "npc_test";

	//ent->materialHandle = trap->RegisterMaterial("TestCharacter");

	ent->bShouldWeCollide = false; // Don't worry about colliding with NPCs
	ent->bShouldWeRender = true;
	ent->bShouldWeThink = true;
	ent->iAct = act;
	return ent;
}

string npc_test::getname() {
	return "NPC";
}
#include "sys_local.h"
#include "../json/cJSON.h"

MaterialHandler* mats = nullptr;

void MaterialHandler::LoadMaterial(const char* matfile) {
}

MaterialHandler::MaterialHandler() {
}

MaterialHandler::~MaterialHandler() {
	Zone::FreeAll("materials");
}

Material* MaterialHandler::GetMaterial(const char* material) {
	return nullptr;
}

Material* MaterialHandler::RegisterMaterial(const char* material) {
	return nullptr;
}

void MaterialHandler::InitMaterials() {
	R_Message(PRIORITY_NOTE, "Initializing materials...\n");
	mats = new MaterialHandler();
}

void MaterialHandler::ShutdownMaterials() {
	R_Message(PRIORITY_NOTE, "Freeing materials...\n");
	delete mats;
}
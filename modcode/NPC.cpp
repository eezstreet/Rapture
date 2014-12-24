#include "NPC.h"
#include "Server.h"

void NPC::interact(Entity* interacter) {
	if(ptInteractingWith != nullptr) {
		// Don't interact with us if we're busy..
		R_Message(PRIORITY_MESSAGE, "%s is busy\n", getname().c_str());
		return;
	}
	ptInteractingWith = (Actor*)interacter;
	ptServer->GetClient()->NPCStartInteraction(this, *getcurrentoptions());
}

bool NPC::mouseover() {
	ptClient->StartLabelDraw(getname().c_str());
	ptClient->ptFocusEnt = this;
	return true;
}

void NPC::StopInteraction(Entity* npc, Entity* interacter) {
	NPC* thisNPC = (NPC*)npc;
	thisNPC->ptInteractingWith = nullptr;
}

void NPC::StopClientFromInteracting(Entity* npc, Entity* interacter) {
	ptServer->GetClient()->NPCPickMenu(-1, true);
}

void NPC::OpenSubmenu(OptionList& rtOptionList) {
	ptServer->GetClient()->NPCChangeMenu(rtOptionList);
}
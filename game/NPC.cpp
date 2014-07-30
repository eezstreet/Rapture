#include "NPC.h"
#include "Client.h"

void NPC::interact(Entity* interacter) {
	if(ptInteractingWith != nullptr) {
		// Don't interact with us if we're busy..
		R_Message(PRIORITY_MESSAGE, "%s is busy\n", getname().c_str());
		return;
	}
	ptInteractingWith = (Actor*)interacter;
	thisClient->NPCStartInteraction(this, *getcurrentoptions());
}

bool NPC::mouseover() {
	thisClient->StartLabelDraw(getname().c_str());
	thisClient->ptFocusEnt = this;
	return true;
}

void NPC::StopInteraction(Entity* npc, Entity* interacter) {
	NPC* thisNPC = (NPC*)npc;
	thisNPC->ptInteractingWith = nullptr;
}

void NPC::StopClientFromInteracting(Entity* npc, Entity* interacter) {
	// NETWORK FIXME
	thisClient->NPCPickMenu(-1, true);
}
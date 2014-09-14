#include "Entity.h"
#include "OptionList.h"
#include "Actor.h"

class NPC : public Actor {
public:
	virtual OptionList* getcurrentoptions() = 0;
	virtual void interact(Entity* interacter);
	virtual bool mouseover();
	virtual string getname() = 0;

	static void StopInteraction(Entity* npc, Entity* interacter);
	static void StopClientFromInteracting(Entity* npc, Entity* interacter);
	static void OpenSubmenu(OptionList& rtOptionList);
	Actor* ptInteractingWith;
};
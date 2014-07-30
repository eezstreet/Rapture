#include "Entity.h"
#include "Actor.h"
#include "NPC.h"

class npc_test : public NPC {
public:
	virtual void render();
	virtual void think();
	virtual void spawn();

	static Entity* spawnme(float x, float y, int spawnflags, int act);

	virtual string getname();
	virtual OptionList* getcurrentoptions();
private:
	Entity* ptInteractingWith;
};
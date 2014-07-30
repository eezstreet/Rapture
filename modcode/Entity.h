#pragma once
#include "QuadTree.h"
#include "Local.h"

/////////////////////
//
//  Entities
//	An entity is an object in the
//	world, such as a monster or a player.
//	These entities may also be objects
//	without AI, such as chests.
//
/////////////////////

class Entity : public QTreeNode<float> {
protected:
	string uuid;
	int spawnflags;
	Material* materialHandle;
	AnimationManager* ptAnims;
	bool bIsHighlighted;
	int iDepthScoreOffset;
public:
	virtual void render() = 0;
	virtual void think() = 0;
	virtual void spawn() = 0;
	virtual void interact(Entity* interacter) = 0;
	virtual bool mouseover() = 0;
	unsigned int iAct;

	bool bShouldWeRender;
	bool bShouldWeThink;
	bool bShouldWeCollide;

	int GetSpawnflags() { return spawnflags; }
	string classname;
	QuadTree<Entity, float>* ptContainingTree;

	Entity() {
		iDepthScoreOffset = 0; // This needs to be always 0 (unless specified) otherwise bad things happen
	}

	static int GetDrawX(Entity* in);
	static int GetDrawY(Entity* in);
friend struct Worldspace;
};
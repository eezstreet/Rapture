#include "Entity.h"
#include "DungeonManager.h"
#include "Worldspace.h"


int Entity::GetDrawX(Entity* in) {
	int worldX = Worldspace::WorldPlaceToScreenSpaceIX(in->x, in->y);
	int offsetX = ptDungeonManager->GetWorld(in->iAct)->PlayerOffsetX(thisClient->ptPlayer);

	return worldX + offsetX;
}

int Entity::GetDrawY(Entity* in) {
	int worldY = Worldspace::WorldPlaceToScreenSpaceIY(in->x, in->y);
	int offsetY = ptDungeonManager->GetWorld(in->iAct)->PlayerOffsetY(thisClient->ptPlayer);

	return worldY + offsetY;
}
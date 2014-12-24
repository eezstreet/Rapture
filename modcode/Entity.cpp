#include "Entity.h"
#include "DungeonManager.h"
#include "Worldspace.h"
#include "Server.h"


int Entity::GetDrawX(Entity* in) {
	int worldX = Worldspace::WorldPlaceToScreenSpaceIX(in->x, in->y);
	int offsetX = ptServer->ptDungeonManager->GetWorld(in->iAct)->PlayerOffsetX(ptServer->GetClient()->ptPlayer);

	return worldX + offsetX;
}

int Entity::GetDrawY(Entity* in) {
	int worldY = Worldspace::WorldPlaceToScreenSpaceIY(in->x, in->y);
	int offsetY = ptServer->ptDungeonManager->GetWorld(in->iAct)->PlayerOffsetY(ptServer->GetClient()->ptPlayer);

	return worldY + offsetY;
}
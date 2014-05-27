#include "g_local.h"

SpatialEntity::SpatialEntity(Entity* from) {
	uuid = from->uuid;
	x = from->x;
	y = from->y;
	w = from->w;
	h = from->h;
}

SpatialEntity::SpatialEntity() {
}
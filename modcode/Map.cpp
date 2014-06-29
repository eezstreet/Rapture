#include "g_local.h"

vector<Entity*> Map::FindEntities(const string& classname) {
	vector<Entity*> returnValue;
	vector<Entity*> mapEnts = qtEntTree.NodesIn((float)x, (float)y, (float)w, (float)h);
	for(auto it = mapEnts.begin(); it != mapEnts.end(); ++it) {
		if(classname == (*it)->classname) {
			returnValue.push_back((*it));
		}
	}

	return returnValue;
}
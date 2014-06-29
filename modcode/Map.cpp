#include "g_local.h"

vector<Entity*> Map::FindEntities(const string& classname) {
	vector<Entity*> returnValue;
	vector<Entity*> mapEnts = qtEntTree.NodesIn(x, y, w, h);
	for(auto it = mapEnts.begin(); it != mapEnts.end(); ++it) {
		if(classname == (*it)->classname) {
			returnValue.push_back((*it));
		}
	}

	return returnValue;
}
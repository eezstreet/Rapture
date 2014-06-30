#pragma once
#include "QuadTree.h"
#include "Player.h"
#include "Map.h"
#include "Tile.h"
#include "Client.h"

/////////////////////
//
//  Worldspace
//	The worldspace contains everything for one whole act.
//
/////////////////////

struct Worldspace {
public:
	Worldspace();
	~Worldspace();
	QuadTree<Map, int>* qtMapTree;

	void InsertInto(Map* theMap);
	void SpawnEntity(Entity* ent, bool bShouldRender, bool bShouldThink, bool bShouldCollide);
	void AddPlayer(Player* ptPlayer);
	Player* GetFirstPlayer();
	Player* FindPlayerByNumber(unsigned int playerNum);

	void Run();
	void Render(Client* ptClient);

	static float WorldPlaceToScreenSpaceFX(float x, float y);
	static float WorldPlaceToScreenSpaceFY(float x, float y);
	static int WorldPlaceToScreenSpaceIX(int x, int y);
	static int WorldPlaceToScreenSpaceIY(int x, int y);
	static float ScreenSpaceToWorldPlaceX(int x, int y, Player* ptPlayer);
	static float ScreenSpaceToWorldPlaceY(int x, int y, Player* ptPlayer);

	float PlayerOffsetX(Player* ptPlayer);
	float PlayerOffsetY(Player* ptPlayer);

	void UpdateEntities();
	void ActorMoved(Actor* ptActor);
private:
	unordered_map<string, Entity*> mRenderList;
	unordered_map<string, Entity*> mThinkList;
	unordered_map<string, Entity*> mCollideList;

	vector<Player*> vPlayers;
	vector<Actor*> vActorsMoved;
};
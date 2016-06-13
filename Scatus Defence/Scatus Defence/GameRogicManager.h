#pragma once
#include "ResourceMgr.h"
#include "SceneMgr.h"
#include "2DMap.h"
#include "RogicTimer.h"
#include "GameTimer.h"
#include <algorithm>
#include "ObjectMgr.h"
#include "CollisionMgr.h"
#include "ResourceMgr.h"
#include "Goblin.h"
#include "Cyclop.h"
#define G_Rogic_Mgr  GameRogicManager::Instance()
class GameRogicManager
{
public:
	GameRogicManager();
	~GameRogicManager();

public:
	void Init(ID3D11Device* device);

	void Update(float dt);
	bool OnMouseDown;

	Player* GetPlayer() { return mPlayer; }
	const std::vector<GameObject*>&		GetAllObjects() { return mObjectMgr.GetAllObjects(); }
	void Gamestart();
	void GameEnd();
	void GameTitle();

	void Waving(float dt);

	void MoveAI();
	void AIManager(float dt);
	void Add_Monster(UINT waveLevel);
	void Printloc();
	ResourceMgr mResourceMgr;
	ObjectMgr	mObjectMgr;
	CollisionMgr mCollisionMgr;
	static GameRogicManager* Instance();
private:


private:


	UINT mWaveLevel;
	UINT m_enemysnum;
	UINT player_num;
	Player* mPlayer;
	std::string gamename;
	POINT mLastMousePos;
	Gamestate_type mGameState;

	Map* m_pMap;



	std::list<GameObject*> m_Our_list;
	std::list<GameObject*> m_Enemies_list;
	std::unordered_map<UINT, std::unordered_map<ObjectType::Types, UINT>> mPerWaveMonsterNum;

};


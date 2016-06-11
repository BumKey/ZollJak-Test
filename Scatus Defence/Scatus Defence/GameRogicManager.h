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

private:
	void Gamestart();
	void GameEnd();
	void GameTitle();
	void StartWave();
	void EndWave();
	void Waving(float dt);
	void Waiting_Wave();
	void MoveAI();
	void AIManager(float dt);
	void add_Monster(UINT waveLevel);
	void printloc();

private:
	ResourceMgr mResourceMgr;
	ObjectMgr	mObjectMgr;
	CollisionMgr mCollisionMgr;

	UINT mWaveLevel;
	UINT m_enemysnum;
	UINT player_num;
	Player* mPlayer;
	std::string gamename;
	POINT mLastMousePos;
	Gamestate_type mGameState;

	Map* m_pMap;
	GameTimer mTimer;
	RogicTimer mRogicTimer;

	std::list<GameObject*> m_Our_list;
	std::list<GameObject*> m_Enemies_list;
	std::unordered_map<UINT, std::unordered_map<ObjectType::Types, UINT>> mPerWaveMonsterNum;
	//string nextwave_time;
	//string remain_time;
	//string total_playtime;
};


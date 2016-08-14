#pragma once
#include <algorithm>

#include "2DMap.h"
#include "RogicTimer.h"
#include "GameTimer.h"
#include "Singletone.h"
#include "ObjectMgr.h"
#include "ResourceMgr.h"
#include "CollisionMgr.h"
#include "Goblin.h"
#include "Cyclop.h"

#define Rogic_Mgr GameRogicManager::GetInstance()

class GameRogicManager : public Singletone<GameRogicManager>
{
private:
	GameRogicManager();
	~GameRogicManager();

	friend class Singletone<GameRogicManager>;
public:
	void Update(float dt);
	bool OnMouseDown;

	void Gamestart();
	void GameEnd();
	void GameTitle();

	void Waving(float dt);

	void MoveAI();
	void AIManager(float dt);
	void Add_Monster(UINT waveLevel);
	void Printloc();

private:
	UINT mWaveLevel;
	UINT m_enemysnum;
	UINT player_num;

	std::string gamename;
	POINT mLastMousePos;
	Gamestate_type mGameState;

	Map* m_pMap;

	std::list<GameObject*> m_Our_list;
	std::list<GameObject*> m_Enemies_list;
	std::unordered_map<UINT, std::unordered_map<ObjectType::Types, UINT>> mPerWaveMonsterNum;

};


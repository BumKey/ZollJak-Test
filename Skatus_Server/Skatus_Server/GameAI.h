#pragma once
#include <algorithm>

#include "RogicTimer.h"
#include "ObjectMgr.h"
#include "CollisionMgr.h"
#include "Protocol.h"
#include <minwindef.h>
#include <unordered_map>

class GameAI 
{
private:
	GameAI();
	~GameAI();

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
	UINT mEnemyNum;
	UINT mPlayerNum;

	Gamestate_type mGameState;

	std::unordered_map<UINT, std::unordered_map<ObjectType::Types, UINT>> mPerWaveMonsterNum;

};


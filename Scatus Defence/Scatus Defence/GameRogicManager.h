#pragma once
#include "ResourceMgr.h"
#include "SceneMgr.h"
#include "2DMap.h"
#include "RogicTimer.h"
#include "GameTimer.h"
#include <algorithm>
#include "ObjectMgr.h"
#include "p_Warrior.h"
#include "ResourceMgr.h"
#include "Goblin.h"
using namespace std;



class GameRogicManager
{
public:
	GameRogicManager(ObjectMgr * ObjMgr_, ResourceMgr * m_resourceMgr);
	~GameRogicManager();

public:
	void Update(float dt);
	bool OnMouseDown;

private:
	void Gamestart();
	void GameEnd();
	void GameTitle();
	void StartWave();
	void EndWave();
	void Waving(float dt);
	void Waiting_Wave();
	void printloc();
	void MoveAI();
	void AIManager(float dt);
	void add_Monster();

private:
	RogicTimer mRogicTimer;
	ObjectMgr * mObjMgr; // 오브젝트매니저에 접근하기 위한 포인터
	int wave_level;
	int Gamestatement;
	int m_enemysnum;
	string gamename = "스카투스 디펜스";
	int player_num;
	Map * m_pMap;
	ResourceMgr * mResourceMgr;
	POINT mLastMousePos;
	Player* mPlayer;
	std::list<GameObject*> m_Our_list;
	std::list<GameObject*> m_Enemies_list;
	GameTimer mTimer;
	//string nextwave_time;
	//string remain_time;
	//string total_playtime;
};


#pragma once
#include "ResourceMgr.h"
#include "SceneMgr.h"
#include "2DMap.h"
#include "RogicTimer.h"
#include "GameTimer.h"
#include <algorithm>
#include "ObjectMgr.h"
#include "ResourceMgr.h"
#include "Goblin.h"

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
	ObjectMgr * mObjMgr; // ������Ʈ�Ŵ����� �����ϱ� ���� ������
	int wave_level;
	int Gamestatement;
	int m_enemysnum;
	std::string gamename = "��ī���� ���潺";
	int player_num;
	ResourceMgr * mResourceMgr;
	POINT mLastMousePos;
	Player* mPlayer;
	std::list<GameObject*> m_Our_list;
	std::list<GameObject*> m_Enemies_list;
	RogicTimer mRogicTimer;
	GameTimer mTimer;
	Map* m_pMap;
	//string nextwave_time;
	//string remain_time;
	//string total_playtime;
};


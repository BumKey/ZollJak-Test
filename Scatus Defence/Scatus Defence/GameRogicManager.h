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

//나 돌아갈래

class GameRogicManager
{
public:
	RogicTimer mRogicTimer;
	ObjectMgr * m_ObjMgr; // 오브젝트매니저에 접근하기 위한 포인터
	int wave_level;
	int Gamestatement;
	int m_enemysnum;
	string gamename = "스카투스 디펜스";
	int player_num;
	Map * m_pMap;
	ResourceMgr * m_resourceMgr;
	POINT mLastMousePos;
	bool m_OnMouseDown;
	Player* mPlayer;
	std::list<GameObject*> m_Our_list;
	std::list<GameObject*> m_Enemies_list;
	GameTimer mTimer;
	//string nextwave_time;
	//string remain_time;
	//string total_playtime;


	GameRogicManager(ObjectMgr * ObjMgr_, ResourceMgr * m_resourceMgr);
	~GameRogicManager();

	void GameRogicManager::GameManaging();
	void Gamestart();
	void Update();
	void GameEnd();
	void GameTitle();
	void StartWave();
	void EndWave();
	void Waving();
	void Waiting_Wave();
	void GameRogicManager::SetPlayer();
	void GameRogicManager::OnMouseDown(WPARAM btnState, int x, int y);
	void GameRogicManager::OnMouseMove(WPARAM btnState, int x, int y);
	void GameRogicManager::OnMouseUp(WPARAM btnState, int x, int y);
	void GameRogicManager::GetKeyMesage();
	void GameRogicManager::printloc();
	void Setting_Team();
	void GameRogicManager::MoveAI();
	void AIManager();
	void GameRogicManager::Setting_teamlist();
	void GameRogicManager::add_Our(GameObject* a);
	void add_Monster();


};


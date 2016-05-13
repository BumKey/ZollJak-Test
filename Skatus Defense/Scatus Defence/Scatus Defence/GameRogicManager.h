#pragma once
#include "ResourceMgr.h"
#include "SceneMgr.h"
#include "Map.h"
#include "RogicTimer.h"
#include "GameTimer.h"
#include <algorithm>
using namespace std;



class GameRogicManager
{
public:
	RogicTimer mRogicTimer;
	SceneMgr * m_SceneMgr; // 씬매니저의 오브젝트에 접근하기 위한 포인터
	int wave_level;
	int Gamestatement;
	int m_enemysnum;
	string gamename = "스카투스 디펜스";
	int player_num;
	Map * m_pMap;
	POINT mLastMousePos;
	bool m_OnMouseDown;
	GameObject* mPlayer;
	Warrior * mPlayer_w;
	std::list<GameObject*> m_Our_list;
	std::list<GameObject*> m_Enemies_list;
	GameTimer mTimer;
	//string nextwave_time;
	//string remain_time;
	//string total_playtime;


	void GameRogicManager::GameManaging();
	GameRogicManager(SceneMgr *SceneMgr_);
	~GameRogicManager();
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
	void GameRogicManager::add_Enemies(GameObject* a);


};


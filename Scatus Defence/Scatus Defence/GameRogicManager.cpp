#include "GameRogicManager.h"
#include "sceneMgr.h"
#include "time.h"
#include "ResourceMgr.h"

GameRogicManager::GameRogicManager() : mWaveLevel(1), mGameState(Gamestate_type::game_title),
gamename("스카투스 디펜스"), player_num(0), OnMouseDown(false), mPlayer(nullptr)
{
	m_pMap = new Map();
	m_pMap->Load("Skatus_Map.txt");
	//map; 추후에 초기화 진행
	//mLastMousePos;

	// 1레벨 웨이브 몬스터 갯수
	mPerWaveMonsterNum[1][ObjectType::Goblin] = 20;
	mPerWaveMonsterNum[1][ObjectType::Cyclop] = 2;

	// 추후 알아서 추가..
};

GameRogicManager::~GameRogicManager()
{

};


void GameRogicManager::Gamestart()
{
	//GameState = game_waiting_wave;
	GameState = game_waving;
	printf("게임이 시작되었습니다.");
	mRogicTimer.SetBeforeTime(); //플레이타임 측정시작
	mRogicTimer.SetWaveTimer(); //웨이브스위치 작동

	// 새로운 적들 생성
	Add_Monster(1);

	Printloc();

	//m_pMap->print();
	//게임 각종 오브젝트 추가

}
void GameRogicManager::GameEnd()
{
	mObjectMgr.ReleaseAll();
	printf("\n게임이 종료되었습니다\n\n", mWaveLevel);

}
void GameRogicManager::GameTitle()
{
	printf("\n게임 시작을 위해 아무 키나 누르시오\n");

}

void GameRogicManager::Init(ID3D11Device * device)
{
	mResourceMgr.Init(device);
	mObjectMgr.Init(&mResourceMgr);
	mCollisionMgr.Init(&mObjectMgr);

	mObjectMgr.Update();

	//플레이어설정
	mPlayer = mObjectMgr.GetPlayer();
}

void GameRogicManager::Update(float dt)
{
	//printf("스카투스 디펜스!!\n");
	mRogicTimer.SetCurrentTime();// 현재시간 측정 및 총 플레이타임 측정함수
								 //printf("\n\n\n\n\n\n\n현재 플레이 타임 : %d초",mRogicTimer.GetPlayTime());

	switch (GameState)
	{
	case game_title:
		if (OnMouseDown)
		{
			Gamestart();
		}
		else
			GameTitle();
		break;
	case game_start:


		break;
	case game_waving:
		Waving(dt);
		break;
	case game_waiting_wave:

		Waiting_Wave();
		break;
	case game_ending:
		GameEnd();
		break;
	default:
		break;
	}
	//system("cls");//콘솔창 지우기

	mObjectMgr.Update(dt);
	mCollisionMgr.Update(dt);

}



void GameRogicManager::EndWave()
{
	if (mWaveLevel == 100)
	{
		GameState = game_ending;
	}
	else
	{

		GameState = game_waiting_wave;
	}
	//mRogicTimer.SetWaveTimer(); //WaveTimer초기화
								//맵 내부의 모든 적들 삭제
	mObjectMgr.ReleaseAllMonsters();
	/*
	for (std::list<GameObject*>::iterator i = m_SceneMgr->mObjects.begin(); i != m_SceneMgr->mObjects.end();)
	{
	if ((*i)->Get_Object_type() == type_monster)
	{
	i = m_SceneMgr->mObjects.erase(i);


	}
	else
	{
	++i;
	}

	}
	for (std::list<GameObject*>::iterator i = m_Enemies_list.begin(); i != m_Enemies_list.end();)
	{
	if ((*i)->Get_Object_type() == type_monster)
	{
	i = m_Enemies_list.erase(i);


	}
	else
	{
	++i;
	}

	}
	*/
	//waiting_wave로 상태변경
}
void GameRogicManager::StartWave()
{
	mRogicTimer.SetWaveTimer(); //WaveTimer초기화
	GameState = game_waving;
	mWaveLevel++;
	printf("\n\n\n%d 레벨 웨이브가 시작되었습니다\n", mWaveLevel);
	printf("\n 몬스터를 생성중입니다\n", mWaveLevel);
	//m_pMap->print();

	//wavingstart로 상태변경


}

void GameRogicManager::Waving(float dt)
{
	//next_wave 카운팅
	bool timer= mRogicTimer.WaveTimer();
	AIManager(dt);
	if (timer)
	{

		EndWave();

	}
}

void GameRogicManager::Waiting_Wave()
{
	//1초마다 nextwave_time 1씩 감소
	//bool timer = mRogicTimer.WaveTimer();
}

void GameRogicManager::Printloc()
{
	printf("\n");
	mPlayer->PrintLocation();
	for (auto i : mObjectMgr.GetMonsters())
	{
		i->PrintLocation();
	}
}

void GameRogicManager::Add_Monster(UINT waveLevel)
{
	assert(waveLevel > 0);

	InstanceDesc info;

	UINT goblinNum = mPerWaveMonsterNum[waveLevel][ObjectType::Goblin];
	for (UINT i = 0; i < goblinNum; ++i)
	{
		info.Pos = XMFLOAT3(mPlayer->GetPos().x + 50.0f + rand() % 150,
			-0.1f, mPlayer->GetPos().z + 50.0f + rand() % 150);
		info.Scale = 0.1f + MathHelper::RandF() / 5.0f;
		info.Rot.y = 0;

		Goblin::Type type;
		if (i % 2) {
			type = Goblin::Type::Blue;
			info.Scale = 0.4f;
		}
		else if (i % 3)
		{
			type = Goblin::Type::Red;
			info.Scale = 0.3f;
		}
		else if (i % 5)
		{
			type = Goblin::Type::Red;
			info.Scale = 0.7f;
		}
		else {
			type = Goblin::Type::Blue;
			info.Scale = 0.6f;
		}
		mObjectMgr.AddMonster(new Goblin(mResourceMgr.GetSkinnedMesh(ObjectType::Goblin), info, type));
	}

	UINT cyclopNum = mPerWaveMonsterNum[waveLevel][ObjectType::Cyclop];
	for (UINT i = 0; i < cyclopNum; ++i) {
		info.Pos = XMFLOAT3(mPlayer->GetPos().x + 50.0f + rand() % 150,
			-0.1f, mPlayer->GetPos().z + 50.0f + rand() % 150);
		info.Scale = 7.0f + MathHelper::RandF();
		info.Rot.x = MathHelper::Pi;
		info.Rot.y = 0.0f;

		mObjectMgr.AddMonster(new Cyclop(mResourceMgr.GetSkinnedMesh(ObjectType::Cyclop), info));
	}
}
void GameRogicManager::MoveAI()
{

}
void GameRogicManager::AIManager(float dt)
{
	// 적들의 행동지정	
	//타겟변경은 나중에 0.5초마다 한번씩
	for (auto iterM : mObjectMgr.GetMonsters())
	{
		// 현재 타겟은 오로지 플레이어로 설정.
		// 추후 타겟을 신전이나 다른 플레이어로 바꾸는 기능 추가할 것.
		iterM->SetTarget(mPlayer);
		
		// 거리가 가까우면 공격
		if (mCollisionMgr.DetectWithPlayer(iterM)) 
			iterM->SetAI_State(AI_State::AttackToTarget);
		else 
			iterM->SetAI_State(AI_State::MovingToTarget);
	}
}
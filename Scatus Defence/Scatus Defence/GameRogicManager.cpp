#include "GameRogicManager.h"
#include "sceneMgr.h"
#include "time.h"
#include "ResourceMgr.h"

GameRogicManager::GameRogicManager(ObjectMgr * ObjMgr_, ResourceMgr * resourceMgr_)
{
	mResourceMgr = resourceMgr_;
	mObjMgr = ObjMgr_; // 씬매니저의 오브젝트에 접근하기 위한 포인터
	wave_level = 0;
	Gamestatement= game_title;
	gamename = "스카투스 디펜스";
	m_pMap = new Map();
	m_pMap->Load("Skatus_Map.txt");
	player_num = 0;
	//map; 추후에 초기화 진행
	//mLastMousePos;
	OnMouseDown=false;
	mPlayer = NULL;

	mPlayer = mObjMgr->GetPlayer();

};

GameRogicManager::~GameRogicManager()
{

};

void GameRogicManager::Gamestart()
{
	GameState = game_waiting_wave;
	printf("게임이 시작되었습니다.");
	mRogicTimer.SetBeforeTime(); //플레이타임 측정시작
	mRogicTimer.SetWaveTimer(); //웨이브스위치 작동
	printloc();
	//플레이어설정
	mPlayer = mObjMgr->GetPlayer();

	//m_pMap->print();
	//게임 각종 오브젝트 추가

	
}
void GameRogicManager::GameEnd()
{
	mObjMgr->ReleaseAll(*mResourceMgr);
	printf("\n게임이 종료되었습니다\n\n", wave_level);
	
}
void GameRogicManager::GameTitle()
{
	printf("\n게임 시작을 위해 아무 키나 누르시오\n");
	
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
}



void GameRogicManager::EndWave()
{

	if (wave_level==100)
	{
		GameState = game_ending;
	}
	else
	{
	
		GameState = game_waiting_wave;
	}
	mRogicTimer.SetWaveTimer(); //WaveTimer초기화
	//맵 내부의 모든 적들 삭제
	mObjMgr->ReleaseAllMonsters(*mResourceMgr);
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
	wave_level++;
	printf("\n\n\n%d 레벨 웨이브가 시작되었습니다\n", wave_level);
	printf("\n 몬스터를 생성중입니다\n", wave_level);
	m_pMap->print();

	//for (int i = 0; i < 2;i++)
	//{
	//	 add_Monster();
	//	auto it = m_ObjMgr->GetAllMonsters().end();
	//	it--;
	//
	//	printf("고블린 %d의 위치 x: %f y:%f\n", i, (*it)->GetPos2D().x, (*it)->GetPos2D().y);
	//}

	// 새로운 적들 생성
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
	bool timer = mRogicTimer.WaveTimer();
	if (timer);
	{
		
		StartWave();

	}
}

void GameRogicManager::printloc()
{
	mObjMgr->GetPlayer()->PrintLocation();
	for (auto i : mObjMgr->GetMonsters())
	{
		i->PrintLocation();
	}
}

void GameRogicManager::add_Monster()
{
	InstanceDesc info;
	
	info.Pos = XMFLOAT3(mPlayer->GetPos().x + 50.0f - rand() % 100,
		0, mPlayer->GetPos().z + 50.0f - rand() % 100);
	info.Scale = MathHelper::RandF()*2.0f + 0.5f;
	info.Yaw = MathHelper::RandF()*MathHelper::Pi * 2;

	GoblinType type;
	if (rand() % 2) type = GoblinType::Red;
	else	   type = GoblinType::Blue;
	mObjMgr->AddMonster(new Goblin(mResourceMgr->GetGoblinMesh(), info, type));
}
void GameRogicManager::MoveAI()
{

}
void GameRogicManager::AIManager(float dt)
{
	int j = 0;
	j = 0;
	// 적들의 행동지정	
	//타겟변경은 나중에 0.5초마다 한번씩
	for (auto iterM : mObjMgr->GetMonsters())
	{
		//나와 가장가까운 적을 타겟으로 변경
		if (iterM->GetState() == type_idle || iterM->GetState() == type_walk)
		{
			//이동
			iterM->SetTarget(mPlayer);
			printf("적 세팅");
			//기지와 나사이의 거리가 적과 나 사이의 거리보다 가깝다면 타겟을 기지로 변경) 
		
			Vector2D currPos = iterM->GetPos2D();
			Vector2D targetPos = iterM->GetTarget()->GetPos2D();
			if (Vec2DDistance(currPos, m_pMap->temple_loc)/*나와 기지 사이의 거리*/
				< Vec2DDistance(currPos, targetPos)/*나와 타겟 사이의 거리*/)
			{
				iterM->SetTarget(nullptr); // 나중에 신전 객체 등장하면 신전 객체로 바꿔줘야함
			}

			iterM->MoveTo(targetPos, dt);

			printf("고블린 %d의 위치 x: %f y:%f\n", j, currPos.x, currPos.y);
			if (iterM->GetTarget() == NULL) //신전이 목표일때
			{

				if (Vec2DDistance(currPos, m_pMap->temple_loc) < 10)
				{

					//i->SetObj_State(type_battle); //공격으로 상태전환
					printf("신전공격");
				}
				else
				{
					/*i->SetHeading((m_pMap->temple_loc) - (i->GetPos2D()));
					i->Move2D(i->Heading(), 0.03);*/
				}
			}
			else
			{
				if (Vec2DDistance(currPos, targetPos) <= 3.0f)
				{
					iterM->SetState(type_battle);
					printf("배틀실행");
				}
			}

		}
		else if (iterM->GetState() == type_battle)
		{
			mRogicTimer.SetAttackTimer(); //공격시간의 간격을 두기 위한 타이머 설정
			iterM->SetState(type_attack); //공격으로 상태전환

		}
		else if (iterM->GetState() == type_attack)
		{
			if (mRogicTimer.AttackTimer(iterM->GetProperty().attackspeed))//공격시간이 되면 공격
			{
				iterM->SetState(type_battle);
				iterM->Attack(iterM->GetTarget());  //공격시간의 간격을 두기 위한 타이머 설정
			}
		}
	}
	j++;
}



	
	


		



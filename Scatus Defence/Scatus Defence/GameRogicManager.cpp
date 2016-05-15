#include "GameRogicManager.h"
#include "sceneMgr.h"
#include "time.h"
#include "ResourceMgr.h"

GameRogicManager::GameRogicManager(ObjectMgr * ObjMgr_, ResourceMgr * resourceMgr_)
{
	m_resourceMgr = resourceMgr_;
	m_ObjMgr = ObjMgr_; // 씬매니저의 오브젝트에 접근하기 위한 포인터
	wave_level = 0;
	Gamestatement= game_title;
	gamename = "스카투스 디펜스";
	m_pMap = new Map();
	m_pMap->Load("Skatus_Map.txt");
	player_num = 0;
	//map; 추후에 초기화 진행
	//mLastMousePos;
	m_OnMouseDown=false;
	mPlayer = NULL;



};

GameRogicManager::~GameRogicManager()
{

};

void GameRogicManager::GameManaging()
{	
	//printf("스카투스 디펜스!!\n");
	mRogicTimer.SetCurrentTime();// 현재시간 측정 및 총 플레이타임 측정함수
	//printf("\n\n\n\n\n\n\n현재 플레이 타임 : %d초",mRogicTimer.GetPlayTime());
	
	switch (GameState)
	{
	case game_title:
		if (m_OnMouseDown)
		{
			Gamestart();
		}
		else
			GameTitle();
		break;
	case game_start:
		
	
		break;
	case game_waving:
		Waving();
		break;
	case game_waiting_wave :

		Waiting_Wave();
		break;
	case game_ending :
		GameEnd();
		break;
	default:
		break;
	}
	//system("cls");//콘솔창 지우기
}
void GameRogicManager::Gamestart()
{
	GameState = game_waiting_wave;
	printf("게임이 시작되었습니다.");
	mRogicTimer.SetBeforeTime(); //플레이타임 측정시작
	mRogicTimer.SetWaveTimer(); //웨이브스위치 작동
	printloc();
	//플레이어설정
	SetPlayer();
	Setting_Team();
	Setting_teamlist();
	//m_pMap->print();
	//게임 각종 오브젝트 추가
	mPlayer = m_ObjMgr->GetPlayer();

	
}
void GameRogicManager::GameEnd()
{
	m_ObjMgr->ReleaseAll(*m_resourceMgr);
	printf("\n게임이 종료되었습니다\n\n", wave_level);
	
}
void GameRogicManager::GameTitle()
{
	printf("\n게임 시작을 위해 아무 키나 누르시오\n");
	
}

void GameRogicManager::Update()
{
	GameManaging();
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
	m_ObjMgr->ReleaseAllMonsers(*m_resourceMgr);
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

	
		

	for (int i = 0; i < 2;i++)
	{
		add_Monster();
		//auto it = m_ObjMgr->GetAllMonsters().end();
		//it--;
	
		//printf("고블린 %d의 위치 x: %f y:%f\n", i, (*it)->GetPos2D().x, (*it)->GetPos2D().y);
	}

	// 새로운 적들 생성

	//팀분류 시작
	Setting_Team();
	//wavingstart로 상태변경
	

}

void GameRogicManager::Waving()
{
	
	
	//next_wave 카운팅
	bool timer= mRogicTimer.WaveTimer();
	AIManager();
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
void GameRogicManager::GetKeyMesage()
{

	if (GetAsyncKeyState('W') & 0x8000)
	{
		
	}
	if (GetAsyncKeyState('S') & 0x8000)
	{
	}
	if (GetAsyncKeyState('A') & 0x8000)
	{
	}
	if (GetAsyncKeyState('D') & 0x8000)
	{
	}

}

void GameRogicManager::OnMouseDown(WPARAM btnState, int x, int y)
{

	m_OnMouseDown = true;
	if (GameState==game_waving || GameState == game_waiting_wave)
	{
		mPlayer->SetObj_State(type_attack);
		//플레이어 상태 공격상태로 변환/ 맵 내부의 고블린과 충돌검사 진행
	}
}
void GameRogicManager::OnMouseMove(WPARAM btnState, int x, int y)
{

}
void GameRogicManager::OnMouseUp(WPARAM btnState, int x, int y)
{

}

void GameRogicManager::printloc()
{
	for (auto i : m_ObjMgr->GetAllObjects())
	{
		if ( i->Get_Object_type() == type_monster || i->Get_Object_type() == type_p_warrior) {
			i->printlocation();
		}
	}


}

void GameRogicManager::SetPlayer()
{


		for (auto i :m_ObjMgr->GetAllObjects())
		{
			if (i->Get_Object_type() == type_p_warrior) {
				mPlayer = (p_Warrior*)i; // 문제가 될 수 있는 부분
			}
		}

}

void GameRogicManager::Setting_Team()
{
	//경고 몬스터, 아군 죽을때 마다 초기화 해줘야함
	//중복검사를 해야함
	// type_을 한데 정리해서 Get_Object_type>10 적 <10 아군이런 식으로 바꾸자 나중에

	for (auto i : m_ObjMgr->GetAllMonsters())
	{
			i->Oppenents = m_ObjMgr->GetAllOurTeam();
	}
	for (auto i : m_ObjMgr->GetAllOurTeam())
	{
		i->Oppenents = m_ObjMgr->GetAllMonsters();
	}
}
void GameRogicManager::Setting_teamlist()
{
	//아마 삭제하면 될듯한 함수
}
void GameRogicManager::add_Our(GameObject* a)
{
	m_Our_list.push_back(a);
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
		m_ObjMgr->AddMonster(new Goblin(m_resourceMgr->GetGoblinMesh(), info, type));



	
}
void GameRogicManager::MoveAI()
{


}
void GameRogicManager::AIManager()
{
	int j = 0;
	j = 0;
	// 적들의 행동지정
	for (auto i : m_ObjMgr->GetAllMonsters())
	{
		

		if (i->Get_Object_type() == type_monster) {//타겟변경은 나중에 0.5초마다 한번씩
												  //나와 가장가까운 적을 타겟으로 변경
			if (i->Get_States() == type_idle || i->Get_States() == type_walk)
			{//이동
				i->SettingTarget(i->Oppenents);
				printf("적 세팅");
				//기지와 나사이의 거리가 적과 나 사이의 거리보다 가깝다면 타겟을 기지로 변경) 
				if (i->current_target_obj != NULL)
				{
					if (Vec2DDistance(i->GetPos2D(), m_pMap->temple_loc)/*나와 기지 사이의 거리*/
						< Vec2DDistance(i->GetPos2D(), i->current_target_obj->GetPos2D())/*나와 타겟 사이의 거리*/)
					{
						i->current_target_obj = NULL; // 나중에 신전 객체 등장하면 신전 객체로 바꿔줘야함

					}
					i->SetHeading((i->current_target_obj->GetPos2D()) - (i->GetPos2D()));
					i->Move2D(i->Heading(), 0.03);
				}
				printf("고블린 %d의 위치 x: %f y:%f\n", j, i->GetPos2D().x, i->GetPos2D().y);
				if (i->current_target_obj == NULL) //신전이 목표일때
				{

					if (Vec2DDistance(i->GetPos2D(), m_pMap->temple_loc) < 10)
					{
						
						//i->SetObj_State(type_battle); //공격으로 상태전환
						printf("신전공격");
					}
					else
					{
						i->SetHeading((m_pMap->temple_loc) - (i->GetPos2D()));
						i->Move2D(i->Heading(), 0.03);
					}
				}
				else
				{
					if (Vec2DDistance(i->GetPos2D(), i->current_target_obj->GetPos2D()) < 10)
					{
						i->SetObj_State(type_battle);
						printf("배틀실행");
					}
				}

			}
			else if (i->Get_States() == type_battle)
			{
				mRogicTimer.SetAttackTimer(); //공격시간의 간격을 두기 위한 타이머 설정
				i->SetObj_State(type_attack); //공격으로 상태전환

			}
			else if (i->Get_States() == type_attack)
			{
				if (mRogicTimer.AttackTimer(i->Get_Properties()->attackspeed))//공격시간이 되면 공격
				{
					i->SetObj_State(type_battle);
					i->Attack(i->current_target_obj);  //공격시간의 간격을 두기 위한 타이머 설정

				}
			}

		}
		j++;

	}

	m_ObjMgr->ReleaseAllDeads(*m_resourceMgr);
	Setting_Team();
}



	
	


		



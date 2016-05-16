#include "GameRogicManager.h"
#include "sceneMgr.h"
#include "time.h"
#include "ResourceMgr.h"

GameRogicManager::GameRogicManager(ObjectMgr * ObjMgr_, ResourceMgr * resourceMgr_)
{
	mResourceMgr = resourceMgr_;
	mObjMgr = ObjMgr_; // ���Ŵ����� ������Ʈ�� �����ϱ� ���� ������
	wave_level = 0;
	Gamestatement= game_title;
	gamename = "��ī���� ���潺";
	m_pMap = new Map();
	m_pMap->Load("Skatus_Map.txt");
	player_num = 0;
	//map; ���Ŀ� �ʱ�ȭ ����
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
	printf("������ ���۵Ǿ����ϴ�.");
	mRogicTimer.SetBeforeTime(); //�÷���Ÿ�� ��������
	mRogicTimer.SetWaveTimer(); //���̺꽺��ġ �۵�
	printloc();
	//�÷��̾��
	mPlayer = mObjMgr->GetPlayer();

	//m_pMap->print();
	//���� ���� ������Ʈ �߰�

	
}
void GameRogicManager::GameEnd()
{
	mObjMgr->ReleaseAll(*mResourceMgr);
	printf("\n������ ����Ǿ����ϴ�\n\n", wave_level);
	
}
void GameRogicManager::GameTitle()
{
	printf("\n���� ������ ���� �ƹ� Ű�� �����ÿ�\n");
	
}

void GameRogicManager::Update(float dt)
{
	//printf("��ī���� ���潺!!\n");
	mRogicTimer.SetCurrentTime();// ����ð� ���� �� �� �÷���Ÿ�� �����Լ�
								 //printf("\n\n\n\n\n\n\n���� �÷��� Ÿ�� : %d��",mRogicTimer.GetPlayTime());

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
	//system("cls");//�ܼ�â �����
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
	mRogicTimer.SetWaveTimer(); //WaveTimer�ʱ�ȭ
	//�� ������ ��� ���� ����
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
	//waiting_wave�� ���º���
}
void GameRogicManager::StartWave()
{
	mRogicTimer.SetWaveTimer(); //WaveTimer�ʱ�ȭ
	GameState = game_waving;
	wave_level++;
	printf("\n\n\n%d ���� ���̺갡 ���۵Ǿ����ϴ�\n", wave_level);
	printf("\n ���͸� �������Դϴ�\n", wave_level);
	m_pMap->print();

	//for (int i = 0; i < 2;i++)
	//{
	//	 add_Monster();
	//	auto it = m_ObjMgr->GetAllMonsters().end();
	//	it--;
	//
	//	printf("��� %d�� ��ġ x: %f y:%f\n", i, (*it)->GetPos2D().x, (*it)->GetPos2D().y);
	//}

	// ���ο� ���� ����
	//wavingstart�� ���º���
	

}

void GameRogicManager::Waving(float dt)
{
	//next_wave ī����
	bool timer= mRogicTimer.WaveTimer();
	AIManager(dt);
	if (timer)
	{
		
		EndWave();
		
	}

}

void GameRogicManager::Waiting_Wave()
{
	//1�ʸ��� nextwave_time 1�� ����
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
	// ������ �ൿ����	
	//Ÿ�ٺ����� ���߿� 0.5�ʸ��� �ѹ���
	for (auto iterM : mObjMgr->GetMonsters())
	{
		//���� ���尡��� ���� Ÿ������ ����
		if (iterM->GetState() == type_idle || iterM->GetState() == type_walk)
		{
			//�̵�
			iterM->SetTarget(mPlayer);
			printf("�� ����");
			//������ �������� �Ÿ��� ���� �� ������ �Ÿ����� �����ٸ� Ÿ���� ������ ����) 
		
			Vector2D currPos = iterM->GetPos2D();
			Vector2D targetPos = iterM->GetTarget()->GetPos2D();
			if (Vec2DDistance(currPos, m_pMap->temple_loc)/*���� ���� ������ �Ÿ�*/
				< Vec2DDistance(currPos, targetPos)/*���� Ÿ�� ������ �Ÿ�*/)
			{
				iterM->SetTarget(nullptr); // ���߿� ���� ��ü �����ϸ� ���� ��ü�� �ٲ������
			}

			iterM->MoveTo(targetPos, dt);

			printf("��� %d�� ��ġ x: %f y:%f\n", j, currPos.x, currPos.y);
			if (iterM->GetTarget() == NULL) //������ ��ǥ�϶�
			{

				if (Vec2DDistance(currPos, m_pMap->temple_loc) < 10)
				{

					//i->SetObj_State(type_battle); //�������� ������ȯ
					printf("��������");
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
					printf("��Ʋ����");
				}
			}

		}
		else if (iterM->GetState() == type_battle)
		{
			mRogicTimer.SetAttackTimer(); //���ݽð��� ������ �α� ���� Ÿ�̸� ����
			iterM->SetState(type_attack); //�������� ������ȯ

		}
		else if (iterM->GetState() == type_attack)
		{
			if (mRogicTimer.AttackTimer(iterM->GetProperty().attackspeed))//���ݽð��� �Ǹ� ����
			{
				iterM->SetState(type_battle);
				iterM->Attack(iterM->GetTarget());  //���ݽð��� ������ �α� ���� Ÿ�̸� ����
			}
		}
	}
	j++;
}



	
	


		



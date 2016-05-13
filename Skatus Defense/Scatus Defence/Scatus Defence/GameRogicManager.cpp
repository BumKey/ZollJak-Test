#include "GameRogicManager.h"
#include "sceneMgr.h"
#include "time.h"
#include "ResourceMgr.h"

GameRogicManager::GameRogicManager(SceneMgr * SceneMgr_)
{
	m_SceneMgr = SceneMgr_; // ���Ŵ����� ������Ʈ�� �����ϱ� ���� ������
	wave_level = 0;
	Gamestatement= game_title;
	gamename = "��ī���� ���潺";
	m_pMap = new Map();
	m_pMap->Load("Skatus_Map.txt");
	player_num = 0;
	//map; ���Ŀ� �ʱ�ȭ ����
	//mLastMousePos;
	m_OnMouseDown=false;
	mPlayer = NULL;



};

GameRogicManager::~GameRogicManager()
{

};

void GameRogicManager::GameManaging()
{	
	//printf("��ī���� ���潺!!\n");
	mRogicTimer.SetCurrentTime();// ����ð� ���� �� �� �÷���Ÿ�� �����Լ�
	//printf("\n\n\n\n\n\n\n���� �÷��� Ÿ�� : %d��",mRogicTimer.GetPlayTime());
	
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
	//system("cls");//�ܼ�â �����
}
void GameRogicManager::Gamestart()
{
	GameState = game_waiting_wave;
	printf("������ ���۵Ǿ����ϴ�.");
	mRogicTimer.SetBeforeTime(); //�÷���Ÿ�� ��������
	mRogicTimer.SetWaveTimer(); //���̺꽺��ġ �۵�
	printloc();
	//�÷��̾��
	SetPlayer();
	Setting_Team();
	Setting_teamlist();
	//m_pMap->print();
	//���� ���� ������Ʈ �߰�

	
}
void GameRogicManager::GameEnd()
{
	printf("\n������ ����Ǿ����ϴ�\n\n", wave_level);
	
}
void GameRogicManager::GameTitle()
{
	printf("\n���� ������ ���� �ƹ� Ű�� �����ÿ�\n");
	
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
	mRogicTimer.SetWaveTimer(); //WaveTimer�ʱ�ȭ
	//�� ������ ��� ���� ����
	
	/*
	std::list<GameObject*>::iterator i;
	for (i = m_SceneMgr->mObjects.begin(); i != m_SceneMgr->mObjects.end();i++)
	{
		
		if ((*i)->Get_Object_type() == type_goblin) {
			i=m_SceneMgr->mObjects.erase(*i);// �ݺ��ڸ� ��ȯ���� ������, ���� ��ҷ� �����ϴ� �ݺ��ڸ� ����� �� ���� �����߻�
			
		}

	}*/

	for (std::list<GameObject*>::iterator i = m_SceneMgr->mObjects.begin(); i != m_SceneMgr->mObjects.end();)
	{
		if ((*i)->Get_Object_type() == type_goblin)
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
		if ((*i)->Get_Object_type() == type_goblin)
		{
			i = m_Enemies_list.erase(i);


		}
		else
		{
			++i;
		}

	}
	
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
	XMFLOAT4X4 rockWorld[2];
	XMMATRIX modelScale = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	XMMATRIX modelRot = XMMatrixRotationY(0.0f);
	XMMATRIX modelOffset = XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	modelScale = XMMatrixScaling(0.8f, 0.8f, 0.8f);
	modelOffset = XMMatrixTranslation(-1.0f, 1.4f, -7.0f);
	for (int i = 0; i < 2; i++)
	{
		XMStoreFloat4x4(&rockWorld[i], modelScale*modelRot*modelOffset);		
		m_SceneMgr->AddObject(ResourceMgr::RockModel, rockWorld[i], Model_Effect::Base, type_goblin, m_pMap->enemy_loc);
	
		auto it = m_SceneMgr->mObjects.end();
		it--;
		add_Enemies(*it);
		printf("��� %d�� ��ġ x: %f y:%f\n", i, (*it)->Pos().x, (*it)->Pos().y);
	}

	// ���ο� ���� ����

	//���з� ����
	Setting_Team();
	//wavingstart�� ���º���
	

}

void GameRogicManager::Waving()
{
	
	
	//next_wave ī����
	bool timer= mRogicTimer.WaveTimer();
	AIManager();
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
		//�÷��̾� ���� ���ݻ��·� ��ȯ/ �� ������ ����� �浹�˻� ����
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
	for (auto i : m_SceneMgr->mObjects)
	{
		if ( i->Get_Object_type() !=type_object) {
			i->printlocation();
		}
	}


}

void GameRogicManager::SetPlayer()
{


		for (auto i : m_SceneMgr->mObjects)
		{
			if (i->Get_Object_type() == type_p_warrior) {
				mPlayer = (Warrior*)i;
			}
		}

}

void GameRogicManager::Setting_Team()
{
	//��� ����, �Ʊ� ������ ���� �ʱ�ȭ �������
	//�ߺ��˻縦 �ؾ���
	// type_�� �ѵ� �����ؼ� Get_Object_type>10 �� <10 �Ʊ��̷� ������ �ٲ��� ���߿�

	for (auto i : m_SceneMgr->mObjects)
	{
		if (i->Get_Object_type() == type_p_warrior) {
			i->Oppenents = m_Enemies_list;
		}
		else if (i->Get_Object_type() == type_goblin)
		{
			i->Oppenents = m_Our_list;
			
		}
	
	}
}
void GameRogicManager::Setting_teamlist()
{
	m_Our_list.clear();
	m_Enemies_list.clear();
	for (auto i : m_SceneMgr->mObjects)
	{
		if (i->Get_Object_type() == type_p_warrior) {
			m_Our_list.push_back(i);

		}
		else if (i->Get_Object_type() == type_goblin)
		{
			m_Enemies_list.push_back(i);

		}
	}
}
void GameRogicManager::add_Our(GameObject* a)
{
	m_Our_list.push_back(a);
}

void GameRogicManager::add_Enemies(GameObject* a)
{
	m_Enemies_list.push_back(a);
}
void GameRogicManager::MoveAI()
{


}
void GameRogicManager::AIManager()
{
	int j = 0;
	j = 0;
	// ������ �ൿ����
	for (auto i : m_Enemies_list/*m_Enemies_list*/)
	{

		if (i->Get_Object_type() == type_goblin) {//Ÿ�ٺ����� ���߿� 0.5�ʸ��� �ѹ���
												  //���� ���尡��� ���� Ÿ������ ����
			if (i->Get_States() == type_idle || i->Get_States() == type_walk)
			{//�̵�
				i->SettingTarget(i->Oppenents);
				printf("�� ����");
				//������ �������� �Ÿ��� ���� �� ������ �Ÿ����� �����ٸ� Ÿ���� ������ ����) 
				if (i->current_target_obj != NULL)
				{
					if (Vec2DDistance(i->Pos(), m_pMap->temple_loc)/*���� ���� ������ �Ÿ�*/
						< Vec2DDistance(i->Pos(), i->current_target_obj->Pos())/*���� Ÿ�� ������ �Ÿ�*/)
					{
						i->current_target_obj = NULL; // ���߿� ���� ��ü �����ϸ� ���� ��ü�� �ٲ������

					}
					i->SetHeading((i->current_target_obj->Pos()) - (i->Pos()));
					i->Move2D(i->Heading(), 0.03);
				}
				printf("��� %d�� ��ġ x: %f y:%f\n", j, i->Pos().x, i->Pos().y);
				if (i->current_target_obj == NULL) //������ ��ǥ�϶�
				{

					if (Vec2DDistance(i->Pos(), m_pMap->temple_loc) < 10)
					{

						//i->SetObj_State(type_battle); //�������� ������ȯ
						printf("��������");
					}
					else
					{
						i->SetHeading((m_pMap->temple_loc) - (i->Pos()));
						i->Move2D(i->Heading(), 0.03);
					}
				}
				else
				{
					if (Vec2DDistance(i->Pos(), i->current_target_obj->Pos()) < 10)
					{
						i->SetObj_State(type_battle);
						printf("��Ʋ����");
					}
				}

			}
			else if (i->Get_States() == type_battle)
			{
				mRogicTimer.SetAttackTimer(); //���ݽð��� ������ �α� ���� Ÿ�̸� ����
				i->SetObj_State(type_attack); //�������� ������ȯ

			}
			else if (i->Get_States() == type_attack)
			{
				if (mRogicTimer.AttackTimer(i->Get_Properties()->attackspeed))//���ݽð��� �Ǹ� ����
				{
					i->SetObj_State(type_battle);
					i->Attack(i->current_target_obj);  //���ݽð��� ������ �α� ���� Ÿ�̸� ����

				}
			}

		}
		j++;
	}


	
	for (std::list<GameObject*>::iterator i = m_Enemies_list.begin(); i != m_Enemies_list.end();)
	{
		if ((*i)->Get_States() == type_die)
		{
			i = m_Enemies_list.erase(i);
			//���� �׿����� our_lsit�� Oppnents �ʱ�ȭ
	
		}
		else
		{
			++i;
		}

	}

	for (std::list<GameObject*>::iterator i = m_Our_list.begin(); i != m_Our_list.end();)
	{
		if ((*i)->Get_States() == type_die)
		{
			i = m_Our_list.erase(i);// ���� 3�� ������ �׾������
			//�Ʊ��� �׿����� Enemies_lsit�� Oppnents �ʱ�ȭ


		}
		else
		{

			++i;
		}

	}

	
	for (std::list<GameObject*>::iterator i = m_SceneMgr->mObjects.begin(); i != m_SceneMgr->mObjects.end();)
	{
		if ((*i)->Get_States() == type_die)
		{
			i = m_SceneMgr->mObjects.erase(i);

			Setting_Team();

		}
		else
		{
			++i;
		}

	}
}

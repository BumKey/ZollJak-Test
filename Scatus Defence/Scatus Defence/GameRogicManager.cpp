#include "GameRogicManager.h"
#include "sceneMgr.h"
#include "time.h"
#include "ResourceMgr.h"

GameRogicManager::GameRogicManager() : mWaveLevel(1), mGameState(Gamestate_type::game_title),
gamename("��ī���� ���潺"), player_num(0), OnMouseDown(false), mPlayer(nullptr)
{
	m_pMap = new Map();
	m_pMap->Load("Skatus_Map.txt");
	//map; ���Ŀ� �ʱ�ȭ ����
	//mLastMousePos;

	// 1���� ���̺� ���� ����
	mPerWaveMonsterNum[1][ObjectType::Goblin] = 20;
	mPerWaveMonsterNum[1][ObjectType::Cyclop] = 2;

	// ���� �˾Ƽ� �߰�..
};

GameRogicManager::~GameRogicManager()
{

};


void GameRogicManager::Gamestart()
{
	//GameState = game_waiting_wave;
	GameState = game_waving;
	printf("������ ���۵Ǿ����ϴ�.");
	mRogicTimer.SetBeforeTime(); //�÷���Ÿ�� ��������
	mRogicTimer.SetWaveTimer(); //���̺꽺��ġ �۵�

	// ���ο� ���� ����
	Add_Monster(1);

	Printloc();

	//m_pMap->print();
	//���� ���� ������Ʈ �߰�

}
void GameRogicManager::GameEnd()
{
	mObjectMgr.ReleaseAll();
	printf("\n������ ����Ǿ����ϴ�\n\n", mWaveLevel);

}
void GameRogicManager::GameTitle()
{
	printf("\n���� ������ ���� �ƹ� Ű�� �����ÿ�\n");

}

void GameRogicManager::Init(ID3D11Device * device)
{
	mResourceMgr.Init(device);
	mObjectMgr.Init(&mResourceMgr);
	mCollisionMgr.Init(&mObjectMgr);

	mObjectMgr.Update();

	//�÷��̾��
	mPlayer = mObjectMgr.GetPlayer();
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
	//mRogicTimer.SetWaveTimer(); //WaveTimer�ʱ�ȭ
								//�� ������ ��� ���� ����
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
	//waiting_wave�� ���º���
}
void GameRogicManager::StartWave()
{
	mRogicTimer.SetWaveTimer(); //WaveTimer�ʱ�ȭ
	GameState = game_waving;
	mWaveLevel++;
	printf("\n\n\n%d ���� ���̺갡 ���۵Ǿ����ϴ�\n", mWaveLevel);
	printf("\n ���͸� �������Դϴ�\n", mWaveLevel);
	//m_pMap->print();

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
	// ������ �ൿ����	
	//Ÿ�ٺ����� ���߿� 0.5�ʸ��� �ѹ���
	for (auto iterM : mObjectMgr.GetMonsters())
	{
		// ���� Ÿ���� ������ �÷��̾�� ����.
		// ���� Ÿ���� �����̳� �ٸ� �÷��̾�� �ٲٴ� ��� �߰��� ��.
		iterM->SetTarget(mPlayer);
		
		// �Ÿ��� ������ ����
		if (mCollisionMgr.DetectWithPlayer(iterM)) 
			iterM->SetAI_State(AI_State::AttackToTarget);
		else 
			iterM->SetAI_State(AI_State::MovingToTarget);
	}
}
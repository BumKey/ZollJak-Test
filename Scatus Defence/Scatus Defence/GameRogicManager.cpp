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
	mPerWaveMonsterNum[1][ObjectType::Goblin] = 15;
	mPerWaveMonsterNum[1][ObjectType::Cyclop] = 2;

	mPerWaveMonsterNum[2][ObjectType::Goblin] = 20;
	mPerWaveMonsterNum[2][ObjectType::Cyclop] = 5;

	mPerWaveMonsterNum[3][ObjectType::Goblin] = 40;
	mPerWaveMonsterNum[3][ObjectType::Cyclop] = 10;


	// ���� �˾Ƽ� �߰�..
};

GameRogicManager::~GameRogicManager()
{

};


void GameRogicManager::Gamestart()
{
	//GameState = game_waiting_wave;
//	GameState = game_waving;
	

	// ���ο� ���� ����
	Add_Monster(1);

	//Printloc();

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

	
	//system("cls");//�ܼ�â �����

	mObjectMgr.Update(dt);
	mCollisionMgr.Update(dt);

}





void GameRogicManager::Waving(float dt)
{
	AIManager(dt);

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

GameRogicManager* GameRogicManager::Instance()
{
	static GameRogicManager instance;

	return &instance;
}
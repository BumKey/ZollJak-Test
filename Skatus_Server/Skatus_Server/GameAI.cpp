//#include "GameAI.h"
//
//GameAI::GameAI() : mWaveLevel(1), mGameState(Gamestate_type::game_title),
//mPlayerNum(0), OnMouseDown(false)
//{
//	// 1���� ���̺� ���� ����
//	mPerWaveMonsterNum[1][ObjectType::Goblin] = 15;
//	mPerWaveMonsterNum[1][ObjectType::Cyclop] = 2;
//
//	mPerWaveMonsterNum[2][ObjectType::Goblin] = 20;
//	mPerWaveMonsterNum[2][ObjectType::Cyclop] = 5;
//
//	mPerWaveMonsterNum[3][ObjectType::Goblin] = 40;
//	mPerWaveMonsterNum[3][ObjectType::Cyclop] = 10;
//
//};
//
//GameAI::~GameAI()
//{
//
//};
//
//
//void GameAI::Gamestart()
//{
//	//GameState = game_waiting_wave;
////	GameState = game_waving;
//	
//
//	// ���ο� ���� ����
//	Add_Monster(1);
//
//	//Printloc();
//
//	//m_pMap->print();
//	//���� ���� ������Ʈ �߰�
//
//}
//void GameAI::GameEnd()
//{
//	Object_Mgr->ReleaseAll();
//	printf("\n������ ����Ǿ����ϴ�\n\n", mWaveLevel);
//
//}
//void GameAI::GameTitle()
//{
//	printf("\n���� ������ ���� �ƹ� Ű�� �����ÿ�\n");
//
//}
//
////void GameAI::Init(ID3D11Device * device)
////{
////	mResourceMgr.Init(device);
////	Object_Mgr->Init(&mResourceMgr);
////	mCollisionMgr.Init(&mObjectMgr);
////
////	Object_Mgr->Update();
////
////	//�÷��̾��
////	player = Object_Mgr->GetPlayer();
////}
//
//void GameAI::Update(float dt)
//{
//
//}
//
//void GameAI::Waving(float dt)
//{
//	AIManager(dt);
//}
//
//void GameAI::Printloc()
//{
//	printf("\n");
//	Player::GetInstance()->PrintLocation();
//	for (auto i : Object_Mgr->GetMonsters())
//	{
//		i->PrintLocation();
//	}
//}
//
//void GameAI::Add_Monster(UINT waveLevel)
//{
//	assert(waveLevel > 0);
//	auto player = Player::GetInstance();
//
//	InstanceDesc info;
//
//	UINT goblinNum = mPerWaveMonsterNum[waveLevel][ObjectType::Goblin];
//	for (UINT i = 0; i < goblinNum; ++i)
//	{
//		info.Pos = XMFLOAT3(player->GetPos().x + 50.0f + rand() % 150,
//			-0.1f, player->GetPos().z + 50.0f + rand() % 150);
//		info.Scale = 0.1f + MathHelper::RandF() / 5.0f;
//
//		Goblin::Type type;
//		if (i % 2) {
//			type = Goblin::Type::Blue;
//			info.Scale = 0.4f;
//		}
//		else if (i % 3)
//		{
//			type = Goblin::Type::Red;
//			info.Scale = 0.3f;
//		}
//		else if (i % 5)
//		{
//			type = Goblin::Type::Red;
//			info.Scale = 0.7f;
//		}
//		else {
//			type = Goblin::Type::Blue;
//			info.Scale = 0.6f;
//		}
//		Object_Mgr->AddMonster(new Goblin(Resource_Mgr->GetSkinnedMesh(ObjectType::Goblin), info, type));
//	}
//
//	UINT cyclopNum = mPerWaveMonsterNum[waveLevel][ObjectType::Cyclop];
//	for (UINT i = 0; i < cyclopNum; ++i) {
//		info.Pos = XMFLOAT3(player->GetPos().x + 50.0f + rand() % 150,
//			-0.1f, player->GetPos().z + 50.0f + rand() % 150);
//		info.Scale = 6.0f + MathHelper::RandF();
//		info.Rot.x = MathHelper::Pi;
//
//		Object_Mgr->AddMonster(new Cyclop(Resource_Mgr->GetSkinnedMesh(ObjectType::Cyclop), info));
//	}
//}
//void GameAI::MoveAI()
//{
//
//}
//void GameAI::AIManager(float dt)
//{
//	// ������ �ൿ����	
//	//Ÿ�ٺ����� ���߿� 0.5�ʸ��� �ѹ���
//	auto player = Player::GetInstance();
//	for (auto iterM : Object_Mgr->GetMonsters())
//	{
//		// ���� Ÿ���� ������ �÷��̾�� ����.
//		// ���� Ÿ���� �����̳� �ٸ� �÷��̾�� �ٲٴ� ��� �߰��� ��.
//		iterM->SetTarget(player);
//		
//		// �Ÿ��� ������ ����
//		if (Collision_Mgr->DetectWithPlayer(iterM)) 
//			iterM->SetAI_State(AI_State::AttackToTarget);
//		else 
//			iterM->SetAI_State(AI_State::MovingToTarget);
//	}
//}
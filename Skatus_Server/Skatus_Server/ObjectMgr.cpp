#include "ObjectMgr.h"

// 임의의 수치임
ObjectMgr::ObjectMgr() : mStage(1), mTotalObjectNum(0)
{
	mMaxMonsters = mStage * 200;
	mMaxStructures = mStage * 5;

	CreateMap();
}

ObjectMgr::~ObjectMgr()
{
}

void ObjectMgr::AddObject(ObjectType::Types oType)
{
	ForClientInfo info;
	info.ObjectType = oType;

	switch (oType)
	{
	case ObjectType::Monster:
	case ObjectType::Goblin:
	case ObjectType::Cyclop:
		info.Pos = XMFLOAT3(300.0f - rand() % 200,
			0.0f, -180.0f + rand() % 200);
		info.Scale = 0.1f + ((float)(rand()) / (float)RAND_MAX) / 5.0f;
		mMonsters.push_back(info);
		break;

	case ObjectType::Obstacle:
	case ObjectType::Tree:
		info.Pos = XMFLOAT3(100.0f + rand() % 200,
			-0.1f, -300.0f + rand() % 200);
		info.Scale = ((float)(rand()) / (float)RAND_MAX)*2.0f + 0.5f;
		info.Rot.y = ((float)(rand()) / (float)RAND_MAX)*PI* 2;
		mObstacles.push_back(info);
		break;
	case ObjectType::Base:
		break;
	case ObjectType::Stairs:
		break;
	case ObjectType::Pillar1:
		break;
	case ObjectType::Pillar2:
		break;
	case ObjectType::Pillar3:
		break;
	case ObjectType::Pillar4:
		break;
	case ObjectType::Rock:
		break;
	case ObjectType::Temple:
		info.Pos = XMFLOAT3(250.0f, 0.05f, -370.0f);
		info.Rot.y = 0.0f;
		info.Scale = 0.3f;
		mObstacles.push_back(info);
		break;

	default:
		assert(false, "incorrect type at AddObject");
		break;
	}

	++mTotalObjectNum;
}

void ObjectMgr::AddPlayer(ObjectType::Types oType, DWORD client_id)
{
	ForClientInfo info;
	info.ObjectType = oType;

	switch (oType)
	{
	case ObjectType::Player:
	case ObjectType::Warrior:
	case ObjectType::Archer:
	case ObjectType::Builder:
		info.Pos = XMFLOAT3(rand() % 20 + 200.0f, 0.0f, rand() % 20 - 280.0f);
		info.Rot.y = 0.0f;
		info.Scale = 0.05f;
		mPlayers[client_id] = info;
		break;

	default:
		assert(false, "incorrect type at AddObject");
		break;
	}
}

const std::vector<ForClientInfo>& ObjectMgr::GetAllObjects()
{
	mAllObjects.clear();
	mAllObjects.reserve(mTotalObjectNum);

	for (auto i : mObstacles)
		mAllObjects.push_back(i);

	for (auto i : mStructures)
		mAllObjects.push_back(i);

	for (auto i : mMonsters)
		mAllObjects.push_back(i);

	for (auto i : mPlayers)
		mAllObjects.push_back(i);

	return mAllObjects;
}

void ObjectMgr::ReleaseAllMonsters()
{
	mMonsters.clear();
}

void ObjectMgr::CreateMap()
{
	AddObject(ObjectType::Temple);

	for (UINT i = 0; i < 30; ++i)
		AddObject(ObjectType::Tree);

	for (UINT i = 0; i < 30; ++i)
		AddObject(ObjectType::Rock);
}



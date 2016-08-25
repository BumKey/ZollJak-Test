#include "ObjectMgr.h"

// 임의의 수치임
ObjectMgr::ObjectMgr() : mStage(1), mTotalObjectNum(0), mMonsterGeneratedNum(0),
mCurrPlayerNum(0), mTemplePos(XMFLOAT3(230.0f, -0.01f, -370.0f))
{
	mMaxMonsters = mStage * 200;
	mMaxStructures = mStage * 5;

	for (int i = 0; i < MAX_USER; ++i) {
		mConnected[i] = false;
		mPlayerYpos[i] = 0.0f;
	}
	CreateMap();
}

ObjectMgr::~ObjectMgr()
{
}

void ObjectMgr::AddObject(ObjectType::Types oType)
{
	BO_InitDesc BasicInfo;
	SO_InitDesc SkinnedInfo;
	BasicInfo.ObjectType = oType;
	SkinnedInfo.ObjectType = oType;

	switch (oType)
	{
	case ObjectType::Monster:
	case ObjectType::Goblin:
		SkinnedInfo.Pos = XMFLOAT3(300.0f - rand() % 200, -0.1f, -180.0f + rand() % 200);
		SkinnedInfo.Rot = XMFLOAT3(0.0f, MathHelper::RandF(0.0f, MathHelper::Pi), 0.0f);
		SkinnedInfo.Scale = 0.2f + MathHelper::RandF(0.2f, 0.5f);
		mMonsters[mMonsterGeneratedNum++] = SkinnedInfo;
		break;
	case ObjectType::Cyclop:
		SkinnedInfo.Pos = XMFLOAT3(300.0f - rand() % 200, -0.1f, -180.0f + rand() % 200);
		SkinnedInfo.Rot = XMFLOAT3(MathHelper::Pi, MathHelper::RandF(0.0f, MathHelper::Pi), 0.0f);
		SkinnedInfo.Scale = 3.0f + MathHelper::RandF(2.0f, 5.0f);
		mMonsters[mMonsterGeneratedNum++] = SkinnedInfo;
		break;

	case ObjectType::Obstacle:
	case ObjectType::Tree:
		BasicInfo.Scale = ((float)(rand()) / (float)RAND_MAX)*2.0f +  0.5f;
		BasicInfo.Rot.y = MathHelper::RandF(0.0f, MathHelper::Pi * 2);
		BasicInfo.Pos = XMFLOAT3(100.0f + rand() % 200, -0.1f*BasicInfo.Scale, -300.0f + rand() % 200);
		mObstacles.push_back(BasicInfo);
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
		BasicInfo.Scale = ((float)(rand()) / (float)RAND_MAX)*2.0f + 0.5f;
		BasicInfo.Rot.y = ((float)(rand()) / (float)RAND_MAX)*PI * 2;
		BasicInfo.Pos = XMFLOAT3(100.0f + rand() % 200, -0.5f*BasicInfo.Scale, -300.0f + rand() % 200);
		mObstacles.push_back(BasicInfo);
		break;
	case ObjectType::Temple:
		BasicInfo.Pos = mTemplePos;
		BasicInfo.Rot.y = 0.0f;
		BasicInfo.Scale = 0.3f;
		mObstacles.push_back(BasicInfo);
		break;

	default:
		assert(false, "incorrect type at AddObject");
		break;
	}

	++mTotalObjectNum;
}

void ObjectMgr::AddPlayer(ObjectType::Types oType, DWORD client_id)
{
	assert(oType >= ObjectType::Player && oType <= ObjectType::Builder);

	SO_InitDesc info;
	info.ObjectType = oType;
	info.Pos = XMFLOAT3(rand() % 20 + 200.0f, -0.1f, rand() % 20 - 280.0f);
	info.Rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	info.Scale = 0.05f;
	info.MoveSpeed = 9.0f;
	info.AttackPoint = 50.0f;
	info.AttackSpeed = 2.0f;
	info.Hp = 100;

	mPlayers[client_id] = info;
	mConnected[client_id] = true;
	++mCurrPlayerNum;
	assert(mCurrPlayerNum <= MAX_USER, "OVER THE MAX_USER!!");
}

void ObjectMgr::RemovePlayer(const UINT & id)
{
	--mCurrPlayerNum;
	mConnected[id] = false;
	mPlayers[id].ActionState = ActionState::Die;
}

const std::unordered_map<UINT, SO_InitDesc> ObjectMgr::GetPlayers()
{
	std::unordered_map<UINT, SO_InitDesc> rt;
	SO_InitDesc players[MAX_USER];
	for (UINT i = 0; i < MAX_USER; ++i) {
		if (mConnected[i])
		{
			players[i] = mPlayers[i];

			players[i].Pos.y = -0.1f;
			if (mPlayerYpos[i] != 0.0f) {
				players[i].Pos.y += mPlayerYpos[i];
				mPlayerYpos[i] = 0.0f;
			}
		}
	}

	for (UINT i = 0; i < MAX_USER; ++i)
	{
		rt[i] = players[i];
	}

	//UINT count(mCurrPlayerNum);
	//for (auto m : mMonsters)
	//	allObjects[count++] = m.second;

	return rt;
}

const SO_InitDesc & ObjectMgr::GetPlayer(const UINT & id)
{
	return mPlayers[id];
}

void ObjectMgr::ReleaseAllMonsters()
{
	mMonsters.clear();
}

void ObjectMgr::CreateMap()
{
	AddObject(ObjectType::Temple);

	for (UINT i = 0; i < 20; ++i)
		AddObject(ObjectType::Tree);

	for (UINT i = 0; i < 20; ++i)
		AddObject(ObjectType::Rock);
}



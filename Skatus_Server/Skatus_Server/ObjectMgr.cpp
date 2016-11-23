#include "ObjectMgr.h"

// 임의의 수치임
ObjectMgr::ObjectMgr() : mStage(1), mTotalObjectNum(0), mMonsterGeneratedNum(0),
mCurrPlayerNum(0), mTemplePos(XMFLOAT3(-30.0f, -1.0f, 50.0f))
{
	mMaxMonsters = mStage * 200;
	mMaxStructures = mStage * 5;

	for (int i = 0; i < MAX_USER; ++i) {
		mConnected[i] = false;
	}

	CreateMap();
}

ObjectMgr::~ObjectMgr()
{
}

void ObjectMgr::UpdateMonsters()
{
	for (auto& m : mMonsters)
	{
		int collision(-1);
		for (auto coll : mMonsters)
		{
			if (m.first == coll.first)
				continue;

			if (MathHelper::DistanceVector(m.second.Pos, coll.second.Pos) <= 2.0f)
				collision = coll.first;
		}

		const UINT targetID = SetMonstersTarget(m.second.Pos);
		XMVECTOR targetV;
		XMFLOAT3 targetF;

		if (collision == -1) {
			const XMFLOAT3 targetPos = mPlayers[targetID].Pos;

			targetV = MathHelper::TargetVector2D(targetPos, m.second.Pos);
			XMStoreFloat3(&targetF, targetV);
			targetF = Float3Normalize(targetF);

		}
		else {
			const XMFLOAT3 targetPos = mMonsters[collision].Pos;

			targetV = -MathHelper::TargetVector2D(targetPos, m.second.Pos);
			XMStoreFloat3(&targetF, targetV);
			targetF = Float3Normalize(targetF);
		}

		m.second.Pos = m.second.Pos + targetF*m.second.MoveSpeed;
		m.second.TargetID = targetID;
	}
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
		SkinnedInfo.Pos = XMFLOAT3(rand() % 100, -0.1f, rand() % 100);
		SkinnedInfo.Rot = XMFLOAT3(0.0f, MathHelper::RandF(0.0f, MathHelper::Pi), 0.0f);
		SkinnedInfo.Scale = 0.2f + MathHelper::RandF(0.1f, 0.4f);
		SkinnedInfo.MoveSpeed = 6.5f + MathHelper::RandF(-1.0f, 1.5f);
		SkinnedInfo.AttackPoint = 10.0f;
		SkinnedInfo.AttackSpeed = MathHelper::RandF(0.5f, 1.0f);
		SkinnedInfo.Hp = 50.0f;
		mMonsters[mMonsterGeneratedNum++] = SkinnedInfo;
		break;
	case ObjectType::Cyclop:
		SkinnedInfo.Pos = XMFLOAT3(rand() % 100, -0.1f, rand() % 100);
		SkinnedInfo.Rot = XMFLOAT3(MathHelper::Pi, MathHelper::RandF(0.0f, MathHelper::Pi), 0.0f);
		SkinnedInfo.Scale = 2.5f + MathHelper::RandF(1.5f, 3.0f);
		SkinnedInfo.MoveSpeed = 4.0f + MathHelper::RandF(0.0f, 1.0f);
		SkinnedInfo.AttackPoint = 30.0f;
		SkinnedInfo.AttackSpeed = MathHelper::RandF(0.5f, 1.0f);
		SkinnedInfo.Hp = 120.0f;
		mMonsters[mMonsterGeneratedNum++] = SkinnedInfo;
		break;

	case ObjectType::Obstacle:
	case ObjectType::Tree:
		BasicInfo.Scale = ((float)(rand()) / (float)RAND_MAX)*2.0f +  0.5f;
		BasicInfo.Rot.y = MathHelper::RandF(0.0f, MathHelper::Pi * 2);
		BasicInfo.Pos = XMFLOAT3(100.0f + rand() % 60, -0.1f*BasicInfo.Scale, -50.0f + rand() % 60);
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
		BasicInfo.Rot = XMFLOAT3(0.0f, MathHelper::Pi, 0.0f);
		BasicInfo.Scale = 0.2f;
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
	info.Pos = XMFLOAT3(0.0f , -0.1f, 0.0f);
	info.Rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	info.Scale = 0.05f;
	info.MoveSpeed = 9.0f;
	info.AttackPoint = 20.0f;
	info.AttackSpeed = 2.0f;
	info.Hp = 1000;

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

void ObjectMgr::SetCollsion(const UINT& id, const XMFLOAT3 & pos)
{
	UINT count(0);
	XMFLOAT3 p = pos;
	p.y = -0.1f;
	for (auto o : mObstacles)
	{
		if (MathHelper::DistanceVector(p, o.Pos) < 10.0f)
		{
			if (count >= 10)
				break;

			mCollisionPos[count] = o.Pos;
			++count;
		}
	}

	for (UINT i = count; i < 10; ++i)
		mCollisionPos[i] = XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
}

const UINT ObjectMgr::SetMonstersTarget(const XMFLOAT3& pos)
{
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (mConnected[i] && MathHelper::DistanceVector(pos, mPlayers[i].Pos) <= 10.0f)
			return i;
	}

	if (mCurrPlayerNum > 0) {
		switch (rand() % mCurrPlayerNum)
		{
		case 0:
			if (mConnected[0])
				return 0;
		case 1:
			if (mConnected[1])
				return 1;
		case 2:
			if (mConnected[2])
				return 2;
		}
	}
}

std::unordered_map<UINT, SO_InitDesc> ObjectMgr::GetPlayers()
{
	std::unordered_map<UINT, SO_InitDesc> rt;
	SO_InitDesc players[MAX_USER];
	for (UINT i = 0; i < MAX_USER; ++i) {
		if (mConnected[i])
		{
			players[i] = mPlayers[i];
			players[i].Pos.y = -0.1f;
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

SO_InitDesc & ObjectMgr::GetPlayer(const UINT & id)
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

	for (UINT i = 0; i < 40; ++i)
		AddObject(ObjectType::Tree);

	/*for (UINT i = 0; i < 20; ++i)
		AddObject(ObjectType::Rock);*/
}



#include "ObjectMgr.h"

// 임의의 수치임
ObjectMgr::ObjectMgr() : mCurrPlayerNum(0)
{
}

ObjectMgr::~ObjectMgr()
{
}

void ObjectMgr::AddMainPlayer(SkinnedObject* obj, const UINT& id)
{
	++mCurrPlayerNum;
	mPlayers[id] = obj;
}

void ObjectMgr::AddOtherPlayer(SO_InitDesc desc, const UINT& id)
{
	++mCurrPlayerNum;
	switch (desc.ObjectType)
	{
	case ObjectType::Warrior:
		mPlayers[id] = new Warrior(Resource_Mgr->GetSkinnedMesh(desc.ObjectType), desc);
		break;
	}
}

void ObjectMgr::AddMonster(const ObjectType::Types & type, const SO_InitDesc & desc, const UINT& id)
{
	switch (type)
	{
	case ObjectType::Goblin:
		Goblin::Type goblinType;
		switch (id % 3)
		{
		case 0:
			goblinType = Goblin::Type::Red;
			break;
		case 1:
			goblinType = Goblin::Type::Green;
			break;
		case 2:
			goblinType = Goblin::Type::Blue;
			break;
		}
		mMonsters.push_back(new Goblin(Resource_Mgr->GetSkinnedMesh(type), desc, goblinType));
		break;
	case ObjectType::Cyclop:
		mMonsters.push_back(new Cyclop(Resource_Mgr->GetSkinnedMesh(type), desc));
		break;
	default:
		assert(false, "Wrong parameter: AddMonster, type");
		break;
	}
}

void ObjectMgr::AddObstacle(const ObjectType::Types & type, const BO_InitDesc & desc)
{
	assert(type >= ObjectType::Obstacle);
	if (type == ObjectType::Tree)
		mBasicObjects.push_back(new BasicObject(Resource_Mgr->GetBasicMesh(type), desc, Label::AlphaBasic));
	else
		mBasicObjects.push_back(new BasicObject(Resource_Mgr->GetBasicMesh(type), desc, Label::Basic));
}

void ObjectMgr::RemovePlayer(const UINT & id)
{
	delete mPlayers[id];
	mPlayers[id] = 0;
	Packet_Mgr->Connected[id] = false;
}

void ObjectMgr::UpdatePlayer(const UINT & id, const PlayerInfos & info)
{
	assert(id <= 3);
	mPlayers[id]->SetNextMove(info.Pos);;
	mPlayers[id]->SetRot(info.Rot);

	if (info.ActionState == ActionState::Attack)
		mPlayers[id]->SetAttackState();
}

void ObjectMgr::UpdateMonster(const UINT & id, const MonInfos & info)
{
	mMonsters[id]->SetTarget(mPlayers[info.TargetID]);
	if (info.ActionState == ActionState::Die)
		mMonsters[id]->ChangeActionState(info.ActionState);
}

void ObjectMgr::Update(float dt)
{
	mAllObjects.clear();
	mAllObjects.reserve(mBasicObjects.size() + MAX_USER);

	for (auto i : mBasicObjects)
		mAllObjects.push_back(i);

	for (int i = 0; i < MAX_USER; ++i)
	{
		if (Packet_Mgr->Connected[i])
		{
			mPlayers[i]->Animate(dt);
			if (i != Packet_Mgr->ClientID) {
				mPlayers[i]->MoveToTarget(dt);
				mPlayers[i]->Update(dt);
			}

			mAllObjects.push_back(mPlayers[i]);
		}
	}

	for (auto m : mMonsters)
	{
		m->Animate(dt);
		m->Update(dt);
		for (auto m2 : mMonsters)
		{
			if (MathHelper::DistanceVector(m->GetPos(), m2->GetPos()) <= 3.0f)
				m->MovingCollision(m2->GetPos(), dt);
		}
		mAllObjects.push_back(m);
	}

	for (auto i : mAllObjects)
		i->Update(dt);
}

#include "ObjectMgr.h"

// ������ ��ġ��
ObjectMgr::ObjectMgr() : mCurrPlayerNum(0)
{
}

ObjectMgr::~ObjectMgr()
{
}

void ObjectMgr::AddPlayer(SkinnedObject * player, const UINT& id)
{
	mSkinnedObjects[id] = player;
	++mCurrPlayerNum;
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
		mSkinnedObjects[mCurrPlayerNum + id] = new Goblin(Resource_Mgr->GetSkinnedMesh(type), desc, goblinType);
		break;
	case ObjectType::Cyclop:
		mSkinnedObjects[mCurrPlayerNum + id] = new Cyclop(Resource_Mgr->GetSkinnedMesh(type), desc);
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

void ObjectMgr::Update(const UINT & id, const ObjectInfo & info)
{
	if (id < mCurrPlayerNum) {
		mSkinnedObjects[id]->SetPos(info.Pos);

		if(id != Packet_Mgr->GetClientID())
			mSkinnedObjects[id]->SetRot(info.Rot);
	}
}

void ObjectMgr::Update(float dt)
{
	mAllObjects.clear();
	mAllObjects.reserve(mBasicObjects.size() + mSkinnedObjects.size());

	for (auto i : mBasicObjects)
		mAllObjects.push_back(i);

	for (UINT i = 0; i < mCurrPlayerNum; ++i)
	{
		mSkinnedObjects[i]->Animate(dt);
		mAllObjects.push_back(mSkinnedObjects[i]);
	}

	for (auto i : mAllObjects)
		i->Update(dt);
}

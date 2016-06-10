#include "ObjectMgr.h"

// ������ ��ġ��
ObjectMgr::ObjectMgr() : mStage(1), mTotalObjectNum(0)
{
	mMaxMonsters = mStage * 200;
	mMaxStructures = mStage * 5;
}

ObjectMgr::~ObjectMgr()
{
}

void ObjectMgr::Init(ResourceMgr* resourceMgr)
{
	mResourceMgr = resourceMgr;
}

bool ObjectMgr::AddObstacle(BasicObject * basicObject)
{
	mObstacles.push_back(basicObject);
	mAllObjects.push_back(basicObject);
	++mTotalObjectNum;
	return true;

}

bool ObjectMgr::AddStructure(BasicObject * basicObject)
{
	if (mStructures.size() <= mMaxStructures)
	{
		mStructures.push_back(basicObject);
		mAllObjects.push_back(basicObject);
		++mTotalObjectNum;
		return true;
	}
	else
	{
		//�ִ� ���� �ʰ� �����Ұ� UI���
		return false;
	}

}

bool ObjectMgr::AddProjectile(BasicObject * basicObject)
{
	mProjectiles.push_back(basicObject);
	++mTotalObjectNum;
	return true;
}

bool ObjectMgr::AddMonster(Monster* monster)
{
	if (mMonsters.size() <= mMaxMonsters) {
		mMonsters.push_back(monster);
		mAllObjects.push_back(monster);
		++mTotalObjectNum;
		return true;
	}
	else {

		SafeDelete(monster);
		return false;

	}

}

bool ObjectMgr::AddOurTeam(SkinnedObject* skinnedObject)
{
	if (mOurTeam.size() <= mMaxMonsters) {
		mOurTeam.push_back(skinnedObject);
		mAllObjects.push_back(skinnedObject);
		++mTotalObjectNum;
		return true;
	}
	else {
		
		SafeDelete(skinnedObject);
		return false;

	}

}

void ObjectMgr::Update()
{

	mAllObjects.clear();
	mAllObjects.reserve(mTotalObjectNum);

	mAllObjects.push_back(mPlayer);
	for (auto i : mObstacles)
		mAllObjects.push_back(i);

	for (auto i : mStructures)
		mAllObjects.push_back(i);

	for (auto i : mMonsters) {
		mAllObjects.push_back(i);
	}
}

void ObjectMgr::Update(float dt)
{
	mAllObjects.clear();
	mAllObjects.reserve(mTotalObjectNum);

	mAllObjects.push_back(mPlayer);
	mOurTeam.push_back(mPlayer);
	for (auto i : mObstacles)
		mAllObjects.push_back(i);

	for (auto i : mStructures)
		mAllObjects.push_back(i);

	mPlayer->Animate(dt);
	for (auto i = mMonsters.begin(); i != mMonsters.end();)
	{
		Monster*& monster = *i;
		if (monster->IsDead() && monster->CurrAnimEnd())
		{
			monster->Release(mResourceMgr);
			i = mMonsters.erase(i);
			monster = *i;
		}
		else
			++i;

		mAllObjects.push_back(monster);
		mOppenents.push_back(monster);
		monster->Animate(dt);
	}

	for (auto i : mAllObjects)
		i->Update(dt);
}

void ObjectMgr::ReleaseAll()
{
	mAllObjects.clear();
	mObstacles.clear();
	mStructures.clear();
	mMonsters.clear();
	mOurTeam.clear();
}

void ObjectMgr::ReleaseAllMonsters()
{
	for (auto i : mMonsters)
		i->Release(mResourceMgr);

	mMonsters.clear();
}



#include "ObjectMgr.h"

// 임의의 수치임
ObjectMgr::ObjectMgr() : mStage(1), mTotalObjectNum(0)
{
	mMaxMonsters = mStage * 10;
	mMaxStructures = mStage * 5;
}

ObjectMgr::~ObjectMgr()
{
}

void ObjectMgr::AddObstacle(BasicObject * basicObject)
{
	mObstacles.push_back(basicObject);
	++mTotalObjectNum;
}

void ObjectMgr::AddStructure(BasicObject * basicObject)
{
	if (mStructures.size() <= mMaxStructures)
		mStructures.push_back(basicObject);

	++mTotalObjectNum;
}

void ObjectMgr::AddProjectile(BasicObject * basicObject)
{
	mProjectiles.push_back(basicObject);
	++mTotalObjectNum;
}

void ObjectMgr::AddMonster(SkinnedObject * skinnedObject)
{
	if (mMonsters.size() <= mMaxMonsters)
		mMonsters.push_back(skinnedObject);

	++mTotalObjectNum;
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

	for (auto i : mMonsters) 
		mAllObjects.push_back(i);
}

void ObjectMgr::Update(float dt)
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
		i->Animate(dt);
	}

	mPlayer->PlayerInput(dt);
	mPlayer->Animate(dt);
}

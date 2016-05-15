#include "ObjectMgr.h"

// 임의의 수치임
ObjectMgr::ObjectMgr() : mStage(1), mTotalObjectNum(0)
{
	mMaxMonsters = mStage * 20;
	mMaxStructures = mStage * 5;
}

ObjectMgr::~ObjectMgr()
{
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
	{	mStructures.push_back(basicObject);
		mAllObjects.push_back(basicObject);
		++mTotalObjectNum;
		return true;
	}
	else
	{
		//최대 개수 초과 생성불가 UI출력
		return false;
	}

}

bool ObjectMgr::AddProjectile(BasicObject * basicObject)
{
	mProjectiles.push_back(basicObject);
	//++mTotalObjectNum;
	return true;
}

bool ObjectMgr::AddMonster(GameObject* skinnedObject)
{
	if (mMonsters.size() <= mMaxMonsters) {
		mMonsters.push_back(skinnedObject);
		mAllObjects.push_back(skinnedObject);
		//++mTotalObjectNum;
		return true;
	}
	else {

		SafeDelete(skinnedObject);
		return false;

	}
	
}

bool ObjectMgr::AddOurTeam(GameObject* skinnedObject)
{
	if (mOurTeam.size() <= mMaxMonsters) {
		mOurTeam.push_back(skinnedObject);
		mAllObjects.push_back(skinnedObject);
		//++mTotalObjectNum;
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
	//mAllObjects.reserve(mTotalObjectNum);

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
	//mAllObjects.reserve(mTotalObjectNum); 리스트로 바꿔서 필요없어짐

	mAllObjects.push_back(mPlayer);
	for (auto i : mObstacles)
		mAllObjects.push_back(i);

	for (auto i : mStructures)
		mAllObjects.push_back(i);

	for (auto i : mMonsters) {
		mAllObjects.push_back(i);
		//i->Animate(dt);
	}

	mPlayer->Animate(dt);
	for (auto i : mAllObjects)
		i->Update();
}

void ObjectMgr::ReleaseAll(ResourceMgr& resourceMgr)
{
	mAllObjects.clear();
	mObstacles.clear();
	mStructures.clear();
	mMonsters.clear();
	mOurTeam.clear();
}

void ObjectMgr::ReleaseAllMonsers(ResourceMgr& resourceMgr)
{

	mMonsters.clear();
	for (std::list<GameObject*>::iterator i = mAllObjects.begin(); i != mAllObjects.end();)
	{
		if ((*i)->Get_Object_type() == type_monster)
		{
			i = mAllObjects.erase(i);


		}
		else
		{
			++i;
		}

	}
}

void ObjectMgr::ReleaseAllDeads(ResourceMgr& resourceMgr)
{
	for (std::list<GameObject*>::iterator i = mAllObjects.begin(); i != mAllObjects.end();)
	{
		if ((*i)->Get_States() == type_die)
		{
			i = mAllObjects.erase(i);


		}
		else
		{
			++i;
		}

	}

	for (std::list<GameObject*>::iterator i = mMonsters.begin(); i != mMonsters.end();)
	{
		if ((*i)->Get_States() == type_die)
		{
			i = mMonsters.erase(i);


		}
		else
		{
			++i;
		}

	}

	for (std::list<GameObject*>::iterator i = mOurTeam.begin(); i != mOurTeam.end();)
	{
		if ((*i)->Get_States() == type_die)
		{
			i = mOurTeam.erase(i);


		}
		else
		{
			++i;
		}

	}

}

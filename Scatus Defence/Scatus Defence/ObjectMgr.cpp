#include "ObjectMgr.h"

// 임의의 수치임
ObjectMgr::ObjectMgr() : mStage(1), mTotalObjectNum(0)
{
	mMaxMonsters = mStage * 200;
	mMaxStructures = mStage * 5;
}

ObjectMgr::~ObjectMgr()
{
}

void ObjectMgr::Init()
{
	CreateMap();
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
		//최대 개수 초과 생성불가 UI출력
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
		monster->Release();
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

	mAllObjects.push_back(Player::GetInstance());
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

	mAllObjects.push_back(Player::GetInstance());
	mOurTeam.push_back(Player::GetInstance());
	for (auto i : mObstacles)
		mAllObjects.push_back(i);

	for (auto i : mStructures)
		mAllObjects.push_back(i);

	Player::GetInstance()->Animate(dt);
	for (auto i = mMonsters.begin(); i != mMonsters.end();)
	{
		Monster*& monster = *i;
		if (monster->IsDead() && monster->CurrAnimEnd())
		{
			monster->Release();
			i = mMonsters.erase(i);
		}
		else
		{
			mAllObjects.push_back(monster);
			mOppenents.push_back(monster);
			monster->Animate(dt);

			++i;
		}
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
		i->Release();

	mMonsters.clear();
}

void ObjectMgr::CreateMap()
{
	InstanceDesc info;

	info.Pos = XMFLOAT3(195.0f, 0.05f, -300.0f);
	info.Rot.y = 0.0f;
	info.Scale = 0.3f;

	AddObstacle(new BasicObject(Resource_Mgr->GetBasicMesh(ObjectType::Temple), info, Label::Basic));

	auto player = Player::GetInstance();
	for (UINT i = 0; i < 10; ++i)
	{
		info.Pos = XMFLOAT3(player->GetPos().x + 50.0f - rand() % 100,
			-0.1f, player->GetPos().z + 50.0f - rand() % 100);
		info.Scale = MathHelper::RandF()*2.0f + 0.5f;
		info.Rot.y = MathHelper::RandF()*MathHelper::Pi * 2;

		AddObstacle(new BasicObject(Resource_Mgr->GetBasicMesh(ObjectType::Tree), info, Label::AlphaBasic));
	}

	for (UINT i = 0; i < 20; ++i)
	{
		info.Pos = XMFLOAT3(player->GetPos().x + 50.0f - rand() % 100,
			-0.1f, player->GetPos().z + 50.0f - rand() % 100);
		info.Scale = MathHelper::RandF()*2.0f + 0.5f;
		info.Rot.y = MathHelper::RandF()*MathHelper::Pi * 2.0f;

		AddObstacle(new BasicObject(Resource_Mgr->GetBasicMesh(ObjectType::Rock), info, Label::Basic));
	}
}



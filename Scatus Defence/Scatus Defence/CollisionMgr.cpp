#include "CollisionMgr.h"
#include <iostream>

CollisionMgr::CollisionMgr()
{
}


CollisionMgr::~CollisionMgr()
{
}

void CollisionMgr::Init(ObjectMgr * objectMgr)
{
	mObjectMgr = objectMgr;
}

void CollisionMgr::Update(float dt)
{
	mMonsters = mObjectMgr->GetMonsters();
	mPlayer = mObjectMgr->GetPlayer();

	MovingCollision(dt);

	// 이곳에 AttackCollision을 넣으면 공격판정이 매프레임마다 실행되버린다.
	// AttackCollision();
}

void CollisionMgr::MovingCollision(float dt)
{
	for (auto& iterM : mMonsters)
	{
		if (iterM->HasTarget() && iterM->IsAttack() == false)
		{
			if(LowDetectWithMonsters(iterM))
				iterM->MovingCollision(iterM->GetPos(), dt);
			else
				iterM->SetNoneCollision();
		}
	}
}

void CollisionMgr::AttackCollision()
{
	if (mPlayer->IsAttack())
	{
		std::vector<UINT> detectedMonsters;
		HighDetectWithMonsters(mPlayer, detectedMonsters);
		for (UINT i = 0; i < detectedMonsters.size(); ++i)
		{
			UINT index = detectedMonsters[i];
			mPlayer->Attack(mMonsters[index]);
		}
	}
}

bool CollisionMgr::LowDetectWithMonsters(GameObject* sourceObj)
{
	// 처음 검출된 충돌만 인식하고 나머지는 건너뜀
	// 약식 충돌 -> 군중 애니메이션
	for (UINT i = 0; i < mMonsters.size(); ++i)
	{
		if (sourceObj->GetID() != mMonsters[i]->GetID() &&
			MathHelper::DistanceVector(sourceObj->GetPos(), mMonsters[i]->GetPos()) <= 5.0f)
		{
			std::cout << sourceObj->GetID() << "과" << mMonsters[i]->GetID() << "충돌 검출 중.." << std::endl;
			return true;
		}
		else
			return false;
	}

}

void CollisionMgr::HighDetectWithMonsters(GameObject* sourceObj,  std::vector<UINT>& outIndices)
{
	for (UINT i = 0; i < mMonsters.size(); ++i)
	{
		if (sourceObj->GetID() != mMonsters[i]->GetID() &&
			MathHelper::DistanceVector(sourceObj->GetPos(), mMonsters[i]->GetPos()) <= 5.0f)
		{
			std::cout << sourceObj->GetID() << "과" << mMonsters[i]->GetID() << "충돌 검출 중.." << std::endl;
			outIndices.push_back(i);
		}
	}
}

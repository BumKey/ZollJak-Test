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

	// �̰��� AttackCollision�� ������ ���������� �������Ӹ��� ����ǹ�����.
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
	// ó�� ����� �浹�� �ν��ϰ� �������� �ǳʶ�
	// ��� �浹 -> ���� �ִϸ��̼�
	for (UINT i = 0; i < mMonsters.size(); ++i)
	{
		if (sourceObj->GetID() != mMonsters[i]->GetID() &&
			MathHelper::DistanceVector(sourceObj->GetPos(), mMonsters[i]->GetPos()) <= 5.0f)
		{
			std::cout << sourceObj->GetID() << "��" << mMonsters[i]->GetID() << "�浹 ���� ��.." << std::endl;
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
			std::cout << sourceObj->GetID() << "��" << mMonsters[i]->GetID() << "�浹 ���� ��.." << std::endl;
			outIndices.push_back(i);
		}
	}
}

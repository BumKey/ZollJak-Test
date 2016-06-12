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

	// ���� �÷��̾� ���ݸ� �浹üũ ����
	// ���� ���ݰ����� �����޴������� ��. -> �����ʿ�?
	PlayerAttackCollision();
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

void CollisionMgr::PlayerAttackCollision()
{
	if (mPlayer->IsAttack() && mPlayer->OneHit())
	{
		std::vector<UINT> detectedMonsters;
		HighDetectWithMonsters(mPlayer, detectedMonsters);
		for (UINT i = 0; i < detectedMonsters.size(); ++i)
		{
			UINT index = detectedMonsters[i];
			mPlayer->Attack(mMonsters[index]);
			mMonsters[index]->ChangeActionState(ActionState::Damage);
		}
	}
}

bool CollisionMgr::DetectWithPlayer(GameObject * sourceObj)
{
	XMFLOAT3 sourcePos = sourceObj->GetPos();
	XMFLOAT3 playerPos = mPlayer->GetPos();
	if (MathHelper::DistanceVector(sourcePos, playerPos) <= sourceObj->GetProperty().attackrange)
		return true;
	else
		return false;
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

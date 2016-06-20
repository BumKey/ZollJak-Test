#include "CollisionMgr.h"

CollisionMgr::CollisionMgr()
{
}


CollisionMgr::~CollisionMgr()
{
}

void CollisionMgr::Update(float dt)
{
	MovingCollision(dt);

	// ���� �÷��̾� ���ݸ� �浹üũ ����
	// ���� ���ݰ����� �����޴������� ��. -> �����ʿ�?
	PlayerAttackCollision();
}

void CollisionMgr::MovingCollision(float dt)
{
	for (auto& iterM : Object_Mgr->GetMonsters())
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
	auto player = Player::GetInstance();
	auto monsters = Object_Mgr->GetMonsters();
	if (player->IsAttack() && player->OneHit())
	{
		std::vector<UINT> detectedMonsters;
		HighDetectWithMonsters(player, detectedMonsters);
		for (UINT i = 0; i < detectedMonsters.size(); ++i)
		{
			UINT index = detectedMonsters[i];
			player->Attack(monsters[index]);
			monsters[index]->ChangeActionState(ActionState::Damage);
		}
	}
}

bool CollisionMgr::DetectWithPlayer(GameObject * sourceObj)
{
	auto player = Player::GetInstance();

	XMFLOAT3 sourcePos = sourceObj->GetPos();
	XMFLOAT3 playerPos = player->GetPos();
	if (MathHelper::DistanceVector(sourcePos, playerPos) <= sourceObj->GetProperty().attackrange)
		return true;
	else
		return false;
}

bool CollisionMgr::LowDetectWithMonsters(GameObject* sourceObj)
{
	// ó�� ����� �浹�� �ν��ϰ� �������� �ǳʶ�
	// ��� �浹 -> ���� �ִϸ��̼�
	auto monsters = Object_Mgr->GetMonsters();
	for (UINT i = 0; i < monsters.size(); ++i)
	{
		if (sourceObj->GetID() != monsters[i]->GetID() &&
			MathHelper::DistanceVector(sourceObj->GetPos(), monsters[i]->GetPos()) <= 5.0f)
		{
			std::cout << sourceObj->GetID() << "��" << monsters[i]->GetID() << "�浹 ���� ��.." << std::endl;
			return true;
		}
		else
			return false;
	}

}

void CollisionMgr::HighDetectWithMonsters(GameObject* sourceObj,  std::vector<UINT>& outIndices)
{
	auto monsters = Object_Mgr->GetMonsters();
	for (UINT i = 0; i < monsters.size(); ++i)
	{
		if (sourceObj->GetID() != monsters[i]->GetID() &&
			MathHelper::DistanceVector(sourceObj->GetPos(), monsters[i]->GetPos()) <= 5.0f)
		{
			std::cout << sourceObj->GetID() << "��" << monsters[i]->GetID() << "�浹 ���� ��.." << std::endl;
			outIndices.push_back(i);
		}
	}
}

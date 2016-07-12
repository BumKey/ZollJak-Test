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

	// 현재 플레이어 공격만 충돌체크 감지
	// 몬스터 공격감지는 로직메니져에서 함. -> 개선필요?
	PlayerAttackCollision();
}

void CollisionMgr::MovingCollision(float dt)
{
	auto player = Player::GetInstance();
	auto obstacles = Object_Mgr->GetObstacles();
	auto monsters = Object_Mgr->GetMonsters();

	for (auto iterO : obstacles)
	{
		BasicObject outObj;
		if (DetectWithObstacles(player, outObj))
			player->CollisionMoving(outObj.GetPos(), dt);
	}

	// Monster vs Monster collision
	for (auto& iterM : monsters)
	{
		if (iterM->HasTarget() && iterM->IsAttack() == false)
		{
			std::vector<UINT> detectedMonsterIndices;
			DetectWithMonsters(iterM, detectedMonsterIndices);
			if (detectedMonsterIndices.size() > 0) {
				for(auto i : detectedMonsterIndices)
					iterM->MovingCollision(monsters[i]->GetPos(), dt);
			}
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
		std::vector<UINT> detectedMonsterIndices;
		DetectWithMonsters(player, detectedMonsterIndices);
		for (UINT i = 0; i < detectedMonsterIndices.size(); ++i)
		{
			UINT index = detectedMonsterIndices[i];
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

void CollisionMgr::DetectWithMonsters(GameObject* sourceObj,  std::vector<UINT>& outIndices)
{
	auto monsters = Object_Mgr->GetMonsters();
	for (UINT i = 0; i < monsters.size(); ++i)
	{
		if (sourceObj->GetID() != monsters[i]->GetID() &&
			MathHelper::DistanceVector(sourceObj->GetPos(), monsters[i]->GetPos()) <= 5.0f)
		{
			std::cout << sourceObj->GetID() << "과" << monsters[i]->GetID() << "충돌 검출 중.." << std::endl;
			outIndices.push_back(i);
		}
	}
}

bool CollisionMgr::DetectWithObstacles(GameObject * sourceObj, BasicObject& outObj)
{
	auto obstacles = Object_Mgr->GetObstacles();
	for (UINT i = 0; i < obstacles.size(); ++i)
	{
		if (Detect(sourceObj->GetPos(), obstacles[i]->GetPos()))
		{
			std::cout << sourceObj->GetID() << "과" << obstacles[i]->GetID() << "지물 충돌 중.." << std::endl;
			outObj = *obstacles[i];
			return true;
		}
	}

	return false;
}

bool CollisionMgr::Detect(const XMFLOAT3& sPos, const XMFLOAT3 & dPos)
{
	if (MathHelper::DistanceVector(sPos, dPos) <= 5.0f)
		return true;
	else
		return false;
}

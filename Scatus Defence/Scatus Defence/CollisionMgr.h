#pragma once
#include "Singletone.h"
#include "ObjectMgr.h"
#include "Utilities.h"
#include "2DMap.h"
#include <iostream>

#define Collision_Mgr CollisionMgr::GetInstance()

class CollisionMgr : public Singletone<CollisionMgr>
{
private:
	CollisionMgr();
	~CollisionMgr();

	friend class Singletone<CollisionMgr>;
public:
	void Update(float dt);
	bool DetectWithPlayer(GameObject* sourceObj);

private:
	void MovingCollision(float dt);
	void PlayerAttackCollision();

	void DetectWithMonsters(GameObject* sourceObj, std::vector<UINT>& outIndices);
	bool DetectWithObstacles(GameObject* sourceObj, BasicObject& outObj);		// 성능을 위해 지물과는 보통 하나와 충돌한다고 가정한다.

	// OOBB로 개선필요
	bool Detect(const XMFLOAT3& sPos, const XMFLOAT3& dPos);
};


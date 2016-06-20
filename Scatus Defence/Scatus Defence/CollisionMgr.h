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

	bool LowDetectWithMonsters(GameObject* sourceObj);
	void HighDetectWithMonsters(GameObject* sourceObj, std::vector<UINT>& outIndices);
};


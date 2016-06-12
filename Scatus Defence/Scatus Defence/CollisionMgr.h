#pragma once
#include "ObjectMgr.h"
#include "Utilities.h"
#include "2DMap.h"

class CollisionMgr
{
public:
	CollisionMgr();
	~CollisionMgr();

public:
	void Init(ObjectMgr* objectMgr);
	void Update(float dt);

	bool DetectWithPlayer(GameObject* sourceObj);

private:
	void MovingCollision(float dt);
	void PlayerAttackCollision();

	bool LowDetectWithMonsters(GameObject* sourceObj);
	void HighDetectWithMonsters(GameObject* sourceObj, std::vector<UINT>& outIndices);

private:
	ObjectMgr* mObjectMgr;
	std::vector<Monster*> mMonsters;
	Player* mPlayer;
};


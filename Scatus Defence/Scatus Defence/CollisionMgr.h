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

	/// <summary>
	/// Notice : 현재 이 메서드는 플레이어 공격만 판정.
	/// 몬스터의 공격 충돌판정은 몬스터 행동제어라는 측면에서 RogicMgr에 있다. 
	/// 물론 충돌판정의 변화가 생긴다면 두 곳 다 수정해야한다는 번거로움은 있다.
	void AttackCollision();

private:
	void MovingCollision(float dt);

	bool LowDetectWithMonsters(GameObject* sourceObj);
	void HighDetectWithMonsters(GameObject* sourceObj, std::vector<UINT>& outIndices);

private:
	ObjectMgr* mObjectMgr;
	std::vector<Monster*> mMonsters;
	Player* mPlayer;
};


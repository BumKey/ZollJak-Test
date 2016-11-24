#pragma once
#include "protocol.h"
#include "Singletone.h"
#include "Camera.h"
#include "Terrain.h"
#include "GameObject.h"
#include "xnacollision.h"

#define Collision_Mgr CollisionMgr::GetInstance()

class CollisionMgr : public Singletone<CollisionMgr>
{
public:
	CollisionMgr();
	~CollisionMgr();

	friend class Singletone<CollisionMgr>;
public:
	bool FrustumAABBCulling(GameObject* obj);
	bool MonsterFrustumCulling(GameObject* obj);
	bool CollisionCheckOOBB(GameObject* obj1, GameObject*obj2);

	void CollisionMoving(XMFLOAT3& outPos, const XMFLOAT3& dPos, FLOAT moveSpeed, FLOAT dt);
public:
	XMFLOAT3 PlayerCollPos[COLL_OBJ_NUM];
	XMFLOAT3 MonCollPos[MAX_MONSTER][COLL_OBJ_NUM];
};


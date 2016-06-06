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
	void Collision(ObjectMgr& objectMgr, float dt);

private:
	void MovingCollision(ObjectMgr & objectMgr, float dt);
};


#include "CollisionMgr.h"
#include <iostream>

CollisionMgr::CollisionMgr()
{
}


CollisionMgr::~CollisionMgr()
{
}

void CollisionMgr::Collision(ObjectMgr & objectMgr, float dt)
{
	MovingCollision(objectMgr, dt);
}

void CollisionMgr::MovingCollision(ObjectMgr & objectMgr, float dt)
{
	for (auto& iterM1 : objectMgr.GetMonsters())
	{
		if (iterM1->HasTarget())
		{
			for (const auto& iterM2 : objectMgr.GetMonsters())
			{
				if (MathHelper::DistanceVector(iterM1->GetPos(), iterM2->GetPos()) <= 5.0f &&
					iterM1->GetID() != iterM2->GetID() &&
					iterM1->GetActionState() != ActionState::Attack)
				{
					std::cout << iterM1->GetID() << "과" << iterM2->GetID() << "충돌 검출 중.." << std::endl;
					iterM1->MovingCollision(iterM2->GetPos(), dt);
				}
				else
					iterM1->SetNoneCollision();
			}
		}
	}
}

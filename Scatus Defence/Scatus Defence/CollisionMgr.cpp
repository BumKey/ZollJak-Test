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
				if (Vec2DDistance(iterM1->GetPos2D(), iterM2->GetPos2D()) <= 3.0f && 
					iterM1->GetID() != iterM2->GetID() && 
					iterM1->GetState() != state_type::type_attack)
				{
					std::cout << iterM1->GetID() << "과" << iterM2->GetID() << "충돌 검출 중.." << std::endl;
					iterM1->SetState(state_type::MovingCollision);
					XMFLOAT3 currDir = iterM1->GetTarget()->GetPos() - iterM1->GetPos();
					XMFLOAT3 targetDir = iterM2->GetPos() - iterM1->GetPos();
					iterM1->MoveTo(iterM2->GetPos2D()*-1.0f, dt);
				}
				//else
					//iterM1->SetState(state_type::type_walk);
			}
		}
	}
}

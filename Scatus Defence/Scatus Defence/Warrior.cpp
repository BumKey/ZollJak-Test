#include "Warrior.h"
#include "CollisionMgr.h"

Warrior::Warrior(SkinnedMesh* mesh, const SO_InitDesc& info) : SkinnedObject(mesh, info)
{
	mProperty.name = "Player_Warrior";

	mObjectType = ObjectType::Player;

	mAnimNames[Anims::attack1] = "attack01";
	mAnimNames[Anims::attack2] = "attack02";
	mAnimNames[Anims::hit] = "damage";
	mAnimNames[Anims::run] = "run";
	mAnimNames[Anims::idle] = "idle";
	mAnimNames[Anims::walk] = "walk";

	mProperty.attackspeed = info.AttackSpeed;
	mProperty.movespeed = info.MoveSpeed;
	mProperty.hp_now = info.Hp;
	mProperty.attakpoint = info.AttackPoint;
}


Warrior::~Warrior()
{
}

void Warrior::Update(float dt)
{
	SkinnedObject::Update(dt);
	mFrustumCull = Collision_Mgr->FrustumAABBCulling(this);
}

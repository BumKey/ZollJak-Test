#include "Cyclop.h"

Cyclop::Cyclop(SkinnedMesh* mesh, const SO_InitDesc& info) : Monster(mesh, info)
{
	mProperty.name = "Cyclop";

	mProperty.hp_now = info.Hp;
	mProperty.movespeed = info.MoveSpeed;
	mProperty.attackspeed = info.AttackSpeed;
	mProperty.attackrange = 5.0f;
	mProperty.attakpoint = info.AttackPoint;

	mObjectType = ObjectType::Cyclop;

	mAnimNames[Anims::attack1] = "attack01";
	mAnimNames[Anims::attack2] = "attack02";
	mAnimNames[Anims::hit] = "hit";
	mAnimNames[Anims::dead] = "death";
	mAnimNames[Anims::stunned] = "stunned";
	mAnimNames[Anims::run] = "run";
	mAnimNames[Anims::stunned_hit] = "stunned_hit";
	mAnimNames[Anims::idle] = "idle";
	mAnimNames[Anims::walk] = "walk";

	// mExtentY = mAABB.Extents.x;
}


Cyclop::~Cyclop()
{
}

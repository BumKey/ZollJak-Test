#include "Cyclop.h"

Cyclop::Cyclop(SkinnedMesh* mesh, const InstanceDesc& info) : Monster(mesh, info)
{
	mProperty.name = "Cyclop";

	mProperty.hp_now = 500;
	mProperty.movespeed = MathHelper::RandF(3.7f, 4.7f);
	mProperty.attackspeed = MathHelper::RandF(0.4f, 0.7f);
	mProperty.attackrange = 5.0f;
	mProperty.attakpoint = 10.0f;

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

	mExtentY = mAABB.Extents.x;
}


Cyclop::~Cyclop()
{
}

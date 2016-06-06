#include "Cyclop.h"

Cyclop::Cyclop(SkinnedMesh* mesh, const InstanceDesc& info) : Monster(mesh, info)
{
	mTimePos = 0.0f;
	mProperty.movespeed = MathHelper::RandF(0.07f, 0.1f);
	mProperty.attackspeed = MathHelper::RandF(0.4f, 0.7f);
	mProperty.attakpoint = 10.0f;

	mObjectType = Object_type::cyclop;

	mAnimNames[Anims::attack1] = "attack01";
	mAnimNames[Anims::attack2] = "attack02";
	mAnimNames[Anims::hit] = "hit";
	mAnimNames[Anims::dead] = "death";
	mAnimNames[Anims::stunned] = "stunned";
	mAnimNames[Anims::run] = "run";
	mAnimNames[Anims::stunned_hit] = "stunned_hit";
	mAnimNames[Anims::idle] = "idle";
	mAnimNames[Anims::walk] = "walk";

	SetState(type_idle);
}


Cyclop::~Cyclop()
{
}

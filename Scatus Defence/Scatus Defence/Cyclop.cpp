#include "Cyclop.h"

Cyclop::Cyclop(SkinnedMesh* mesh, const InstanceDesc& info) : Monster(mesh, info)
{
	mTimePos = 0.0f;
	mProperty.movespeed = MathHelper::RandF(0.07f, 0.1f);
	mProperty.attackspeed = MathHelper::RandF(0.4f, 0.7f);
	mProperty.attakpoint = 10.0f;

	mObjectType = Object_type::cyclop;

	mAnimNames[CyclopAnim::attack1] = "attack01";
	mAnimNames[CyclopAnim::attack2] = "attack02";
	mAnimNames[CyclopAnim::hit] = "hit";
	mAnimNames[CyclopAnim::dead] = "death";
	mAnimNames[CyclopAnim::stunned] = "stunned";
	mAnimNames[CyclopAnim::run] = "run";
	mAnimNames[CyclopAnim::stunned_hit] = "stunned_hit";
	mAnimNames[CyclopAnim::idle] = "idle";
	mAnimNames[CyclopAnim::walk] = "walk";

	SetState(type_idle);
}


Cyclop::~Cyclop()
{
}

void Cyclop::Animate(float dt)
{
	SetClip();

	if (mProperty.state == state_type::type_attack)
		mTimePos += dt*mProperty.attackspeed;
	else
		mTimePos += dt*mProperty.movespeed*10.0f;

	mMesh->SkinnedData.GetFinalTransforms(mCurrClipName, mTimePos, mFinalTransforms);

	// Loop animation
	if (mTimePos > mMesh->SkinnedData.GetClipEndTime(mCurrClipName))
	{
		mTimePos = 0.0f;
		if (mCurrClipName == mAnimNames[CyclopAnim::attack1] ||
			mCurrClipName == mAnimNames[CyclopAnim::attack2])		// attack01은 루프 안쓰는 애니메이션
			mProperty.state = state_type::type_idle;
	}
}

void Cyclop::SetClip()
{
	if (mProperty.state == state_type::type_idle)
		mCurrClipName = mAnimNames[CyclopAnim::idle];

	if (mProperty.state == state_type::type_attack &&
		mCurrClipName != mAnimNames[CyclopAnim::attack1] &&
		mCurrClipName != mAnimNames[CyclopAnim::attack2]) {
		if (rand() % 2)
			mCurrClipName = mAnimNames[CyclopAnim::attack1];
		else
			mCurrClipName = mAnimNames[CyclopAnim::attack2];
	}

	//if (mProperty.state == state_type::type_battle)
	//	mCurrClipName = mAnimNames[CyclopAnim::attack2];

	if (mProperty.state == state_type::type_run)
		mCurrClipName = mAnimNames[CyclopAnim::run];

	if (mProperty.state == state_type::type_walk)
		mCurrClipName = mAnimNames[CyclopAnim::walk];

	if (mProperty.state == state_type::type_die)
		mCurrClipName = mAnimNames[CyclopAnim::dead];
}


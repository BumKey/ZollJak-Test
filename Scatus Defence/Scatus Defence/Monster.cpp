#include "Monster.h"


Monster::Monster(SkinnedMesh* mesh, const InstanceDesc& info) : SkinnedObject(mesh, info)
{
}


Monster::~Monster()
{
}

void Monster::SetClip()
{
	if (mProperty.state == state_type::type_idle)
		mCurrClipName = mAnimNames[Anims::idle];

	if (mProperty.state == state_type::type_attack &&
		mCurrClipName != mAnimNames[Anims::attack1] &&
		mCurrClipName != mAnimNames[Anims::attack2]) {
		if (rand() % 2)
			mCurrClipName = mAnimNames[Anims::attack1];
		else
			mCurrClipName = mAnimNames[Anims::attack2];
	}

	if (mProperty.state == state_type::type_run)
		mCurrClipName = mAnimNames[Anims::run];

	if (mProperty.state == state_type::type_walk ||
		mProperty.state == state_type::MovingCollision)
		mCurrClipName = mAnimNames[Anims::walk];

	if (mProperty.state == state_type::type_die)
		mCurrClipName = mAnimNames[Anims::dead];

}

void Monster::Animate(float dt)
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
		if (mCurrClipName == mAnimNames[Anims::attack1] ||
			mCurrClipName == mAnimNames[Anims::attack2])		// attack01은 루프 안쓰는 애니메이션
			mProperty.state = state_type::type_idle;
	}
}

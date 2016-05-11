#include "Player.h"

Player::Player(SkinnedModel* model, const InstanceDesc& info) : SkinnedObject(model, info)
{
	mTimePos = 0.0f;
	mMovingSpeed = 5.5f;
	mCurrClipName = "stand";

	mModel = static_cast<SkinnedModel*>(GameObject::mModel);
}


Player::~Player()
{
}

bool Player::SetClip(std::string clipName)
{
	if (clipName == "attack01")
		mTimePos = 0.0f;

	if (mCurrClipName == "attack01" && mTimePos < mModel->SkinnedMeshData.GetClipEndTime(mCurrClipName))
		return false;
	else
		mCurrClipName = clipName;
}

void Player::Animate(float dt)
{
	mTimePos += dt;

	mModel->SkinnedMeshData.GetFinalTransforms(mCurrClipName, mTimePos, mFinalTransforms);

	// Loop animation
	if (mTimePos > mModel->SkinnedMeshData.GetClipEndTime(mCurrClipName))
	{
		mTimePos = 0.0f;
		if (mCurrClipName == "attack01")
			mCurrClipName = "stand";
	}
}


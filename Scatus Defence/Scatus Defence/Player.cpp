#include "Player.h"

Player::Player(SkinnedMesh* Mesh, const InstanceDesc& info) : SkinnedObject(Mesh, info)
{
	mTimePos = 0.0f;
	mMovingSpeed = 5.5f;
	mCurrClipName = "stand";
}


Player::~Player()
{
}

bool Player::SetClip(std::string clipName)
{
	if (clipName == "attack01")
		mTimePos = 0.0f;

	if (mCurrClipName == "attack01" && mTimePos < mMesh->SkinnedData.GetClipEndTime(mCurrClipName))
		return false;
	else
		mCurrClipName = clipName;
}

void Player::Animate(float dt)
{
	mTimePos += dt;

	mMesh->SkinnedData.GetFinalTransforms(mCurrClipName, mTimePos, mFinalTransforms);

	// Loop animation
	if (mTimePos > mMesh->SkinnedData.GetClipEndTime(mCurrClipName))
	{
		mTimePos = 0.0f;
		if (mCurrClipName == "attack01")		// attack01은 루프 안쓰는 애니메이션
			mCurrClipName = "stand";
	}
}

void Player::Release(ResourceMgr & rMgr)
{
	rMgr.ReleaseGoblinMesh(); // 임시
}


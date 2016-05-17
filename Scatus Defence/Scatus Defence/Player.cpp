#include "Player.h"

Player::Player(SkinnedMesh* Mesh, const InstanceDesc& info) : SkinnedObject(Mesh, info)
{
	mTimePos = 0.0f;
	mProperty.movespeed = 5.5f;
	mCurrClipName = "stand";
	mProperty.hp_now = 100;
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
		if (mCurrClipName == "attack01")		// attack01�� ���� �Ⱦ��� �ִϸ��̼�
			mCurrClipName = "stand";
	}
}

void Player::Release(ResourceMgr & rMgr)
{
	rMgr.ReleaseGoblinMesh(); // �ӽ�
}


#include "Monster.h"

Monster::Monster(SkinnedMesh* mesh, const InstanceDesc& info) : SkinnedObject(mesh, info)
{
}


Monster::~Monster()
{
}

// 이 메서드는 현재 타겟이 설정되어 있다고 가정한다.
void Monster::MoveToTarget(float dt)
{
	assert(mTarget);
	if (mCollisionState == CollisionState::None && mActionState != ActionState::Attack 
		&& mActionState != ActionState::Damage && mActionState != ActionState::Die)
	{
		XMVECTOR vTarget = MathHelper::TargetVector2D(mTarget->GetPos(), mPosition);

		XMVECTOR s = XMVectorReplicate(dt*mProperty.movespeed);
		XMVECTOR p = XMLoadFloat3(&mPosition);

		// 방향으로 이동
		XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, vTarget, p));

		// 방향으로 회전
		XMFLOAT3 fTargetDir;
		XMStoreFloat3(&fTargetDir, vTarget);
		float dot = -fTargetDir.x*mCurrLook.x - fTargetDir.z*mCurrLook.z;
		float det = -fTargetDir.x*mCurrLook.z + fTargetDir.z*mCurrLook.x;
		float angle = atan2(det, dot);

		RotateY(angle*dt*MathHelper::Pi);

		ChangeActionState(ActionState::Walk);
	}
}

void Monster::MovingCollision(const XMFLOAT3& crushedObjectPos, float dt)
{
	mCollisionState = CollisionState::MovingCollision;
	XMVECTOR vTarget = MathHelper::TargetVector2D(mTarget->GetPos(), mPosition);
	XMVECTOR vCrushedObject = MathHelper::TargetVector2D(crushedObjectPos, mPosition);
	XMVECTOR vDir = XMVector3Normalize(vTarget - vCrushedObject);

	XMVECTOR s = XMVectorReplicate(dt*mProperty.movespeed/2.0f);
	XMVECTOR p = XMLoadFloat3(&mPosition);

	// 부딪힌 오브젝트부터 멀어지는 방향
	XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, vDir, p));
}

void Monster::AttackToTarget(float dt)
{
	ChangeActionState(ActionState::Attack);

	// 방향으로 회전
	XMVECTOR vTarget = MathHelper::TargetVector2D(mTarget->GetPos(), mPosition);

	XMFLOAT3 fTargetDir;
	XMStoreFloat3(&fTargetDir, vTarget);
	float dot = -fTargetDir.x*mCurrLook.x - fTargetDir.z*mCurrLook.z;
	float det = -fTargetDir.x*mCurrLook.z + fTargetDir.z*mCurrLook.x;
	float angle = atan2(det, dot);

	RotateY(angle*dt*MathHelper::Pi);

	if((mCurrClipName == mAnimNames[Anims::attack1] ||
		mCurrClipName == mAnimNames[Anims::attack2]) && mTimePos == 0.0f)
		Attack(mTarget);
}
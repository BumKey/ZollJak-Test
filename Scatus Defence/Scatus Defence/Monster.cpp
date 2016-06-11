#include "Monster.h"

Monster::Monster(SkinnedMesh* mesh, const InstanceDesc& info) : SkinnedObject(mesh, info)
{
}


Monster::~Monster()
{
}

// �� �޼���� ���� Ÿ���� �����Ǿ� �ִٰ� �����Ѵ�.
void Monster::MoveToTarget(float dt)
{
	assert(mTarget);
	if (mCollisionState == CollisionState::None && mActionState != ActionState::Attack 
		&& mActionState != ActionState::Damage && mActionState != ActionState::Die)
	{
		XMVECTOR vTarget = MathHelper::TargetVector2D(mTarget->GetPos(), mPosition);

		XMVECTOR s = XMVectorReplicate(dt*mProperty.movespeed);
		XMVECTOR p = XMLoadFloat3(&mPosition);

		// �������� �̵�
		XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, vTarget, p));

		// �������� ȸ��
		mAngle = AngleToTarget(vTarget)*dt*MathHelper::Pi;

		RotateY(mAngle);

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

	// �ε��� ������Ʈ���� �־����� ����
	XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, vDir, p));
}

float Monster::AngleToTarget(XMVECTOR vTarget)
{
	// �������� ȸ��
	XMFLOAT3 fTargetDir;
	XMStoreFloat3(&fTargetDir, vTarget);
	float dot = -fTargetDir.x*mCurrLook.x - fTargetDir.z*mCurrLook.z;
	float det = -fTargetDir.x*mCurrLook.z + fTargetDir.z*mCurrLook.x;
	float angle = atan2(det, dot);

	if (mActionState != ActionState::Die || mActionState != ActionState::Damage)
		return angle;
	else
		return mAngle;
}

void Monster::AttackToTarget(float dt)
{
	ChangeActionState(ActionState::Attack);

	// �������� ȸ��
	XMVECTOR vTarget = MathHelper::TargetVector2D(mTarget->GetPos(), mPosition);
	mAngle = AngleToTarget(vTarget)*dt*MathHelper::Pi;

	RotateY(mAngle);

	if((mCurrClipName == mAnimNames[Anims::attack1] ||
		mCurrClipName == mAnimNames[Anims::attack2]) && mTimePos == 0.0f)
		Attack(mTarget);
}
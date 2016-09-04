#include "Monster.h"

Monster::Monster() : SkinnedObject()
{
	mAI_States = AI_State::None;
}

Monster::Monster(SkinnedMesh* mesh, const SO_InitDesc& info) : SkinnedObject(mesh, info)
{
	mAI_States = AI_State::None;
}


Monster::~Monster()
{
}

// �� �޼���� ���� Ÿ���� �����Ǿ� �ִٰ� �����Ѵ�.
void Monster::MoveToTarget(float dt)
{
	if (mCollisionState == CollisionState::None && mActionState != ActionState::Attack 
		&& mActionState != ActionState::Damage && mActionState != ActionState::Die)
	{
		XMVECTOR vTarget = MathHelper::TargetVector2D(mTargetPos, mPosition);

		XMVECTOR s = XMVectorReplicate(dt*mProperty.movespeed);
		XMVECTOR p = XMLoadFloat3(&mPosition);

		// �������� �̵�
		XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, vTarget, p));

		// �������� ȸ��
		float angle = MathHelper::AngleToTarget(vTarget, mCurrLook)*dt*MathHelper::Pi;

		RotateY(angle);

		ChangeActionState(ActionState::Walk);
	}
}

void Monster::Update(float dt)
{
	SkinnedObject::Update(dt);

	/*if (mActionState != ActionState::Die || mActionState != ActionState::Damage)
	{
		if (mAI_States == AI_State::AttackToTarget)
			AttackToTarget(dt);
		else if (mAI_States == AI_State::MovingToTarget)
			MoveToTarget(dt);
	}*/
}

void Monster::MovingCollision(const XMFLOAT3& crushedObjectPos, float dt)
{
	mCollisionState = CollisionState::MovingCollision;
	XMVECTOR vTarget = MathHelper::TargetVector2D(mTargetPos, mPosition);
	XMVECTOR vCrushedObject = MathHelper::TargetVector2D(crushedObjectPos, mPosition);
	XMVECTOR vDir = XMVector3Normalize(vTarget - vCrushedObject);

	XMVECTOR s = XMVectorReplicate(dt*mProperty.movespeed/2.0f);
	XMVECTOR p = XMLoadFloat3(&mPosition);

	// �ε��� ������Ʈ���� �־����� ����
	XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, vDir, p));
}

void Monster::SetAI_State(AI_State::States state)
{
	if (mAI_States != AI_State::AttackToTarget && state == AI_State::AttackToTarget)
	{
		mAI_States = state;
		SetAttackState();
	}
	else
		mAI_States = state;
}

void Monster::AttackToTarget(float dt)
{
	// �������� ȸ��
	XMVECTOR vTarget =	MathHelper::TargetVector2D(mTargetPos, mPosition);
	float angle =		MathHelper::AngleToTarget(vTarget, mCurrLook)*dt*MathHelper::Pi;

	RotateY(angle);

	if (OneHit())
	{
		//Attack(mTarget);
		//mTarget->ChangeActionState(ActionState::Damage);
	}
}
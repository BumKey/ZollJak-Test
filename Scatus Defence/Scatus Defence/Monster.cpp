#include "Monster.h"


Monster::Monster(SkinnedMesh* mesh, const InstanceDesc& info) : SkinnedObject(mesh, info), mHasTarget(false)
{
}


Monster::~Monster()
{
}

// 이 메서드는 현재 타겟이 설정되어 있다고 가정한다.
void Monster::MoveToTarget(float dt)
{
	assert(mTarget);
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

	mActionState = ActionState::Walk;
}

void Monster::AttackToTarget(float dt)
{
	mActionState = ActionState::Attack;

	// 방향으로 회전
	XMVECTOR vTarget = MathHelper::TargetVector2D(mTarget->GetPos(), mPosition);

	XMFLOAT3 fTargetDir;
	XMStoreFloat3(&fTargetDir, vTarget);
	float dot = -fTargetDir.x*mCurrLook.x - fTargetDir.z*mCurrLook.z;
	float det = -fTargetDir.x*mCurrLook.z + fTargetDir.z*mCurrLook.x;
	float angle = atan2(det, dot);

	RotateY(angle*dt*MathHelper::Pi);

	if (mTarget->GetActionState() != ActionState::Die)
	{
		int mTarget_hp = mTarget->GetProperty().hp_now;
		int armor = mTarget->GetProperty().guardpoint;
		float damage = mProperty.attakpoint;

		//mTarget->SetHP(mTarget_hp + (damage*(1 - (armor*0.06)) / (1 + 0.06*armor)));
		//mTarget->SetHP(mTarget_hp - damage);

		printf("공격을 성공했습니다. 상대의 체력 : %d \n", mTarget->GetProperty().hp_now);

		if (mTarget->GetProperty().hp_now <= -500)
		{
			printf("사망자 이름 : %s\n", mTarget->GetProperty().name);
		}
		if (mTarget->GetProperty().hp_now <= 0)
		{
			mTarget->Die();
			printf("타겟 사망");
			mActionState = ActionState::Idle;
		}
	}
}

void Monster::SetTarget(GameObject * target)
{
	if (target) {
		mTarget = target;
		mHasTarget = true;
	}
}

void Monster::SetNoTarget()
{
	mTarget = nullptr;
	mHasTarget = false;
}
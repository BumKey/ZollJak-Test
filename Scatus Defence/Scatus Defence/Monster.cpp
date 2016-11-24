#include "Monster.h"
#include "ObjectMgr.h"
#include "Sound_Manager.h"
#include "UI_Manager.h"
Monster::Monster() : SkinnedObject()
{
	mAI_States = AI_State::None;
	mTimer.Reset();
}

Monster::Monster(SkinnedMesh* mesh, const SO_InitDesc& info) : SkinnedObject(mesh, info)
{
	mAI_States = AI_State::None;

	mTimer.Reset();
}


Monster::~Monster()
{
}

// �� �޼���� ���� Ÿ���� �����Ǿ� �ִٰ� �����Ѵ�.
void Monster::MoveToTarget(float dt)
{
	if (MathHelper::Float3Equal(mPosition, mTargetPos))
	{
		ChangeActionState(ActionState::Idle);
	}
	else if (mHasTarget && mProperty.hp_now > 0 && mCollisionState == CollisionState::None && mActionState != ActionState::Attack
		&& mActionState != ActionState::Damage && mActionState != ActionState::Die)
	{
		XMVECTOR vTarget = MathHelper::TargetVector2D(mTargetPos, mPosition);

		XMVECTOR s;
		if(m_bDoubleSpeed)
			s = XMVectorReplicate(dt*mProperty.movespeed*2.0f);
		else
			s = XMVectorReplicate(dt*mProperty.movespeed);

		XMVECTOR p = XMLoadFloat3(&mPosition);

		// �������� �̵�
		XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, vTarget, p));

		// �������� ȸ��
		float angle = MathHelper::AngleToTarget(vTarget, mCurrLook)*dt*MathHelper::Pi;

		RotateY(angle);

		for (UINT i = 0; i < COLL_OBJ_NUM; ++i)
		{
			const XMFLOAT3& cp = Collision_Mgr->MonCollPos[mMonID][i];
			if (MathHelper::DistanceVector(mPosition, cp) < 3.0f)
			{
				XMFLOAT3 target;
				XMStoreFloat3(&target, MathHelper::TargetVector2D(cp, mPosition));

				mPosition = mPosition - target*dt*mProperty.movespeed;
			}
		}

		ChangeActionState(ActionState::Walk);
	}
}

void Monster::Update(float dt)
{
	if (mProperty.hp_now <= 0)
		mActionState = ActionState::Die;

	if (mActionState != ActionState::Die)
	{
		if ((mActionState != ActionState::Damage || mActionState != ActionState::Attack)
			&& mHasTarget)
		{
			if (MathHelper::DistanceVector(mPosition, mTarget->GetPos()) <= 3.0f)
				AttackToTarget(dt);
			else
				MoveToTarget(dt);
		}

		if (mTimer.TotalTime() > 1.0f)
		{
			switch (rand() % 7)
			{
			case 0: 		if (this->GetObjectType() == ObjectType::Goblin)
			{
				Sound_Mgr->Play3DEffect(Sound_Giant_roar2, this->GetPos().x, this->GetPos().y, this->GetPos().z);
			}
							else if (this->GetObjectType() == ObjectType::Cyclop)
							{
								Sound_Mgr->Play3DEffect(Sound_Giant_roar1, this->GetPos().x, this->GetPos().y, this->GetPos().z);
							}

							break;
			default: break;
			}
			mTimer.Reset();

		}
		else
			mTimer.Tick();

		SkinnedObject::Update(dt);
		mFrustumCull = Collision_Mgr->FrustumAABBCulling(this);
	}
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
	/*if (mAI_States != AI_State::AttackToTarget && state == AI_State::AttackToTarget)
	{
		mAI_States = state;
		SetAttackState();
	}
	else
		mAI_States = state;*/
}

void Monster::Attack(SkinnedObject * target)
{
	if (mHasTarget && target->GetActionState() != ActionState::Die && target->GetActionState() != ActionState::Damage)
	{
		int mTarget_hp = target->GetProperty().hp_now;
		int armor = target->GetProperty().guardpoint;
		float damage = mProperty.attakpoint;

		target->Damage(damage);
		m_bForOneHit = false;
	}
}

void Monster::AttackToTarget(float dt)
{
	if (mHasTarget && mProperty.hp_now > 0)
	{
		// �������� ȸ��
		XMVECTOR vTarget = MathHelper::TargetVector2D(mTargetPos, mPosition);
		float angle = MathHelper::AngleToTarget(vTarget, mCurrLook)*dt*MathHelper::Pi;

		RotateY(angle);
		ChangeActionState(ActionState::Attack);

		if (m_bForOneHit)
			Attack(mTarget);
	}
}
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

// 이 메서드는 현재 타겟이 설정되어 있다고 가정한다.
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

		// 방향으로 이동
		XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, vTarget, p));

		// 방향으로 회전
		float angle = MathHelper::AngleToTarget(vTarget, mCurrLook)*dt*MathHelper::Pi;

		RotateY(angle);

		ChangeActionState(ActionState::Walk);
	}
}

void Monster::FrustumCulling()
{
	auto m = mMesh->GetAABB();
	const Camera& cam = *Camera::GetInstance();

	XMVECTOR detView = XMMatrixDeterminant(cam.View());
	XMMATRIX invView = XMMatrixInverse(&detView, cam.View());

	// 현재 모델은 어떻게 누워있는지 통일되어 있지 않다.
	float max = -MathHelper::Infinity;
	max = MathHelper::Max(max, m.Extents.x);
	max = MathHelper::Max(max, m.Extents.y);
	max = MathHelper::Max(max, m.Extents.z);

	if (max != m.Extents.y)
	{
		m.Extents.x = m.Extents.y;
		m.Extents.y = max;
		m.Extents.z = m.Extents.x;
	}

	XMMATRIX W = XMLoadFloat4x4(&mWorld);
	XMMATRIX T = XMMatrixTranslation(0.0f, Terrain::GetInstance()->GetHeight(mPosition)+max*mScaling, 0.0f);
	W = XMMatrixMultiply(W, T);

	XMVECTOR v = XMLoadFloat3(&m.Center);
	v = XMVector3TransformCoord(v, W);
	
	// Decompose the matrix into its individual parts.
	XMVECTOR scale;
	XMVECTOR rotQuat;
	XMVECTOR translation;
	XMMatrixDecompose(&scale, &rotQuat, &translation, W);

	XNA::OrientedBox ob;
	XMStoreFloat3(&ob.Center, v);
	ob.Extents.x = m.Extents.x * mScaling;
	ob.Extents.y = max * mScaling;
	ob.Extents.z = m.Extents.z * mScaling;
	XMStoreFloat4(&ob.Orientation, rotQuat);

	XMMatrixDecompose(&scale, &rotQuat, &translation, invView);

	// Transform the camera frustum from view space to the object's local space.
	XNA::Frustum worldspaceFrustum;
	XNA::TransformFrustum(&worldspaceFrustum, &cam.GetCamFrustum(), XMVectorGetX(scale), rotQuat, translation);

	// Perform the box/frustum intersection test in local space.
	mFrustumCull = XNA::IntersectOrientedBoxFrustum(&ob, &worldspaceFrustum);
	if (mFrustumCull == 0)
		int a = 0;
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

	// 부딪힌 오브젝트부터 멀어지는 방향
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
		// 방향으로 회전
		XMVECTOR vTarget = MathHelper::TargetVector2D(mTargetPos, mPosition);
		float angle = MathHelper::AngleToTarget(vTarget, mCurrLook)*dt*MathHelper::Pi;

		RotateY(angle);
		ChangeActionState(ActionState::Attack);

		if (m_bForOneHit)
			Attack(mTarget);
	}
}
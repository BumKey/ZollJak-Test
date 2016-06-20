#include "GameObject.h"
UINT GameObject::GeneratedCount = 0;

GameObject::GameObject() : m_bForOneHit(false),
mPosition(0.0f, 0.0f, 0.0f), mScaling(1.0f), mRotation(0.0f, 0.0f, 0.0f), mDirection(0.0f, 0.0f, 0.0f),
mRight(1.0f, 0.0f, 0.0f), mUp(0.0f, 1.0f, 0.0f), mCurrLook(0.0f, 0.0f, -1.0f), mPrevLook(0.0f, 0.0f, -1.0f)
{
	mID = GeneratedCount++;
	mActionState = ActionState::Idle;
	mCollisionState = CollisionState::None;
}

GameObject::GameObject(GameMesh* mesh, const InstanceDesc& info) : mMesh(mesh), m_bForOneHit(false),
mPosition(0.0f, 0.0f, 0.0f), mScaling(1.0f), mRotation(0.0f, 0.0f, 0.0f), mDirection(0.0f, 0.0f, 0.0f),
mRight(1.0f, 0.0f, 0.0f), mUp(0.0f, 1.0f, 0.0f), mCurrLook(0.0f, 0.0f, -1.0f), mPrevLook(0.0f, 0.0f, -1.0f)
{
	mID = GeneratedCount++;
	mActionState = ActionState::Idle;
	mCollisionState = CollisionState::None;

	XMMATRIX S = XMMatrixScaling(info.Scale, info.Scale, info.Scale);
	XMMATRIX R = XMMatrixRotationRollPitchYaw(info.Rot.x, info.Rot.y, info.Rot.z);
	XMMATRIX T = XMMatrixTranslation(info.Pos.x, info.Pos.y, info.Pos.z);

	XMStoreFloat4x4(&mWorld, S*R*T);

	mScaling = info.Scale;
	mRotation = info.Rot;
	mPosition = info.Pos;

	R = XMMatrixRotationY(info.Rot.y);
	XMStoreFloat3(&mRight, XMVector3TransformNormal(XMLoadFloat3(&mRight), R));
	XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
	XMStoreFloat3(&mCurrLook, XMVector3TransformNormal(XMLoadFloat3(&mCurrLook), R));
}


GameObject::~GameObject()
{
}

void GameObject::Walk(float d)
{
	mDirection = mCurrLook;

	// mPosition += d*mLook
	XMVECTOR s = XMVectorReplicate(d*mProperty.movespeed);
	XMVECTOR l = XMLoadFloat3(&mCurrLook);
	XMVECTOR p = XMLoadFloat3(&mPosition);
	XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, l, p));

	ChangeActionState(ActionState::Run);
}

void GameObject::Strafe(float d)
{
	mDirection = mRight;

	// mPosition += d*mRight
	XMVECTOR s = XMVectorReplicate(d*mProperty.movespeed);
	XMVECTOR r = XMLoadFloat3(&mRight);
	XMVECTOR p = XMLoadFloat3(&mPosition);
	XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, r, p));

	ChangeActionState(ActionState::Run);
}

void GameObject::RotateY(float angle)
{
	mRotation.y += angle;
	mPrevLook = mCurrLook;
	// Rotate the basis vectors about the world y-axis.
	XMMATRIX R = XMMatrixRotationY(angle);

	XMStoreFloat3(&mRight, XMVector3TransformNormal(XMLoadFloat3(&mRight), R));
	XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
	XMStoreFloat3(&mCurrLook, XMVector3TransformNormal(XMLoadFloat3(&mCurrLook), R));
}

void  GameObject::PrintLocation()
{
	printf("%s 위치 출력: <%f, %f, %f> \n",
		mProperty.name, mPosition.x, mPosition.y, mPosition.z);
}

void GameObject::Die()
{
	mActionState = ActionState::Die;
	// 죽을 때 필요한 처리들은 오버라이드롤 구현..
}

void GameObject::Attack(GameObject * target)
{
	if (target->GetActionState() != ActionState::Die && target->GetActionState() != ActionState::Damage)
	{
		int mTarget_hp = target->GetProperty().hp_now;
		int armor = target->GetProperty().guardpoint;
		float damage = mProperty.attakpoint;

		target->SetHP(mTarget_hp + (damage*(1 - (armor*0.06)) / (1 + 0.06*armor)));
		target->SetHP(mTarget_hp - damage);

		printf("공격을 성공했습니다. 상대의 체력 : %d \n", target->GetProperty().hp_now);

		if (target->GetProperty().hp_now <= 0)
		{
			target->Die();
			printf("타겟 사망");
		}

		m_bForOneHit = false;
	}
}

void GameObject::ChangeActionState(ActionState::States aState)
{
	if (mActionState != ActionState::Die)
		mActionState = aState;
}

void GameObject::SetTarget(GameObject * target)
{
	if (target) {
		mTarget = target;
		mHasTarget = true;
	}
}

void GameObject::Release()
{
	assert(mObjectType != ObjectType::None);
	Resource_Mgr->ReleaseMesh(mObjectType);
}
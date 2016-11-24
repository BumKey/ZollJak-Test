#include "GameObject.h"
#include "Sound_Manager.h"
#include "SceneMgr.h"

UINT GameObject::GeneratedCount = 0;

GameObject::GameObject() : mExtentY(0.0f), mFrustumCull(false),
mPosition(0.0f, 0.0f, 0.0f), mScaling(1.0f), mRotation(0.0f, 0.0f, 0.0f), mDirection(0.0f, 0.0f, 0.0f),
mRight(1.0f, 0.0f, 0.0f), mUp(0.0f, 1.0f, 0.0f), mCurrLook(0.0f, 0.0f, -1.0f), mPrevLook(0.0f, 0.0f, -1.0f)
{
	mID = GeneratedCount++;
	mCollisionState = CollisionState::None;
}

GameObject::GameObject(GameMesh* mesh, const BO_InitDesc& info) : mMesh(mesh), mFrustumCull(false),
mPosition(0.0f, 0.0f, 0.0f), mScaling(1.0f), mRotation(0.0f, 0.0f, 0.0f), mDirection(0.0f, 0.0f, 0.0f), mExtentY(0.0f),
mRight(1.0f, 0.0f, 0.0f), mUp(0.0f, 1.0f, 0.0f), mCurrLook(0.0f, 0.0f, -1.0f), mPrevLook(0.0f, 0.0f, -1.0f)
{
	mID = GeneratedCount++;
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

	if(BoundaryCheck())
		XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, -l, p));
}

void GameObject::Strafe(float d)
{
	mDirection = mRight;

	// mPosition += d*mRight
	XMVECTOR s = XMVectorReplicate(d*mProperty.movespeed);
	XMVECTOR r = XMLoadFloat3(&mRight);
	XMVECTOR p = XMLoadFloat3(&mPosition);
	XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, r, p));

	if (BoundaryCheck())
		XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, -r, p));
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
	DEBUG_MSG("%s 위치 출력: <%f, %f, %f> \n",
		mProperty.name, mPosition.x, mPosition.y, mPosition.z);
}

bool GameObject::BoundaryCheck()
{
	if ((mPosition.x > 188.0f || mPosition.x < -188.0f)
		|| (mPosition.z > 188.0f || mPosition.z < -188.0f))
		return true;
	else
		return false;
}

void GameObject::Damage(float damage)
{
	//target->SetHP(mTarget_hp + (damage*(1 - (armor*0.06)) / (1 + 0.06*armor)));
	mProperty.hp_now -= damage;

	Sound_Mgr->Play3DEffect(Sound_impact, mPosition.x, mPosition.y, mPosition.z);
	//	Sound_Mgr->Play3DEffect(Sound_Giant_attack1, GetPos().x, GetPos().y, GetPos().z);
}

void GameObject::Release()
{
	assert(mObjectType != ObjectType::None);
	Resource_Mgr->ReleaseMesh(mObjectType);
}
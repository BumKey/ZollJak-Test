#include "GameObject.h"
UINT GameObject::GeneratedCount = 0;

GameObject::GameObject(GameMesh* mesh, const InstanceDesc& info) : mMesh(mesh),
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

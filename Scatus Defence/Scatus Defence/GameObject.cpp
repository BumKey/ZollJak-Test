#include "GameObject.h"
UINT GameObject::GeneratedCount = 0;

GameObject::GameObject(GameMesh* mesh) : mMesh(mesh), mHasTarget(false),
mPosition(0.0f, 0.0f, 0.0f), mScaling(1.0f), mRotation(0.0f, 0.0f, 0.0f), mDirection(0.0f, 0.0f, 0.0f),
mRight(1.0f, 0.0f, 0.0f), mUp(0.0f, 1.0f, 0.0f), mCurrLook(0.0f, 0.0f, -1.0f), mPrevLook(0.0f, 0.0f, -1.0f)
{
	XMStoreFloat4x4(&mWorld, XMMatrixIdentity());
	mID = GeneratedCount++;
}


GameObject::~GameObject()
{
}

void  GameObject::PrintLocation()
{
	printf("%s 위치 출력: <%f, %f, %f> \n", 
		mProperty.name, mPosition.x, mPosition.y, mPosition.z);
}

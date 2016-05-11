#include "GameObject.h"



GameObject::GameObject() :mPosition(0.0f, 0.0f, 0.0f), mScaling(1.0f), mRotation(0.0f, 0.0f, 0.0f),
mRight(1.0f, 0.0f, 0.0f), mUp(0.0f, 1.0f, 0.0f), mCurrLook(0.0f, 0.0f, -1.0f), mPrevLook(0.0f, 0.0f, -1.0f)
{
	XMStoreFloat4x4(&mWorld, XMMatrixIdentity());
}


GameObject::~GameObject()
{
}

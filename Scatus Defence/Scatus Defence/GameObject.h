#pragma once
#include <Camera.h>
#include "RenderStates.h"
#include "Effects.h"

struct InstanceDesc
{
	XMFLOAT3 Pos;
	FLOAT Yaw;
	FLOAT Scale;
};

class GameObject
{
public:
	GameObject();
	virtual ~GameObject();

public:
	virtual void DrawToScene(ID3D11DeviceContext* dc, const Camera& cam, XMFLOAT4X4 shadowTransform) = 0;
	virtual void DrawToShadowMap(ID3D11DeviceContext* dc, const Camera& cam, const XMMATRIX& lightViewProj) = 0;
	virtual void DrawToSsaoNormalDepthMap(ID3D11DeviceContext* dc, const Camera& cam) = 0;

	XMFLOAT4X4		GetWorld() { return mWorld; }
	XMFLOAT3		GetPos() { return mPosition; }
	XMFLOAT3		GetLook() { return mCurrLook; }

protected:
	XMFLOAT4X4 mWorld;

	FLOAT	 mScaling;
	XMFLOAT3 mRotation;
	XMFLOAT3 mPosition;

	XMFLOAT3 mRight;
	XMFLOAT3 mUp;
	XMFLOAT3 mCurrLook;
	XMFLOAT3 mPrevLook;
};


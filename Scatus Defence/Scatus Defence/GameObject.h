#pragma once
#include <Camera.h>
#include "RenderStates.h"
#include "Effects.h"
#include "GameModel.h"

struct InstanceDesc
{
	XMFLOAT3 Pos;
	FLOAT Yaw;
	FLOAT Scale;
};

class GameObject
{
public:
	GameObject(GameModel* model);
	virtual ~GameObject();

public:
	virtual void DrawToScene(ID3D11DeviceContext* dc, const Camera& cam, XMFLOAT4X4 shadowTransform, FLOAT tHeight) = 0;
	virtual void DrawToShadowMap(ID3D11DeviceContext* dc, const Camera& cam, const XMMATRIX& lightViewProj, FLOAT tHeight) = 0;
	virtual void DrawToSsaoNormalDepthMap(ID3D11DeviceContext* dc, const Camera& cam, FLOAT tHeight) = 0;

	GameModel*		GetModel() { return mModel; }
	XMFLOAT4X4		GetWorld() { return mWorld; }
	XMFLOAT3		GetPos() { return mPosition; }
	XMFLOAT3		GetLook() { return mCurrLook; }
	UINT			GetID() { return mID; }
	UINT			GetObjectGeneratedCount() { return GeneratedCount; }

private:
	UINT mID;
	static UINT GeneratedCount;

protected:
	XMFLOAT4X4 mWorld;
	GameModel* mModel;

	FLOAT	 mScaling;
	XMFLOAT3 mRotation;
	XMFLOAT3 mPosition;

	XMFLOAT3 mRight;
	XMFLOAT3 mUp;
	XMFLOAT3 mCurrLook;
	XMFLOAT3 mPrevLook;
};


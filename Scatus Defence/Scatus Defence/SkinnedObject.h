#pragma once
#include <Camera.h>
#include "SkinnedModel.h"
#include "RenderStates.h"
#include "Effects.h"

struct InstanceInfo
{
	XMFLOAT3 Pos;
	FLOAT Yaw;
	FLOAT Scale;
};

class SkinnedObject
{
public:
	SkinnedObject(SkinnedModel* model, const InstanceInfo& info);
	~SkinnedObject();

public:
	bool SetClip(std::string clipName);
	void Animate(float dt);
	void Strafe(float d);
	void Walk(float d);
	void RotateY(float angle);
	void Update(float terrainHeight);

	void DrawToScene(ID3D11DeviceContext* dc, const Camera& cam, XMFLOAT4X4 shadowTransform);
	void DrawToShadowMap(ID3D11DeviceContext* dc, const Camera& cam, const XMMATRIX& lightViewProj);
	void DrawToSsaoNormalDepthMap(ID3D11DeviceContext* dc, const Camera& cam);

	SkinnedModel*	GetModel() { return mModel; }
	XMFLOAT4X4		GetWorld() { return mWorld; }
	XMFLOAT3		GetPos() { return mPosition; }
	XMFLOAT3		GetLook() { return mCurrLook; }

private:
	SkinnedModel* mModel;
	XMFLOAT4X4 mWorld;

	FLOAT mScaling;
	XMFLOAT3 mRotation;
	XMFLOAT3 mPosition;

	XMFLOAT3 mRight;
	XMFLOAT3 mUp;
	XMFLOAT3 mCurrLook;
	XMFLOAT3 mPrevLook;

	FLOAT mTimePos;
	FLOAT mMovingSpeed;

	std::string mCurrClipName;
	std::vector<XMFLOAT4X4> mFinalTransforms;
};


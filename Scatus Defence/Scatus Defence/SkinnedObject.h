#pragma once
#include "GameObject.h"
#include "SkinnedModel.h"

class SkinnedObject : public GameObject
{
public:
	SkinnedObject(SkinnedModel* model, const InstanceDesc& info);
	virtual~SkinnedObject();

public:
	void Walk(float d);
	void Strafe(float d);
	void RotateY(float angle);
	void Update(float terrainHeight);

	void DrawToScene(ID3D11DeviceContext* dc, const Camera& cam, XMFLOAT4X4 shadowTransform, FLOAT tHeight);
	void DrawToShadowMap(ID3D11DeviceContext* dc, const Camera& cam, const XMMATRIX& lightViewProj, FLOAT tHeight);
	void DrawToSsaoNormalDepthMap(ID3D11DeviceContext* dc, const Camera& cam, FLOAT tHeight);

	virtual bool SetClip(std::string clipName) = 0;
	virtual void Animate(float dt) = 0;

protected:
	FLOAT mTimePos;
	FLOAT mMovingSpeed;

	std::string mCurrClipName;
	std::vector<XMFLOAT4X4> mFinalTransforms;
};


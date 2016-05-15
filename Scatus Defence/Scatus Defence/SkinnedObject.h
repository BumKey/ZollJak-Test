#pragma once
#include "GameObject.h"
#include "SkinnedMesh.h"

class SkinnedObject : public GameObject
{
public:
	SkinnedObject(SkinnedMesh* mesh, const InstanceDesc& info);
	virtual~SkinnedObject();

public:
	virtual void Walk(float d);
	virtual void Strafe(float d);
	virtual void MoveTo(Vector2D direction, float dt);
	virtual void RotateY(float angle);
	virtual void Attack(GameObject * Target);
	virtual void Update();

	virtual void DrawToScene(ID3D11DeviceContext* dc, const Camera& cam, XMFLOAT4X4 shadowTransform, FLOAT tHeight);
	virtual void DrawToShadowMap(ID3D11DeviceContext* dc, const Camera& cam, const XMMATRIX& lightViewProj, FLOAT tHeight);
	virtual void DrawToSsaoNormalDepthMap(ID3D11DeviceContext* dc, const Camera& cam, FLOAT tHeight);
	virtual void Release(ResourceMgr& rMgr) = 0;

	virtual void Animate(float dt) = 0;

	bool  AnimEnd(std::string clipName);
	void  SetSpeed(FLOAT speed) { mMovingSpeed = speed; }

protected:
	SkinnedMesh* mMesh;

	FLOAT mTimePos;
	FLOAT mMovingSpeed;

	std::string mCurrClipName;
	std::vector<XMFLOAT4X4> mFinalTransforms;
};


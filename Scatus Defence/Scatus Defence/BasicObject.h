#pragma once
#include "GameObject.h"
#include "BasicMesh.h"


enum Label {
	Basic,
	AlphaBasic
};

class BasicObject : public GameObject
{
public:
	BasicObject(BasicMesh* Mesh, const InstanceDesc& info, Label label);
	~BasicObject();

public:
	virtual void Walk(float d);
	virtual void Strafe(float d);
	virtual void MoveToTarget(float dt);
	virtual void RotateY(float angle);
	virtual void Update(float dt);

	virtual void DrawToScene(ID3D11DeviceContext* dc, const Camera& cam, const XMFLOAT4X4& shadowTransform, const FLOAT& tHeight);
	virtual void DrawToShadowMap(ID3D11DeviceContext* dc, const Camera& cam, const XMMATRIX& lightViewProj, const FLOAT& tHeight);
	virtual void DrawToSsaoNormalDepthMap(ID3D11DeviceContext* dc, const Camera& cam, const FLOAT& tHeight);

	virtual void Release(ResourceMgr* rMgr);

	Label GetLabel() { return mLabel; }

private:
	Label mLabel;

};


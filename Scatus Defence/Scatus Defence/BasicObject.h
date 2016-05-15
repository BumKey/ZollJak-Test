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
	virtual void MoveTo(Vector2D direction, float dt);
	virtual void RotateY(float angle);
	virtual void Update();

	virtual void DrawToScene(ID3D11DeviceContext* dc, const Camera& cam, XMFLOAT4X4 shadowTransform, FLOAT tHeight);
	virtual void DrawToShadowMap(ID3D11DeviceContext* dc, const Camera& cam, const XMMATRIX& lightViewProj, FLOAT tHeight);
	virtual void DrawToSsaoNormalDepthMap(ID3D11DeviceContext* dc, const Camera& cam, FLOAT tHeight);

	virtual void Release(ResourceMgr& rMgr);

	Label GetType() { return mLabel; }

private:
	Label mLabel;

};


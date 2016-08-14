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
	BasicObject();
	BasicObject(BasicMesh* mesh, const BO_InitDesc& info, Label label);
	~BasicObject();

public:
	virtual void Update(float dt);

	virtual void DrawToScene(ID3D11DeviceContext* dc, const XMFLOAT4X4& shadowTransform);
	virtual void DrawToShadowMap(ID3D11DeviceContext* dc, const XMMATRIX& lightViewProj);
	virtual void DrawToSsaoNormalDepthMap(ID3D11DeviceContext* dc);

	Label GetLabel() { return mLabel; }

private:
	Label mLabel;
};


#pragma once
#include "GameObject.h"
#include "BasicModel.h"


enum Label {
	Basic,
	AlphaBasic
};

class BasicObject : public GameObject
{
public:
	BasicObject(BasicModel* model, XMFLOAT4X4 world, Label label);
	~BasicObject();

	void Move(XMVECTOR direction, float dt);
	void DrawToScene(ID3D11DeviceContext* dc, const Camera& cam, XMFLOAT4X4 shadowTransform);
	void DrawToShadowMap(ID3D11DeviceContext* dc, const Camera& cam, const XMMATRIX& lightViewProj);
	void DrawToSsaoNormalDepthMap(ID3D11DeviceContext* dc, const Camera& cam);

	Label GetType() { return mLabel; }
	BasicModel * GetModel() { return mModel; }

private:
	BasicModel* mModel;	
	Label mLabel;

};


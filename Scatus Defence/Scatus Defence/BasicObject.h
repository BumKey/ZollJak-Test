#pragma once
#include <Camera.h>
#include "BasicModel.h"
#include "RenderStates.h"
#include "Effects.h"

enum Label {
	Basic,
	AlphaBasic
};

class BasicObject 
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
	XMFLOAT4X4 GetWorld() { return mWorld; }

private:
	BasicModel* mModel;	
	XMFLOAT4X4 mWorld;

	FLOAT mScaling;
	XMFLOAT3 mRotation;
	XMFLOAT3 mPosition;

	XMFLOAT3 mRight;
	XMFLOAT3 mUp;
	XMFLOAT3 mCurrLook;

	Label mLabel;

};


#pragma once
#include "BasicObject.h"

class Temple : public BasicObject, public Singletone<Temple>
{
public:
	Temple();
	~Temple();

	void Init(BasicMesh* mesh, const BO_InitDesc& info);

	virtual void DrawToScene(ID3D11DeviceContext* dc, const XMFLOAT4X4& shadowTransform);
	virtual void DrawToShadowMap(ID3D11DeviceContext* dc, const XMMATRIX& lightViewProj);
	virtual void DrawToSsaoNormalDepthMap(ID3D11DeviceContext* dc);
};


#pragma once
#include "BasicModel.h"
#include "d3dUtil.h"
#include <Camera.h>
#include "RenderStates.h"
#include "Effects.h"



// 월드 행렬, 메쉬 포인터, 그리기 함수를 가진
// 게임 객체 상태변화 관리

// 클래스 유형 : 엔티티

enum Model_Effect {
	Base = 0, Alpha
};

class GameObject
{
public:
	GameObject();
	GameObject(BasicModel* model, XMFLOAT4X4 world, Model_Effect me);
	~GameObject();

private:
	XMFLOAT4X4 mWorld;
	BasicModel* mModel;

	Model_Effect mME;

public:
	void Draw(ID3D11DeviceContext* dc, const Camera& cam, XMFLOAT4X4 shadowTransform);
	void Animate();
	void Move(XMVECTOR direction, float dt);

	void DrawObjectToShadowMap(ID3D11DeviceContext* dc, const Camera& cam, const XMMATRIX& lightViewProj);
	void DrawObjectToSsaoNormalDepthMap(ID3D11DeviceContext* dc, const Camera& cam);

	Model_Effect GetType();
	BasicModel* GetModel();
	XMFLOAT4X4 GetWorld();
};


#pragma once
#include <list>
#include <GeometryGenerator.h>
#include "Effects.h"
#include "GameObject.h"
#include "RenderStates.h"
#include <Camera.h>
#include <MathHelper.h>
#include "Sky.h"
#include "ShadowMap.h"
#include "Ssao.h"

#define MAX_OBJECTS		100
#define SMapSize		2048

struct BoundingSphere
{
	BoundingSphere() : Center(0.0f, 0.0f, 0.0f), Radius(0.0f) {}
	XMFLOAT3 Center;
	float Radius;
};

/// <summary>
/// 클래스 유형 : 컨트롤러
/// 장면 어디에 객체, 조명등을 넣을 건지,
/// 어떤 장면 효과를 쓸 것인 지 관장하는 클래스
class SceneMgr
{
public:
	SceneMgr(int width, int height);
	~SceneMgr();

private:
	// About Shadow
	void BuildShadowTransform();
	void BindDsvAndSetNullRenderTarget();

	// About Ssao
	void SetNormalDepthRenderTarget(ID3D11DepthStencilView* dsv);
	void BuildScreenQuadGeometryBuffers(ID3D11Device* device);
	void ComputeSsao();
	void BlurAmbientMap(int blurCount);

public:
	void Init(ID3D11Device* device, ID3D11DeviceContext* dc);
	void ReSize(UINT width, UINT height);
	void DrawAllObjects();
	void AnimateAllObjects();
	void UpdateScene(float dt);

	void CameraYawPitch(float dx, float dy);
	void DrawSky();

	void ComputeSceneBoundingBox();

	void CreateShadowMap();
	void CreateSsaoMap(ID3D11DepthStencilView* dsv);

	// About Debug
	void DrawScreenQuad();



	void AddObject(BasicModel* mesh, XMFLOAT4X4 world, Model_Effect me);
	DirectionalLight*			GetDirLight();

private:
	ID3D11DeviceContext* md3dImmediateContext;

	UINT mClientWidth;
	UINT mClientHeight;

	Camera mCam;							// 후에 mPlayer로 대체된다.
	std::list<GameObject*>  mObjects;		// 검색, 삭제가 빠르도록 오브젝트를 2진 트리에

	XMFLOAT4X4 mLightView;
	XMFLOAT4X4 mLightProj;
	XMFLOAT4X4 mShadowTransform;

	ID3D11Buffer* mScreenQuadVB;
	ID3D11Buffer* mScreenQuadIB;

	float mLightRotationAngle;
	XMFLOAT3 mOriginalLightDir[3];
	DirectionalLight mDirLights[3];

	ID3D11Buffer* mSkySphereVB;
	ID3D11Buffer* mSkySphereIB;

	ShadowMap* mSmap;
	Ssao* mSsao;
	Sky* mSky;

	BoundingSphere mSceneBounds;
};


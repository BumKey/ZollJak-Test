#pragma once
#include "Singletone.h"
#include "GeometryGenerator.h"
#include "Effects.h"
#include "RenderStates.h"
#include "ResourceMgr.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Camera.h"
#include "MathHelper.h"
#include "Terrain.h"
#include "Sky.h"
#include "ShadowMap.h"
#include "Ssao.h"
#include <list>

#define SMapSize		4092
#define Scene_Mgr SceneMgr::GetInstance()

struct BoundingSphere
{
	BoundingSphere() : Center(0.0f, 0.0f, 0.0f), Radius(0.0f) {}
	XMFLOAT3 Center;
	float Radius;
};

/// <summary>
/// 클래스 유형 : 컨트롤러
/// 장면의 그리기와 관련된 기능을 관장하는 클래스
/// </summary>
class SceneMgr : public Singletone<SceneMgr>
{
private:
	SceneMgr();
	~SceneMgr();

	friend class Singletone<SceneMgr>;
public:
	void Init(ID3D11Device* device, ID3D11DeviceContext * dc, 
		ID3D11DepthStencilView* dsv, ID3D11RenderTargetView* rtv, 
		UINT width, UINT height);
	void OnResize(UINT width, UINT height,
		ID3D11DepthStencilView* dsv, ID3D11RenderTargetView* rtv);
	void ComputeSceneBoundingBox();

	void Update(float dt);
	void DrawScene();

	DirectionalLight*		GetDirLight() { return mDirLights; }

private:
	void BuildShadowTransform();
	void CreateShadowMap();
	void CreateSsaoMap();

	void BuildScreenQuadGeometryBuffers(ID3D11Device* device);
	void BuildDebugSphere(ID3D11Device* device);
	void BuildTreeSpritesBuffer(ID3D11Device * device);
	void BuildGrassSpritesBuffer(ID3D11Device * device);

	void DrawTreeSprites();
	void DrawGrassSprites();
	void DrawScreenQuadSsao();
	void DrawScreenQuadShadow();
	void DrawBS();

private:
	float mLightRotationAngle;
	static const UINT TreeCount = 1000;
	static const UINT GrassCount = 2000;

	ID3D11DeviceContext* md3dImmediateContext;
	ID3D11DepthStencilView* mDepthStencilView;
	ID3D11RenderTargetView* mRenderTargetView;
	D3D11_VIEWPORT mScreenViewport;

	XMFLOAT3 mOriginalLightDir[3];
	DirectionalLight mDirLights[3];

	XMFLOAT4X4 mLightView;
	XMFLOAT4X4 mLightProj;
	XMFLOAT4X4 mShadowTransform;

	ID3D11Buffer* mScreenQuadVB;
	ID3D11Buffer* mScreenQuadIB;

	ID3D11Buffer* mTreeSpritesVB;
	ID3D11Buffer* mGrassSpritesVB;

	ID3D11Buffer* mSkySphereVB;
	ID3D11Buffer* mSkySphereIB;

	UINT mDSIndicesNum;
	ID3D11Buffer* mDebugSphereVB;
	ID3D11Buffer* mDebugSphereIB;

	ShadowMap* mSmap;
	Ssao* mSsao;
	Sky* mSky;

	Material mTreeMat;
	Material mGrassMat;

	ID3D11ShaderResourceView* mTreeTextureMapArraySRV;
	ID3D11ShaderResourceView* mGrassTextureMapArraySRV;
	BoundingSphere mSceneBounds;
};


#pragma once
#include "GeometryGenerator.h"
#include "Effects.h"
#include "BasicObject.h"
#include "SkinnedObject.h"
#include "RenderStates.h"
#include "ResourceMgr.h"
#include "Camera.h"
#include "MathHelper.h"
#include "Terrain.h"
#include "Sky.h"
#include "ShadowMap.h"
#include "Ssao.h"
#include <list>

#define SMapSize		2048

struct BoundingSphere
{
	BoundingSphere() : Center(0.0f, 0.0f, 0.0f), Radius(0.0f) {}
	XMFLOAT3 Center;
	float Radius;
};

/// <summary>
/// 클래스 유형 : 컨트롤러
/// 장면의 그리기와 관련된 기능을 관장하는 클래스
class SceneMgr
{
public:
	SceneMgr();
	~SceneMgr();

public:
	void Init(ID3D11Device* device, ID3D11DeviceContext * dc, 
		ID3D11DepthStencilView* dsv, ID3D11RenderTargetView* rtv, 
		const Camera& cam, UINT width, UINT height);
	void OnResize(UINT width, UINT height, const Camera& cam,
		ID3D11DepthStencilView* dsv, ID3D11RenderTargetView* rtv);
	void ComputeSceneBoundingBox(const std::vector<GameObject*>& allObjects);
	void Update(float dt);
	void DrawScene(const std::vector<GameObject*>& allObjects, const Camera& cam);

	DirectionalLight*		GetDirLight() { return mDirLights; }
	FLOAT					GetTerrainHeight(XMFLOAT3 pos) { return mTerrain.GetHeight(pos); }

private:
	void BuildShadowTransform();
	void CreateShadowMap(const std::vector<GameObject*>& allObjects, const Camera& cam);
	void CreateSsaoMap(const std::vector<GameObject*>& allObjects, const Camera& cam);
	void BuildScreenQuadGeometryBuffers(ID3D11Device* device);
	void DrawScreenQuad();

private:
	ID3D11DeviceContext* md3dImmediateContext;
	ID3D11DepthStencilView* mDepthStencilView;
	ID3D11RenderTargetView* mRenderTargetView;
	D3D11_VIEWPORT mScreenViewport;

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
	Terrain mTerrain;

	BoundingSphere mSceneBounds;
};


#pragma once
#include "Singletone.h"
#include "GeometryGenerator.h"
#include "Effects.h"
#include "BasicObject.h"
#include "SkinnedObject.h"
#include "RenderStates.h"
#include "ResourceMgr.h"
#include "ObjectMgr.h"
#include "Camera.h"
#include "MathHelper.h"
#include "Terrain.h"
#include "Sky.h"
#include "ShadowMap.h"
#include "Ssao.h"
#include <list>

#define SMapSize		2046
#define Scene_Mgr SceneMgr::GetInstance()

struct BoundingSphere
{
	BoundingSphere() : Center(0.0f, 0.0f, 0.0f), Radius(0.0f) {}
	XMFLOAT3 Center;
	float Radius;
};

/// <summary>
/// Ŭ���� ���� : ��Ʈ�ѷ�
/// ����� �׸���� ���õ� ����� �����ϴ� Ŭ����
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
	FLOAT					GetTerrainHeight(XMFLOAT3 pos) const { return mTerrain.GetHeight(pos); }

private:
	void BuildShadowTransform();
	void CreateShadowMap();
	void CreateSsaoMap();
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


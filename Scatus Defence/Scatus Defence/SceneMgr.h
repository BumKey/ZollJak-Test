#pragma once
#include <list>
#include <GeometryGenerator.h>
#include "Effects.h"
#include "BasicObject.h"
#include "SkinnedObject.h"
#include "RenderStates.h"
#include "ResourceMgr.h"
#include <Camera.h>
#include <MathHelper.h>
#include "Terrain.h"
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

public:
	void Init(ID3D11Device* device, ID3D11DeviceContext* dc, 
		ID3D11DepthStencilView* dsv, ID3D11RenderTargetView* rtv);
	void ReSize(UINT width, UINT height);
	void DrawScene();
	void AnimateAllObjects(float dt);
	void UpdateScene(float dt);

	void PlayerYawPitch(float dx, float dy);
	void CameraLookAt(const XMFLOAT3& camPos, const XMFLOAT3& target);
	void ComputeSceneBoundingBox();

	void AddBasicObject(BasicModel* mesh, XMFLOAT4X4 world, Label label);
	void AddSkinnedObject(SkinnedModel* mesh, InstanceInfo info);

	void					SetPlayer(SkinnedObject* player) { mPlayer = player; }
	DirectionalLight*		GetDirLight() { return mDirLights; }
	FLOAT					GetTerrainHeight(FLOAT x, FLOAT z)	{ return mTerrain.GetHeight(x, z); }

private:
	// About Shadow
	void BuildShadowTransform();
	void BindDsvAndSetNullRenderTarget();
	void CreateShadowMap();

	// About Ssao
	void CreateSsaoMap();
	void BuildScreenQuadGeometryBuffers(ID3D11Device* device);

	// About Debug
	void DrawScreenQuad();
private:
	ID3D11DeviceContext* md3dImmediateContext;
	ID3D11DepthStencilView* mDepthStencilView;
	ID3D11RenderTargetView* mRenderTargetView;
	D3D11_VIEWPORT mScreenViewport;

	UINT mClientWidth;
	UINT mClientHeight;

	Camera mCam;							
	SkinnedObject* mPlayer;

	std::list<BasicObject*>  mBasicObjects;		
	std::list<SkinnedObject*>  mSkinnedObjects;

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


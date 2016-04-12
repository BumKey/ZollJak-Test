#pragma once
#include "GameFramework.h"

GameFrameWork::GameFrameWork(HINSTANCE hInstance)
	: D3DApp(hInstance), mSceneMgr(mClientWidth, mClientHeight)
{
	mMainWndCaption = L"MeshView Demo";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;
}

GameFrameWork::~GameFrameWork()
{
	Effects::DestroyAll();
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();
}

bool GameFrameWork::Init()
{
	if (!D3DApp::Init())
		return false;

	// Must init Effects first since InputLayouts depend on shader signatures.
	Effects::InitAll(md3dDevice);
	InputLayouts::InitAll(md3dDevice);
	RenderStates::InitAll(md3dDevice);

	mTexMgr.Init(md3dDevice);
	mSceneMgr.Init(md3dDevice, md3dImmediateContext);
	ResourceMgr::InitAll(md3dDevice, mTexMgr);

	XMFLOAT4X4 treeWorld, baseWorld, stairsWorld, pillarWorld[4], rockWorld[3];

	XMMATRIX modelScale = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	XMMATRIX modelRot = XMMatrixRotationY(0.0f);
	XMMATRIX modelOffset = XMMatrixTranslation(0.0f, 0.0f, 0.0f);

	XMStoreFloat4x4(&treeWorld, modelScale*modelRot*modelOffset);
	XMStoreFloat4x4(&baseWorld, modelScale*modelRot*modelOffset);

	modelRot = XMMatrixRotationY(0.5f*XM_PI);
	modelOffset = XMMatrixTranslation(0.0f, -2.5f, -12.0f);
	XMStoreFloat4x4(&stairsWorld, modelScale*modelRot*modelOffset);

	modelScale = XMMatrixScaling(0.8f, 0.8f, 0.8f);
	modelOffset = XMMatrixTranslation(-5.0f, 1.5f, 5.0f);
	XMStoreFloat4x4(&pillarWorld[0], modelScale*modelRot*modelOffset);

	modelScale = XMMatrixScaling(0.8f, 0.8f, 0.8f);
	modelOffset = XMMatrixTranslation(5.0f, 1.5f, 5.0f);
	XMStoreFloat4x4(&pillarWorld[1], modelScale*modelRot*modelOffset);

	modelScale = XMMatrixScaling(0.8f, 0.8f, 0.8f);
	modelOffset = XMMatrixTranslation(5.0f, 1.5f, -5.0f);
	XMStoreFloat4x4(&pillarWorld[2], modelScale*modelRot*modelOffset);

	modelScale = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	modelOffset = XMMatrixTranslation(-5.0f, 1.0f, -5.0f);
	XMStoreFloat4x4(&pillarWorld[3], modelScale*modelRot*modelOffset);

	modelScale = XMMatrixScaling(0.8f, 0.8f, 0.8f);
	modelOffset = XMMatrixTranslation(-1.0f, 1.4f, -7.0f);
	XMStoreFloat4x4(&rockWorld[0], modelScale*modelRot*modelOffset);

	modelScale = XMMatrixScaling(0.8f, 0.8f, 0.8f);
	modelOffset = XMMatrixTranslation(5.0f, 1.2f, -2.0f);
	XMStoreFloat4x4(&rockWorld[1], modelScale*modelRot*modelOffset);

	modelScale = XMMatrixScaling(0.8f, 0.8f, 0.8f);
	modelOffset = XMMatrixTranslation(-4.0f, 1.3f, 3.0f);
	XMStoreFloat4x4(&rockWorld[2], modelScale*modelRot*modelOffset);

	// 굳이 컨테이너에 게임객체를 넣은 이유가 있는가 헷갈린다.
	// 다형성으로 프레임워크 변화를 줄이는 정도?
	mSceneMgr.AddObject(ResourceMgr::TreeModel, treeWorld, Model_Effect::Alpha);
	mSceneMgr.AddObject(ResourceMgr::BaseModel, baseWorld, Model_Effect::Base);
	mSceneMgr.AddObject(ResourceMgr::StairsModel, stairsWorld, Model_Effect::Base);
	mSceneMgr.AddObject(ResourceMgr::Pillar1Model, pillarWorld[0], Model_Effect::Base);
	mSceneMgr.AddObject(ResourceMgr::Pillar2Model, pillarWorld[1], Model_Effect::Base);
	mSceneMgr.AddObject(ResourceMgr::Pillar3Model, pillarWorld[2], Model_Effect::Base);
	mSceneMgr.AddObject(ResourceMgr::Pillar4Model, pillarWorld[3], Model_Effect::Base);
	for (int i = 0; i < 3; ++i)
		mSceneMgr.AddObject(ResourceMgr::RockModel, rockWorld[i], Model_Effect::Base);

	mSceneMgr.ComputeSceneBoundingBox();

	return true;
}

void GameFrameWork::OnResize()
{
	D3DApp::OnResize();

	mSceneMgr.ReSize(mClientWidth, mClientHeight);
}

void GameFrameWork::UpdateScene(float dt)
{
	mSceneMgr.UpdateScene(dt);
}

void GameFrameWork::DrawScene()
{
	mSceneMgr.CreateShadowMap();
	mSceneMgr.CreateSsaoMap(mDepthStencilView);

	//
	// Restore the back and depth buffer and viewport to the OM stage.
	//
	ID3D11RenderTargetView* renderTargets[1] = { mRenderTargetView };
	md3dImmediateContext->OMSetRenderTargets(1, renderTargets, mDepthStencilView);
	md3dImmediateContext->RSSetViewports(1, &mScreenViewport);

	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));

	// We already laid down scene depth to the depth buffer in the Normal/Depth map pass,
	// so we can set the depth comparison test to 밇QUALS.? This prevents any overdraw
	// in this rendering pass, as only the nearest visible pixels will pass this depth
	// comparison test.

	md3dImmediateContext->OMSetDepthStencilState(RenderStates::EqualsDSS, 0);

	mSceneMgr.DrawAllObjects();

	// Turn off wireframe.
	md3dImmediateContext->RSSetState(0);

	// Restore from RenderStates::EqualsDSS
	md3dImmediateContext->OMSetDepthStencilState(0, 0);

	// Debug view SSAO or Shadow map.
	// 실제 코드가서 조절
	mSceneMgr.DrawScreenQuad();

	mSceneMgr.DrawSky();

	// Unbind shadow map and AmbientMap as a shader input because we are going to render
	// to it next frame.  These textures can be at any slot, so clear all slots.
	ID3D11ShaderResourceView* nullSRV[16] = { 0 };
	md3dImmediateContext->PSSetShaderResources(0, 16, nullSRV);

	HR(mSwapChain->Present(0, 0));
}

void GameFrameWork::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void GameFrameWork::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void GameFrameWork::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		mSceneMgr.CameraYawPitch(dx, dy);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}


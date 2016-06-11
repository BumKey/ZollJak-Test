#pragma once
#include "GameFramework.h"

GameFrameWork::GameFrameWork(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
	mMainWndCaption = L"Scatus Defence Demo";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	mCam.SetLens(0.20f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
}

GameFrameWork::~GameFrameWork()
{
	Effects::DestroyAll();
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();
}

bool GameFrameWork::Init()
{
	//콘솔창 띄우기
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	freopen("CON", "w", stdout);
	if (!D3DApp::Init())
		return false;

	// Must init Effects first since InputLayouts depend on shader signatures.
	Effects::InitAll(md3dDevice);
	InputLayouts::InitAll(md3dDevice);
	RenderStates::InitAll(md3dDevice);

	mGameRogicMgr.Init(md3dDevice);
	mSceneMgr.Init(md3dDevice, md3dImmediateContext, 
		mDepthStencilView, mRenderTargetView,
		mCam, mClientWidth, mClientHeight);

	mPlayer = mGameRogicMgr.GetPlayer();

	XMFLOAT3 camPos = mPlayer->GetPos();
	camPos.y += 10.0f;
	camPos.z -= 20.0f;
	mCam.LookAt(camPos, mPlayer->GetPos(), XMFLOAT3(0.0f, 1.0f, 0.0f));

	mSceneMgr.ComputeSceneBoundingBox(mPlayer->GetPos());
	return true;
}

void GameFrameWork::OnResize()
{
	D3DApp::OnResize();

	mCam.SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	mSceneMgr.OnResize(mClientWidth, mClientHeight, mCam, 
		mDepthStencilView, mRenderTargetView);
}

void GameFrameWork::UpdateScene(float dt)
{
	mSceneMgr.Update(dt);
	mGameRogicMgr.Update(dt);
	mCam.Update(mPlayer, mSceneMgr);
}

void GameFrameWork::DrawScene()
{
	mSceneMgr.DrawScene(mGameRogicMgr.GetAllObjects(), mCam);
	HR(mSwapChain->Present(0, 0));
}

void GameFrameWork::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;
	mGameRogicMgr.OnMouseDown = true;
	if (btnState & MK_LBUTTON) {
		mPlayer->SetAttackState();
	}
	
	SetCapture(mhMainWnd);
}

void GameFrameWork::OnMouseUp(WPARAM btnState, int x, int y)
{
	mGameRogicMgr.OnMouseDown = false;
	ReleaseCapture();
}

void GameFrameWork::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		// Update안의 LookAt 세번째 변수 mUp으로 해주면
		// 어지럽지만 나름 멋있는 효과가?!
		mCam.Pitch(dy);			
		//mCam.RotateY(dx/3.0f);
		mPlayer->RotateY(dx*2.0f);

		mLastMousePos.x = x;
		mLastMousePos.y = y;
	}

	//static bool switcher(false);
	//if(switcher)
	//{
	//	// Make each pixel correspond to a quarter of a degree.
	//	float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
	//	float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

	//	mSceneMgr.PlayerRot.yPitch(dx, dy);

	//	mLastMousePos.x = x;
	//	mLastMousePos.y = y;
	//	switcher = false;
	//}
	//else
	//{
	//	mLastMousePos.x = x;
	//	mLastMousePos.y = y;
	//	if(m_bReady)
	//		switcher = true;
	//}
	
}


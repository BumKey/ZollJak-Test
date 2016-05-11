#pragma once
#include "GameFramework.h"

GameFrameWork::GameFrameWork(HINSTANCE hInstance)
	: D3DApp(hInstance), m_bReady(false), m_bAttackAnim(false)
{
	mMainWndCaption = L"Scatus Defence Demo";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	mSceneMgr = new SceneMgr(mClientWidth, mClientHeight);
}

GameFrameWork::~GameFrameWork()
{
	Effects::DestroyAll();
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();
	delete mSceneMgr;
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
	ResourceMgr::InitAll(md3dDevice, mTexMgr);

	mSceneMgr->Init(md3dDevice, md3dImmediateContext, mDepthStencilView, mRenderTargetView);

	InstanceDesc info;
	for (UINT i = 0; i < 10; ++i)
	{
		info.Pos = XMFLOAT3(rand() % 150, 0.0f, rand() % 150);
		info.Scale = MathHelper::RandF() + 0.5f;
		info.Yaw = MathHelper::RandF()*MathHelper::Pi*2;

		mSceneMgr->AddBasicObject(ResourceMgr::TreeModel, info, Label::AlphaBasic);
	}

	for (UINT i = 0; i < 20; ++i)
	{
		info.Pos = XMFLOAT3(rand() % 150, 0.0f, rand() % 150);
		info.Scale = MathHelper::RandF() + 0.5f;
		info.Yaw = MathHelper::RandF()*MathHelper::Pi * 2;

		mSceneMgr->AddBasicObject(ResourceMgr::RockModel, info, Label::Basic);
	}

	// mSceneMgr->AddSkinnedObject(ResourceMgr::Goblin, goblinWorld);

	info.Pos = XMFLOAT3(100.0f, 0.3f, 100.0f);
	info.Yaw = 0.0f;
	info.Scale = 0.2f;

	mPlayer = new Player(ResourceMgr::Goblin, info);
	mSceneMgr->SetPlayer(mPlayer);
	mSceneMgr->ComputeSceneBoundingBox();
	return true;
}

void GameFrameWork::OnResize()
{
	D3DApp::OnResize();

	mSceneMgr->ReSize(mClientWidth, mClientHeight);
}

void GameFrameWork::UpdateScene(float dt)
{
	//
	// Control the player.
	//

	if ((GetAsyncKeyState('W') & 0x8000) && (GetAsyncKeyState('A') & 0x8000))
	{
		mPlayer->SetClip("run");
		mPlayer->Walk(-dt);
		mPlayer->Strafe(-dt);
	}
	else if ((GetAsyncKeyState('W') & 0x8000) && (GetAsyncKeyState('D') & 0x8000))
	{
		mPlayer->SetClip("run");
		mPlayer->Walk(-dt);
		mPlayer->Strafe(dt);
	}
	else if ((GetAsyncKeyState('S') & 0x8000) && (GetAsyncKeyState('A') & 0x8000))
	{
		mPlayer->SetClip("run");
		mPlayer->Walk(dt);
		mPlayer->Strafe(-dt);
	}
	else if ((GetAsyncKeyState('S') & 0x8000) && (GetAsyncKeyState('D') & 0x8000))
	{
		mPlayer->SetClip("run");
		mPlayer->Walk(dt);
		mPlayer->Strafe(dt);
	}
	else if (GetAsyncKeyState('W') & 0x8000) {
		mPlayer->SetClip("run");
		mPlayer->Walk(-dt);
	}
	else if (GetAsyncKeyState('S') & 0x8000) {
		mPlayer->SetClip("run");
		mPlayer->Walk(dt);
	}
	else if (GetAsyncKeyState('A') & 0x8000) {
		mPlayer->SetClip("run");
		mPlayer->Strafe(-dt);
	}
	else if (GetAsyncKeyState('D') & 0x8000) {
		mPlayer->SetClip("run");
		mPlayer->Strafe(dt);
	}
	else if (m_bAttackAnim == false)
		mPlayer->SetClip("stand");

	mSceneMgr->UpdateScene(dt);
}

void GameFrameWork::DrawScene()
{
	mSceneMgr->DrawScene();
	HR(mSwapChain->Present(0, 0));
	m_bReady = true;
}

void GameFrameWork::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	if ((btnState & MK_LBUTTON) != 0)
	{
		m_bAttackAnim = true;
		mPlayer->SetClip("attack01");
	}

	SetCapture(mhMainWnd);
}

void GameFrameWork::OnMouseUp(WPARAM btnState, int x, int y)
{
	m_bAttackAnim = false;
	ReleaseCapture();
}

void GameFrameWork::OnMouseMove(WPARAM btnState, int x, int y)
{
	static bool switcher(false);
	if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		mSceneMgr->PlayerYawPitch(dx, dy);

		mLastMousePos.x = x;
		mLastMousePos.y = y;
	}

	//if(switcher)
	//{
	//	// Make each pixel correspond to a quarter of a degree.
	//	float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
	//	float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

	//	mSceneMgr->PlayerYawPitch(dx, dy);

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


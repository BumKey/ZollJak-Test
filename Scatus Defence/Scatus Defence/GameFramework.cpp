#pragma once
#include "GameFramework.h"
#include "Goblin.h"

GameFrameWork::GameFrameWork(HINSTANCE hInstance)
	: D3DApp(hInstance), m_bReady(false), m_bAttackAnim(false)
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

	mObjectMgr.ReleaseAll(mResourceMgr);	// 지금은 큰 의미는 없음.
	mPlayer->Release(mResourceMgr);
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
	mResourceMgr.Init(md3dDevice, &mTexMgr);

	mSceneMgr.Init(md3dDevice, md3dImmediateContext, 
		mDepthStencilView, mRenderTargetView,
		mCam, mClientWidth, mClientHeight);

	InstanceDesc info;

	// 250이 거의 끝자리
	info.Pos = XMFLOAT3(100.0f, 0.05f, -180.0f);
	info.Yaw = 0.0f;
	info.Scale = 0.2f;

	mPlayer = new Player(mResourceMgr.GetGoblinMesh(), info);
	mObjectMgr.SetPlayer(mPlayer);

	for (UINT i = 0; i < 10; ++i)
	{
		info.Pos = XMFLOAT3(mPlayer->GetPos().x +50.0f - rand() % 100,
			0, mPlayer->GetPos().z + 50.0f -rand() % 100);
		info.Scale = MathHelper::RandF()*2.0f + 0.5f;
		info.Yaw = MathHelper::RandF()*MathHelper::Pi*2;

		mObjectMgr.AddObstacle(new BasicObject(mResourceMgr.TreeMesh, info, Label::AlphaBasic));
	}

	for (UINT i = 0; i < 20; ++i)
	{
		info.Pos = XMFLOAT3(mPlayer->GetPos().x + 50.0f - rand() % 100,
			0, mPlayer->GetPos().z + 50.0f - rand() % 100);
		info.Scale = MathHelper::RandF()*2.0f + 0.5f;
		info.Yaw = MathHelper::RandF()*MathHelper::Pi * 2;

		mObjectMgr.AddObstacle(new BasicObject(mResourceMgr.RockMesh, info, Label::Basic));
	}

	for (UINT i = 0; i < 10; ++i)
	{
		info.Pos = XMFLOAT3(mPlayer->GetPos().x + 50.0f - rand() % 100,
			0, mPlayer->GetPos().z + 50.0f - rand() % 100);
		info.Scale = MathHelper::RandF()*2.0f + 0.5f;
		info.Yaw = MathHelper::RandF()*MathHelper::Pi * 2;
		
		GoblinType type;
		if (i % 2) type = GoblinType::Red;
		else	   type = GoblinType::Blue;
		mObjectMgr.AddMonster(new Goblin(mResourceMgr.GetGoblinMesh(), info, type));
	}

	XMFLOAT3 camPos = mPlayer->GetPos();
	camPos.y += 10.0f;
	camPos.z -= 20.0f;
	mCam.LookAt(camPos, mPlayer->GetPos(), XMFLOAT3(0.0f, 1.0f, 0.0f));

	mObjectMgr.Update();
	mSceneMgr.ComputeSceneBoundingBox(mObjectMgr.GetAllObjects());
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
	//
	// Control the player.
	//

	if (m_bAttackAnim == false)
	{
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
		else
			mPlayer->SetClip("stand");
	}
	
	if(m_bAttackAnim && mPlayer->AnimEnd("attack01"))
		m_bAttackAnim = false;

	mObjectMgr.Update(dt);
	mSceneMgr.Update(dt);
	mCam.Update(mPlayer, mSceneMgr);
}

void GameFrameWork::DrawScene()
{
	mSceneMgr.DrawScene(mObjectMgr.GetAllObjects(), mCam);
	HR(mSwapChain->Present(0, 0));
	m_bReady = true;
}

void GameFrameWork::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	if ((btnState & MK_LBUTTON) != 0 && !m_bAttackAnim)
	{
		m_bAttackAnim = true;
		mPlayer->SetClip("attack01");
	}

	SetCapture(mhMainWnd);
}

void GameFrameWork::OnMouseUp(WPARAM btnState, int x, int y)
{
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

	//	mSceneMgr.PlayerYawPitch(dx, dy);

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


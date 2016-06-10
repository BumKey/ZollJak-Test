#pragma once
#include "GameFramework.h"

GameFrameWork::GameFrameWork(HINSTANCE hInstance)
	: D3DApp(hInstance), m_bReady(false)
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

	mObjectMgr.ReleaseAll();	// ������ ū �ǹ̴� ����.
	mPlayer->Release(mResourceMgr);
}

bool GameFrameWork::Init()
{
	//�ܼ�â ����
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	freopen("CON", "w", stdout);
	if (!D3DApp::Init())
		return false;

	// Must init Effects first since InputLayouts depend on shader signatures.
	Effects::InitAll(md3dDevice);
	InputLayouts::InitAll(md3dDevice);
	RenderStates::InitAll(md3dDevice);
	mResourceMgr.Init(md3dDevice);
	mObjectMgr.Init(&mResourceMgr);
	mGameRogicMgr = new GameRogicManager(&mObjectMgr); //����
	mCollisionMgr.Init(&mObjectMgr);

	mSceneMgr.Init(md3dDevice, md3dImmediateContext, 
		mDepthStencilView, mRenderTargetView,
		mCam, mClientWidth, mClientHeight);

	InstanceDesc info;

	// 250�� ���� ���ڸ�
	info.Pos = XMFLOAT3(170.0f, 0.05f, -280.0f);
	info.Rot.y = 0.0f;
	info.Scale = 0.2f;

	mPlayer = new Player(mResourceMgr.GetSkinnedMesh(ObjectType::Goblin), info);
	mObjectMgr.SetPlayer(mPlayer);

	info.Pos = XMFLOAT3(195.0f, 0.05f, -300.0f);
	info.Rot.y = 0.0f;
	info.Scale = 0.3f;

	mObjectMgr.AddObstacle(new BasicObject(mResourceMgr.Temple, info, Label::Basic));

	for (UINT i = 0; i < 10; ++i)
	{
		info.Pos = XMFLOAT3(mPlayer->GetPos().x +50.0f - rand() % 100,
			-0.1f, mPlayer->GetPos().z + 50.0f -rand() % 100);
		info.Scale = MathHelper::RandF()*2.0f + 0.5f;
		info.Rot.y = MathHelper::RandF()*MathHelper::Pi*2;

		mObjectMgr.AddObstacle(new BasicObject(mResourceMgr.TreeMesh, info, Label::AlphaBasic));
	}

	for (UINT i = 0; i < 20; ++i)
	{
		info.Pos = XMFLOAT3(mPlayer->GetPos().x + 50.0f - rand() % 100,
			-0.1f, mPlayer->GetPos().z + 50.0f - rand() % 100);
		info.Scale = MathHelper::RandF()*2.0f + 0.5f;
		info.Rot.y = MathHelper::RandF()*MathHelper::Pi * 2.0f;

		mObjectMgr.AddObstacle(new BasicObject(mResourceMgr.RockMesh, info, Label::Basic));
	}

	for (UINT i = 0; i < 10; ++i)
	{
		info.Pos = XMFLOAT3(mPlayer->GetPos().x + 50.0f + rand() % 150,
			-0.1f, mPlayer->GetPos().z + 50.0f + rand() % 150);
		info.Scale = 0.1f + MathHelper::RandF() / 5.0f;
		info.Rot.y = 0;
		
		Goblin::Type type;
		if (i % 2) {
			type = Goblin::Type::Blue;
			info.Scale = 0.4f;
		}
		else if (i % 3)
		{
			type = Goblin::Type::Red;
			info.Scale = 0.3f;
		}
		else if (i % 5)
		{
			type = Goblin::Type::Red;
			info.Scale = 0.7f;
		}
		else {
			type = Goblin::Type::Blue;
			info.Scale = 0.6f;
		}
		mObjectMgr.AddMonster(new Goblin(mResourceMgr.GetSkinnedMesh(ObjectType::Goblin), info, type));
	}

	for (UINT i = 0; i < 2; ++i) {
		info.Pos = XMFLOAT3(mPlayer->GetPos().x + 50.0f + rand() % 150,
			-0.1f, mPlayer->GetPos().z + 50.0f + rand() % 150);
		info.Scale = 8.0f + MathHelper::RandF();
		info.Rot.x = MathHelper::Pi;
		info.Rot.y = 0.0f;

		mObjectMgr.AddMonster(new Cyclop(mResourceMgr.GetSkinnedMesh(ObjectType::Cyclop), info));
	}

	XMFLOAT3 camPos = mPlayer->GetPos();
	camPos.y += 10.0f;
	camPos.z -= 20.0f;
	mCam.LookAt(camPos, mPlayer->GetPos(), XMFLOAT3(0.0f, 1.0f, 0.0f));

	mObjectMgr.Update();
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
	mObjectMgr.Update(dt);
	mSceneMgr.Update(dt);
	mGameRogicMgr->Update(dt);
	mCollisionMgr.Update(dt);
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
	mGameRogicMgr->OnMouseDown = true;
	if (btnState & MK_LBUTTON) {
		mPlayer->SetAttackState();
		mCollisionMgr.AttackCollision();
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

		// Update���� LookAt ����° ���� mUp���� ���ָ�
		// ���������� ���� ���ִ� ȿ����?!
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


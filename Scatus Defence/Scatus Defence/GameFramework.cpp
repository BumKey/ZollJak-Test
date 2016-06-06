#pragma once
#include "GameFramework.h"

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

	mObjectMgr.ReleaseAll(mResourceMgr);	// ������ ū �ǹ̴� ����.
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
	mSwapChain->GetBuffer(0, _uuidof(IDXGISurface), (LPVOID*)&UI_Mgr->m_backbuffer);
	UI_Mgr->CreateD2DrenderTarget(D3DApp::GetHwnd());
	mGameRogicMgr = new GameRogicManager(&mObjectMgr, &mResourceMgr); //����

	mSceneMgr.Init(md3dDevice, md3dImmediateContext, 
		mDepthStencilView, mRenderTargetView,
		mCam, mClientWidth, mClientHeight);

	InstanceDesc info;

	// 250�� ���� ���ڸ�
	info.Pos = XMFLOAT3(170.0f, 0.05f, -280.0f);
	info.Rot.y = 0.0f;
	info.Scale = 0.2f;

	mPlayer = new Player(mResourceMgr.GetSkinnedMesh(Object_type::goblin), info);
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
		mObjectMgr.AddMonster(new Goblin(mResourceMgr.GetSkinnedMesh(Object_type::goblin), info, type));
	}

	for (UINT i = 0; i < 2; ++i) {
		info.Pos = XMFLOAT3(mPlayer->GetPos().x + 50.0f + rand() % 150,
			-0.1f, mPlayer->GetPos().z + 50.0f + rand() % 150);
		info.Scale = 8.0f + MathHelper::RandF();
		info.Rot.x = MathHelper::Pi;
		info.Rot.y = 0.0f;

		mObjectMgr.AddMonster(new Cyclop(mResourceMgr.GetSkinnedMesh(Object_type::cyclop), info));
	}

	XMFLOAT3 camPos = mPlayer->GetPos();
	camPos.y += 10.0f;
	camPos.z -= 20.0f;
	mCam.LookAt(camPos, mPlayer->GetPos(), XMFLOAT3(0.0f, 1.0f, 0.0f));

	mObjectMgr.Update();
	mSceneMgr.ComputeSceneBoundingBox(mPlayer->GetPos());
	UI_Mgr->Add_Text(L"���� ����", (UI_Mgr->rc.left + UI_Mgr->rc.right) /
		2 - 100, (UI_Mgr->rc.top + UI_Mgr->rc.bottom) / 2 - 150, 1);
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
	mGameRogicMgr->Update(dt);
	mCollisionMgr.Collision(mObjectMgr, dt);
	mCam.Update(mPlayer, mSceneMgr);
}

void GameFrameWork::DrawScene()
{
	mSceneMgr.DrawScene(mObjectMgr.GetAllObjects(), mCam);
	UI_Mgr->Print_All_UI();
	HR(mSwapChain->Present(0, 0));
	m_bReady = true;
}

void GameFrameWork::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;
	mGameRogicMgr->OnMouseDown = true;



		if (UI_Mgr->MouseOn2D(L"���̵� �Է�", x, y))
		{
			UI_Mgr->Set_input_ID_state(true);
			UI_Mgr->Delete_Text(L"���̵� �Է�â");

		}

		if (UI_Mgr->MouseOn2D(L"�α���", x, y))
		{

			UI_Mgr->Delete_Text(L"�α���");


		}
		if (UI_Mgr->MouseOn2D(L"�޽����ڽ�", x, y))
		{
			UI_Mgr->Set_Image_Active(L"�޽����ڽ�", false);
		}
	

	if ((btnState & MK_LBUTTON) != 0 && !m_bAttackAnim)
	{
		m_bAttackAnim = true;
		mPlayer->SetClip("attack01");
	}

	SetCapture(mhMainWnd);
}

void GameFrameWork::OnMouseUp(WPARAM btnState, int x, int y)
{
	if (UI_Mgr->MouseOn2D(L"�α���", x, y))
	{

		UI_Mgr->Delete_All_Image();
		UI_Mgr->Delete_Text_All();
		UI_Mgr->Set_Image_Active(L"�޽����ڽ�", true);

	}
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
		mPlayer->SetState(type_attack);

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


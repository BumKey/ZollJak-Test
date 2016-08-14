#pragma once
#include "GameFramework.h"

GameFrameWork::GameFrameWork(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
	srand(time(NULL));
	mMainWndCaption = L"Scatus Defence Demo";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	Camera::GetInstance()->SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
}

GameFrameWork::~GameFrameWork()
{
	Effects::DestroyAll();
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();
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

	Scene_Mgr->Init(md3dDevice, md3dImmediateContext,
		mDepthStencilView, mRenderTargetView, mClientWidth, mClientHeight);

	Texture_Mgr->Init(md3dDevice);
	Resource_Mgr->Init(md3dDevice);

	Terrain::InitInfo tii;
	tii.HeightMapFilename = L"Textures/terrain3.raw";
	tii.LayerMapFilename0 = L"Textures/grass2.dds";
	tii.LayerMapFilename1 = L"Textures/darkdirt.dds";
	tii.LayerMapFilename2 = L"Textures/red_dirt.dds";
	tii.LayerMapFilename3 = L"Textures/lightdirt.dds";
	tii.LayerMapFilename4 = L"Textures/dirt.dds";
	tii.BlendMapFilename = L"Textures/blend.dds";
	tii.HeightScale = 70.0f;
	tii.HeightmapWidth = 513;
	tii.HeightmapHeight = 513;
	tii.CellSpacing = 3.0f;
	Terrain::GetInstance()->Init(md3dDevice, md3dImmediateContext, tii);

	InstanceDesc info;
	// 250�� ���� ���ڸ�
	info.Pos = XMFLOAT3(210.0f, 0.01f, -280.0f);
	info.Rot.y = 0.0f;
	info.Scale = 0.05f;
	Player::GetInstance()->Init(Resource_Mgr->GetSkinnedMesh(ObjectType::Warrior), info);

	Object_Mgr->Init();

	UI_Mgr->CreateFactorys();
	UI_Mgr->CreateD2DrenderTarget(D3DApp::MainWnd(), mSwapChain);
	UI_Mgr->Load_All_UI();

	Sound_Mgr->Create_Sound(D3DApp::MainWnd());

	// Giljune's Code
	Packet_Mgr->Init();

	const XMFLOAT3& playerPos = Player::GetInstance()->GetPos();
	XMFLOAT3 camPos = playerPos;
	camPos.y += 10.0f;
	camPos.z -= 20.0f;
	Camera::GetInstance()->LookAt(camPos, playerPos, XMFLOAT3(0.0f, 1.0f, 0.0f));

	Object_Mgr->Update();
	Scene_Mgr->ComputeSceneBoundingBox();
	
	return true;
}

void GameFrameWork::OnResize()
{
	D3DApp::OnResize();

	Camera::GetInstance()->SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	Scene_Mgr->OnResize(mClientWidth, mClientHeight, mDepthStencilView, mRenderTargetView);

	UI_Mgr->CreateFactorys();
	UI_Mgr->CreateD2DrenderTarget(D3DApp::MainWnd(), mSwapChain);
	//UI_Mgr->Load_All_UI();
}

void GameFrameWork::UpdateScene(float dt)
{
	// 1. �������� ���ŵ� ������(���͵�)�� �޴´�.
	// 2. Ŭ���̾�Ʈ�� �� ������ ����� �׸���.
	// 3. Ŭ���̾�Ʈ���� Ű����, ���콺 �� �̺�Ʈ�� �߻��Ѵ�.
	// 3. Ŭ���̾�Ʈ�� �׿� ���� ���ŵ� �����͸� ������ ������.
	// 4. ������ �� Ŭ���̾�Ʈ���� ���� ������ ����ȭ�Ѵ�.
	
	if (Packet_Mgr->ReadPacket())
	{
		cs_packet_success packet;
		packet.size = sizeof(packet);
		packet.type = CS_SUCCESS;
		Packet_Mgr->SendPacket(packet);
	}

	State_Mgr->Update(dt);
	Object_Mgr->Update(dt);
	Collision_Mgr->Update(dt);
	Rogic_Mgr->Update(dt);

	Scene_Mgr->Update(dt);

	Player::GetInstance()->Update(dt);
	Camera::GetInstance()->Update();

}

void GameFrameWork::DrawScene()
{
	Scene_Mgr->DrawScene();
	UI_Mgr->Print_All_UI();

	HR(mSwapChain->Present(0, 0));
}

void GameFrameWork::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;
	Rogic_Mgr->OnMouseDown = true;
	if (btnState & MK_LBUTTON) {
		Player::GetInstance()->SetAttackState();
	}
	
	SetCapture(mhMainWnd);
}

void GameFrameWork::OnMouseUp(WPARAM btnState, int x, int y)
{
	Rogic_Mgr->OnMouseDown = false;
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
		Camera::GetInstance()->Pitch(dy);			
		//mCam.RotateY(dx/3.0f);
		Player::GetInstance()->RotateY(dx*2.0f);

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

void GameFrameWork::OnKeyDown(WPARAM keyState)
{
	//Player::GetInstance()->Mo
}


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
	tii.CellSpacing = 2.0f;
	Terrain::GetInstance()->Init(md3dDevice, md3dImmediateContext, tii);

	Camera::GetInstance()->SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);

	Scene_Mgr->Init(md3dDevice, md3dImmediateContext,
		mDepthStencilView, mRenderTargetView,
		mClientWidth, mClientHeight);

	Texture_Mgr->Init(md3dDevice);
	Resource_Mgr->Init(md3dDevice);

	Sound_Mgr->Create_Sound(D3DApp::MainWnd());

	// Giljune's Code
	Packet_Mgr->Init();

	return true;
}

void GameFrameWork::OnResize()
{
	D3DApp::OnResize();

	Camera::GetInstance()->SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	Scene_Mgr->OnResize(mClientWidth, mClientHeight, mDepthStencilView, mRenderTargetView);
}

void GameFrameWork::UpdateScene(float dt)
{
	// 1. 서버에서 갱신된 데이터(몬스터들)를 받는다.
	// 2. 클라이언트가 그 정보로 장면을 그린다.
	// 3. 클라이언트에서 키보드, 마우스 등 이벤트가 발생한다.
	// 3. 클라이언트가 그에 따라 갱신된 데이터를 서버로 보낸다.
	// 4. 서버는 각 클라이언트에서 받은 정보를 동기화한다.
	
	if (Packet_Mgr->ReadPacket())
	{
		Object_Mgr->Update(dt);

		Camera::GetInstance()->Update();
		Scene_Mgr->Update(dt);

		Packet_Mgr->SendPacket();
	}
	else
		UpdateScene(dt);
}

void GameFrameWork::DrawScene()
{
	Scene_Mgr->DrawScene();

	HR(mSwapChain->Present(0, 0));
}

void GameFrameWork::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;
	if (btnState & MK_LBUTTON) {
		Player::GetInstance()->SetAttackState();

		CS_Attack packet;
		Packet_Mgr->SetSendState(PacketMgr::ATTACK);
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


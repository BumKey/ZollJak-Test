#pragma once
#include "GameFramework.h"

GameFrameWork::GameFrameWork(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
	srand(time(NULL));
	mMainWndCaption = L"Scatus Defence Demo";

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
	//콘솔창 띄우기
#ifdef _DEBUG
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	freopen("CON", "w", stdout); 
#endif

	if (!D3DApp::Init())
		return false;

	// Must init Effects first since InputLayouts depend on shader signatures.
	Effects::InitAll(md3dDevice);
	InputLayouts::InitAll(md3dDevice);
	RenderStates::InitAll(md3dDevice);

	Terrain::InitInfo tii;
	tii.HeightMapFilename = L"Textures/terrain4.raw";
	tii.LayerMapFilename0 = L"Textures/grass2.dds";
	tii.LayerMapFilename1 = L"Textures/darkdirt.dds";
	tii.LayerMapFilename2 = L"Textures/red_dirt.dds";
	tii.LayerMapFilename3 = L"Textures/lightdirt.dds";
	tii.LayerMapFilename4 = L"Textures/dirt.dds";
	tii.BlendMapFilename = L"Textures/blend.dds";
	tii.HeightScale = 20.0f;
	tii.HeightmapWidth = 257;
	tii.HeightmapHeight = 257;
	tii.CellSpacing = 1.5f;
	Terrain::GetInstance()->Init(md3dDevice, md3dImmediateContext, tii);

	Camera::GetInstance()->SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 300.0f);

	Scene_Mgr->Init(md3dDevice, md3dImmediateContext,
		mDepthStencilView, mRenderTargetView,
		mClientWidth, mClientHeight);

	Texture_Mgr->Init(md3dDevice);
	Resource_Mgr->Init(md3dDevice);
	mSwapChain->GetBuffer(0, __uuidof(IDXGISurface), (LPVOID*)&UI_Mgr->m_backbuffer);
	UI_Mgr->CreateD2DrenderTarget(D3DApp::MainWnd());
	Sound_Mgr->Create_Sound(D3DApp::MainWnd());

	// Giljune's Code
	Packet_Mgr->Init(mhMainWnd);

	Scene_Mgr->ComputeSceneBoundingBox();

	return true;
}

void GameFrameWork::OnResize()
{
	D3DApp::OnResize();

	Camera::GetInstance()->SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 300.0f);
	Scene_Mgr->OnResize(mClientWidth, mClientHeight, mDepthStencilView, mRenderTargetView);
}

void GameFrameWork::UpdateScene(float dt)
{
	// 1. 서버에서 갱신된 데이터(몬스터들)를 받는다.
	// 2. 클라이언트가 그 정보로 장면을 그린다.
	// 3. 클라이언트에서 키보드, 마우스 등 이벤트가 발생한다.
	// 3. 클라이언트가 그에 따라 갱신된 데이터를 서버로 보낸다.
	// 4. 서버는 각 클라이언트에서 받은 정보를 동기화한다.

	Camera::GetInstance()->Update();

	Object_Mgr->Update(dt);
	Sound_Mgr->Set3DLinstenerPos(Camera::GetInstance()->GetPosition().x, Camera::GetInstance()->GetPosition().y, Camera::GetInstance()->GetPosition().z);
	//Sound_Mgr->Set3DLinstenerPos(Player::GetInstance()->GetPos().x, Player::GetInstance()->GetPos().y, Player::GetInstance()->GetPos().z);
	Scene_Mgr->Update(dt);
	G_State_Mgr->Update();
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

	if (btnState & MK_LBUTTON) {

		Sound_Mgr->Play3DEffect(Sound_p_shout, Player::GetInstance()->GetPos().x, Player::GetInstance()->GetPos().y, Player::GetInstance()->GetPos().z);

		Sound_Mgr->Play3DEffect(Sound_p_swing, Player::GetInstance()->GetPos().x, Player::GetInstance()->GetPos().y, Player::GetInstance()->GetPos().z);
		Player::GetInstance()->ChangeActionState(ActionState::Attack);
		Sound_Mgr->Play3DEffect(Sound_p_swing, Player::GetInstance()->GetPos().x, Player::GetInstance()->GetPos().y, Player::GetInstance()->GetPos().z);
		Sound_Mgr->Play3DEffect(Sound_p_shout, Player::GetInstance()->GetPos().x, Player::GetInstance()->GetPos().y, Player::GetInstance()->GetPos().z);
		auto monsters = Object_Mgr->GetMonsters();
		const XMFLOAT3 p_pos = Player::GetInstance()->GetPos();
		const XMFLOAT3 p_front = Player::GetInstance()->GetLook()*-3.0f;
		for (auto& m : monsters)
		{
			if (MathHelper::DistanceVector(p_pos+p_front, m->GetPos()) <= 3.5f) {
				Player::GetInstance()->Attack(m);
			}
		}

		CS_Attack packet;
		packet.Mon_Num = monsters.size();
		for (UINT i = 0; i < monsters.size(); ++i)
			packet.Mon_HP[i] = monsters[i]->GetProperty().hp_now;

		Packet_Mgr->SendPacket(packet);

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
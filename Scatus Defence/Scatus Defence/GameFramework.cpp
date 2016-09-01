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
	/*mSwapChain->GetBuffer(0, __uuidof(IDXGISurface), (LPVOID*)&UI_Mgr->m_backbuffer);
	UI_Mgr->CreateD2DrenderTarget(D3DApp::MainWnd());*/
	Sound_Mgr->Create_Sound(D3DApp::MainWnd());

	// Giljune's Code
	Packet_Mgr->Init();

	Scene_Mgr->ComputeSceneBoundingBox();

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
	
	if (Packet_Mgr->PacketReceived) {
		ProcessPacket(Packet_Mgr->PacketBuf);
		Packet_Mgr->PacketReceived = false;
	}

	Object_Mgr->Update(dt);

	Camera::GetInstance()->Update();
	Scene_Mgr->Update(dt);
}

void GameFrameWork::ProcessPacket(char * packet)
{
	int& clientID = Packet_Mgr->ClientID;
	HEADER *header = reinterpret_cast<HEADER*>(packet);
	switch (header->Type)
	{
	case eSC::InitPlayer: {
		auto *p = reinterpret_cast<SC_InitPlayer*>(packet);

		SO_InitDesc desc;
		if (clientID == -1) {
			clientID = p->ClientID;

			desc.Pos = p->Player[clientID].Pos;
			desc.Rot = p->Player[clientID].Rot;
			desc.Scale = p->Player[clientID].Scale;
			desc.AttackSpeed = p->Player[clientID].AttackSpeed;
			desc.MoveSpeed = p->Player[clientID].MoveSpeed;
			desc.Hp = p->Player[clientID].Hp;

			auto type = p->Player[clientID].ObjectType;
			Player::GetInstance()->Init(Resource_Mgr->GetSkinnedMesh(type), desc);

			Object_Mgr->AddMainPlayer(Player::GetInstance(), clientID);
			Packet_Mgr->Connected[clientID] = true;
		}

		for (UINT i = 0; i < p->CurrPlayerNum; ++i)
		{
			if (Packet_Mgr->Connected[i])
				continue;

			desc.ObjectType = p->Player[i].ObjectType;
			desc.Pos = p->Player[i].Pos;
			desc.Rot = p->Player[i].Rot;
			desc.Scale = p->Player[i].Scale;
			desc.AttackSpeed = p->Player[i].AttackSpeed;
			desc.MoveSpeed = p->Player[i].MoveSpeed;
			desc.Hp = p->Player[i].Hp;

			Object_Mgr->AddOtherPlayer(desc, i);
			Packet_Mgr->Connected[i] = true;
			
		}

		for (UINT i = 0; i < p->NumOfObjects; ++i)
		{
			desc.Pos = p->MapInfo[i].Pos;
			desc.Rot = p->MapInfo[i].Rot;
			desc.Scale = p->MapInfo[i].Scale;
			auto type = p->MapInfo[i].ObjectType;

			Object_Mgr->AddObstacle(type, desc);
		}

		std::cout << "SC_INIT_PLAYER, ID : " << clientID << std::endl;
		break;
	}

	case eSC::PerFrame: {
		auto *p = reinterpret_cast<SC_PerFrame*>(packet);
		for (UINT i = 0; i < MAX_USER; ++i) {
			if(Packet_Mgr->Connected[i] && i != Packet_Mgr->ClientID)
				Object_Mgr->Update(i, p->Players[i]);
		}

		char* string;
		switch (p->GameState)
		{
		case eGameState::WaveStart:
			string = "WaveStart";

			break;
		case eGameState::WaveWaiting:
			Time_Mgr->gamestate = game_waiting_wave;
			string = "WaveWaiting";
			break;
		case eGameState::Waving:
			Time_Mgr->gamestate = game_waving;
			string = "Waving";
			break;
		case eGameState::GameWaiting:
			string = "GameWaiting";
			break;
		}
		std::cout << "[SC_PerFrame] CurrState : " << string <<
			", ObjectNum : " << p->NumOfObjects << std::endl;
		Time_Mgr->Set_Wavelevel(p->Roundlevel);
		Time_Mgr->Set_time(p->Time);
		break;
	}
	case eSC::AddMonsters: {
		auto *p = reinterpret_cast<SC_AddMonster*>(packet);
		for (UINT i = 0; i < p->NumOfObjects; ++i)
		{
			SO_InitDesc desc;
			desc.Pos = p->InitInfos[i].Pos;
			desc.Rot = p->InitInfos[i].Rot;
			desc.Scale = p->InitInfos[i].Scale;
			desc.AttackSpeed = p->InitInfos[i].AttackSpeed;
			desc.MoveSpeed = p->InitInfos[i].MoveSpeed;
			auto type = p->InitInfos[i].ObjectType;

			Object_Mgr->AddMonster(type, desc, i);
		}
		std::cout << "SC_ADD_MONSTER, ObjectNum : " << p->NumOfObjects << std::endl;
		break;
	}

	default:
		std::cout << "Unknown packet type : " << header->Type << std::endl;
	}
}

void GameFrameWork::DrawScene()
{
	Scene_Mgr->DrawScene();
	//UI_Mgr->Print_All_UI();
	HR(mSwapChain->Present(0, 0));
}

void GameFrameWork::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;
	if (btnState & MK_LBUTTON) {
		Player::GetInstance()->SetAttackState();

		CS_Attack packet;
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
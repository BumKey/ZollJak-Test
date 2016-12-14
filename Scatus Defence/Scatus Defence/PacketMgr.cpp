#include "PacketMgr.h"

PacketMgr::PacketMgr() : ClientID(-1)
{
	mSendBuf.buf = new char[MAX_BUFF_SIZE];
	mSendBuf.len = MAX_BUFF_SIZE;
	mRecvBuf.buf = new char[MAX_BUFF_SIZE];
	mRecvBuf.len = MAX_BUFF_SIZE;

	mPacketBuf = new char[MAX_PACKET_SIZE];

	for (auto& d : Connected)
		d = false;
}

PacketMgr::~PacketMgr()
{
	delete[] mSendBuf.buf;
	delete[] mRecvBuf.buf;

	closesocket(mSocket);
	WSACleanup();
}

void PacketMgr::Init(HWND mainHwnd)
{
	WSADATA	wsaData;
	SOCKADDR_IN recv_addr;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		err_display(L"WSAStartup() Error");

	// WSASocket()
	mSocket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (mSocket == INVALID_SOCKET)
		err_display(L"WSASocket() Error");

	memset(&recv_addr, 0, sizeof(recv_addr));
	recv_addr.sin_family = AF_INET;
	recv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	recv_addr.sin_port = htons(SERVER_PORT);  

	// connect()
	if (connect(mSocket, (SOCKADDR*)&recv_addr, sizeof(recv_addr)) == SOCKET_ERROR)
		err_display(L"connect() Error");

	WSAAsyncSelect(mSocket, mainHwnd, WM_SOCKET, FD_CLOSE | FD_READ);

	DEBUG_MSG("Server Connect Success");
}

void PacketMgr::ReadPacket(SOCKET socket)
{
	static UINT savedBytes = 0;
	DWORD receivedBytes, ioflag = 0;

	if (WSARecv(socket, &mRecvBuf, 1, &receivedBytes, &ioflag, NULL, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
			Packet_Mgr->err_display(L"WSARecv() Error");
	}

	if (receivedBytes == 0)
		DEBUG_MSG("receivedData == 0 ");

	while (1)
	{
		auto header = reinterpret_cast<HEADER*>(mRecvBuf.buf);

		UINT tableNum(0);
		UINT currPacketSize = header->Size;

		if (currPacketSize >= MAX_PACKET_SIZE)
			break;

		if (receivedBytes + savedBytes >= currPacketSize)
		{
			UINT remainingData = currPacketSize - savedBytes;
			// mPacketBuf의 사이즈보다 큰 패킷을 memcpy하면 메모리 오염!!
			memcpy(mPacketBuf + savedBytes, mRecvBuf.buf, remainingData);
			ProcessPacket();
			savedBytes = 0;
			break;
		}
		else
		{
			memcpy(mPacketBuf + savedBytes, mRecvBuf.buf, receivedBytes);
			savedBytes += receivedBytes;
		}
	}
}

void PacketMgr::ProcessPacket()
{
	int& ClientID = Packet_Mgr->ClientID;
	
	HEADER *header = reinterpret_cast<HEADER*>(mPacketBuf);
	switch (header->Type)
	{
	case eSC::InitPlayer: {
		auto *p = reinterpret_cast<SC_InitPlayer*>(mPacketBuf);

		SO_InitDesc desc;
		if (ClientID == -1) {
			ClientID = p->ClientID;

			desc.Pos = p->Player[ClientID].Pos;
			desc.Rot = p->Player[ClientID].Rot;
			desc.Scale = p->Player[ClientID].Scale;
			desc.AttackSpeed = p->Player[ClientID].AttackSpeed;
			desc.AttackPoint = p->Player[ClientID].AttackPoint;
			desc.MoveSpeed = p->Player[ClientID].MoveSpeed;
			desc.Hp = p->Player[ClientID].Hp;

			auto type = p->Player[ClientID].ObjectType;
			Player::GetInstance()->Init(Resource_Mgr->GetSkinnedMesh(type), desc);

			Object_Mgr->AddMainPlayer(Player::GetInstance(), ClientID);
			Packet_Mgr->Connected[ClientID] = true;
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

		DEBUG_MSG("SC_INIT_PLAYER, ID : " << (int)p->ClientID);

		break;
	}
	case eSC::RemovePlayer: {
		auto *p = reinterpret_cast<SC_RemovePlayer*>(mPacketBuf);
		Object_Mgr->RemovePlayer(p->ClientID);
		break;
	}
	case eSC::PlayerInfo: {
		auto *p = reinterpret_cast<SC_PlayerInfo*>(mPacketBuf);
		for (UINT i = 0; i < MAX_USER; ++i) {
			if (Packet_Mgr->Connected[i] && i != Packet_Mgr->ClientID)
				Object_Mgr->UpdatePlayer(i, p->Players[i]);
		}
		break;
	}
	case eSC::MonInfo: {
		auto *p = reinterpret_cast<SC_MonInfo*>(mPacketBuf);
		if (p->NumOfMonsters > 0 && p->NumOfMonsters != Object_Mgr->GetMonsters().size())
		{
			CS_ReAddMonsters packet;
			SendPacket(packet);
		}
		else
		{
			for (UINT i = 0; i < p->NumOfMonsters; ++i) {
				Object_Mgr->UpdateMonster(i, p->Monsters[i]);

				for (UINT c = 0; c < COLL_OBJ_NUM; ++c)
					Collision_Mgr->MonCollPos[i][c] = p->Coll_Ob_Mon[i][c];
			}
		}

		DEBUG_MSG("[MonInfo] : " << p->NumOfMonsters);
		break;
	}
	case eSC::CollisionInfo: {
		auto *p = reinterpret_cast<SC_CollisionInfo*>(mPacketBuf);
		for (UINT i = 0; i < COLL_OBJ_NUM; ++i)
			Collision_Mgr->PlayerCollPos[i] = p->CollisionPos[i];
		break;
	}
	case eSC::FrameInfo: {
		auto *p = reinterpret_cast<SC_FrameInfo*>(mPacketBuf);

		if (p->NumOfPlayers != Object_Mgr->GetCurrPlayerNum()) {
			CS_RePutPlayers packet;
			SendPacket(packet);
		}

		char* string;
		switch (p->GameState)
		{
		case eGameState::WaveStart:
			string = "WaveStart";
			break;
		case eGameState::WaveWaiting:
			Time_Mgr->gamestate = game_waiting_wave;
			Object_Mgr->KillAllMonsteres();
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
		
		DEBUG_MSG("[SC_PerFrame] CurrState : " << string << ", PlayerNum : " << p->NumOfPlayers);
		
		Time_Mgr->Set_Wavelevel(p->Roundlevel);
		Time_Mgr->Set_time(p->Time);
		break;
	}
	case eSC::AddMonsters: {
		auto *p = reinterpret_cast<SC_AddMonster*>(mPacketBuf);
		
		Object_Mgr->ReleaseMonsters();

		for (UINT i = 0; i < p->NumOfObjects; ++i)
		{
			SO_InitDesc desc;
			desc.Pos = p->InitInfos[i].Pos;
			desc.Rot = p->InitInfos[i].Rot;
			desc.Scale = p->InitInfos[i].Scale;
			desc.AttackSpeed = p->InitInfos[i].AttackSpeed;
			desc.MoveSpeed = p->InitInfos[i].MoveSpeed;
			desc.Hp = p->InitInfos[i].Hp;
			desc.AttackPoint = p->InitInfos[i].AttackPoint;
			auto type = p->InitInfos[i].ObjectType;

			Object_Mgr->AddMonster(type, desc, i);
		}
		
		DEBUG_MSG("SC_ADD_MONSTER, ObjectNum : " << p->NumOfObjects);
		break;
	}
	case eSC::ReleaseMons: {
		DEBUG_MSG("SC_ReleaseMons");
		break;
	}

	default:
		DEBUG_MSG("Unknown packet type : " << header->Type);
	}
}

void PacketMgr::err_display(wchar_t *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

#include "PacketMgr.h"

PacketMgr::PacketMgr() : mClientID(-1)
{
	mSendBuf.buf = new char[MAX_BUFF_SIZE];
	mSendBuf.len = MAX_BUFF_SIZE;
	mRecvBuf.buf = new char[MAX_BUFF_SIZE];
	mRecvBuf.len = MAX_BUFF_SIZE;

	mPacketBuf = new char[MAX_PACKET_SIZE];

	for (UINT i = 0; i < MAX_USER; ++i)
		mConnected[i] =  false;
}

PacketMgr::~PacketMgr()
{
	delete[] mSendBuf.buf;
	delete[] mRecvBuf.buf;
	delete[] mPacketBuf;

	closesocket(mSocket);
	WSACleanup();
}

void PacketMgr::Init()
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
	recv_addr.sin_addr.s_addr = inet_addr("192.168.2.114");
	recv_addr.sin_port = htons(SERVER_PORT);

	// connect()
	if (connect(mSocket, (SOCKADDR*)&recv_addr, sizeof(recv_addr)) == SOCKET_ERROR)
		err_display(L"connect() Error");

	std::cout << "Server Connect Success" << std::endl;
}

void PacketMgr::Update()
{
	if (ReadPacket())
	{
		CS_Success packet;
		SendPacket(packet);
	}
}

bool PacketMgr::ReadPacket()
{
	DWORD receivedBytes, ioflag = 0;
	if(WSARecv(mSocket, &mRecvBuf, 1, &receivedBytes, &ioflag, NULL, NULL)==SOCKET_ERROR)
	{ 
		if (WSAGetLastError() != WSA_IO_PENDING)
			err_display(L"WSARecv() Error");

		return false;
	}
	
	if (receivedBytes == 0)
	{
		std::cout << "receivedData == 0 " << std::endl;
		return false;
	}

	static UINT savedBytes = 0;
	while (1)
	{
		auto header = reinterpret_cast<HEADER*>(mRecvBuf.buf);
		UINT currPacketSize = header->Size;
		if (receivedBytes + savedBytes >= currPacketSize)
		{
			UINT remainingData = currPacketSize - savedBytes;
			// mPacketBuf의 사이즈보다 큰 패킷을 memcpy하면 메모리 오염!!
			memcpy(mPacketBuf + savedBytes, mRecvBuf.buf, remainingData);

			ProcessPacket(mPacketBuf);
			savedBytes = 0;
			break;
		}
		else
		{
			memcpy(mPacketBuf + savedBytes, mRecvBuf.buf, receivedBytes);
			savedBytes += receivedBytes;
		}
	}

	return true;
}

void PacketMgr::ProcessPacket(char* packet)
{
	HEADER *header = reinterpret_cast<HEADER*>(packet);
	switch (header->Type)
	{
	case eSC::InitPlayer: {
		auto *p = reinterpret_cast<SC_InitPlayer*>(packet);

		SO_InitDesc desc;
		if (mClientID == -1) {
			mClientID = p->ClientID;

			desc.Pos = p->Player[mClientID].Pos;
			desc.Rot = p->Player[mClientID].Rot;
			desc.Scale = p->Player[mClientID].Scale;
			desc.AttackSpeed = p->Player[mClientID].AttackSpeed;
			desc.MoveSpeed = p->Player[mClientID].MoveSpeed;
			desc.Hp = p->Player[mClientID].Hp;
			auto type = p->Player[mClientID].ObjectType;

			mConnected[mClientID] = true;
			Player::GetInstance()->Init(Resource_Mgr->GetSkinnedMesh(type), desc);
			Object_Mgr->AddPlayer(Player::GetInstance(), mClientID);
		}

		for (UINT i = 0; i < p->CurrPlayerNum; ++i)
		{
			desc.Pos = p->Player[i].Pos;
			desc.Rot = p->Player[i].Rot;
			desc.Scale = p->Player[i].Scale;
			desc.AttackSpeed = p->Player[i].AttackSpeed;
			desc.MoveSpeed = p->Player[i].MoveSpeed;
			desc.Hp = p->Player[i].Hp;
			auto type = p->Player[i].ObjectType;

			if (mConnected[i] == false) {
				Object_Mgr->AddPlayer(new Warrior(Resource_Mgr->GetSkinnedMesh(type), desc), i);
				mConnected[i] = true;
			}
		}

		for (UINT i = 0; i < p->NumOfObjects; ++i)
		{
			desc.Pos = p->MapInfo[i].Pos;
			desc.Rot = p->MapInfo[i].Rot;
			desc.Scale = p->MapInfo[i].Scale;
			auto type = p->MapInfo[i].ObjectType;

			Object_Mgr->AddObstacle(type, desc);
		}
		
		Scene_Mgr->ComputeSceneBoundingBox();

		std::cout << "SC_INIT_PLAYER, ID : " << mClientID << std::endl;
		break;
	}
	case eSC::PutOtherPlayers: {
		auto *p = reinterpret_cast<SC_PutOtherPlayers*>(packet);
		for (UINT i = 0; i < p->CurrPlayerNum; ++i)
		{
			if (mConnected[i] == false) {
				Object_Mgr->AddPlayer(new Warrior(Resource_Mgr->GetSkinnedMesh(
					p->Player[i].ObjectType), p->Player[i]), i);
				mConnected[i] = true;
			}
		}
		std::cout << "SC_PUT_OTHER_PLAYERS, CurrPlayerNum : " << p->CurrPlayerNum << std::endl;
		break;
	}
	case eSC::PerFrame: {
		auto *p = reinterpret_cast<SC_PerFrame*>(packet);
		for (UINT i = 0; i < MAX_USER; ++i)
			Object_Mgr->Update(i, p->Players[i]);
		
		char* string;
		switch (p->GameState)
		{
		case eGameState::WaveStart:
			string = "WaveStart";
			break;
		case eGameState::WaveWaiting:
			string = "WaveWaiting";
			break;
		case eGameState::Waving:
			string = "Waving";
			break;
		case eGameState::GameWaiting:
			string = "GameWaiting";
			break;
		}
		std::cout << "[SC_PerFrame] CurrState : " << string <<
			", ObjectNum : " << p->NumOfObjects << std::endl;
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
		std::cout << "SC_ADD_MONSTER, ObjectNum : " << p->NumOfObjects<< std::endl;
		break;
	}

	default:
		std::cout << "Unknown packet type : " << header->Type << std::endl;
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

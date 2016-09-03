#include "PacketMgr.h"

PacketMgr::PacketMgr() : ClientID(-1)
{
	SendBuf.buf = new char[MAX_BUFF_SIZE];
	SendBuf.len = MAX_BUFF_SIZE;
	RecvBuf.buf = new char[MAX_BUFF_SIZE];
	RecvBuf.len = MAX_BUFF_SIZE;

	for (auto& d : Connected)
		d = false;
}

PacketMgr::~PacketMgr()
{
	delete[] SendBuf.buf;
	delete[] RecvBuf.buf;

	mWorkerThreads[0]->join();
	mWorkerThreads[1]->join();

	closesocket(Socket);
	WSACleanup();
}

void PacketMgr::Init()
{
	WSADATA	wsaData;
	SOCKADDR_IN recv_addr;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		err_display(L"WSAStartup() Error");

	// WSASocket()
	Socket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (Socket == INVALID_SOCKET)
		err_display(L"WSASocket() Error");

	memset(&recv_addr, 0, sizeof(recv_addr));
	recv_addr.sin_family = AF_INET;
	recv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	recv_addr.sin_port = htons(SERVER_PORT);

	// connect()
	if (connect(Socket, (SOCKADDR*)&recv_addr, sizeof(recv_addr)) == SOCKET_ERROR)
		err_display(L"connect() Error");

	mWorkerThreads[0] = new std::thread(ReadPacket);
	mWorkerThreads[1] = new std::thread(ReadPacket);

	std::cout << "Server Connect Success" << std::endl;
}

void PacketMgr::ReadPacket()
{
	SOCKET& socket = Packet_Mgr->Socket;
	WSABUF& recvBuf = Packet_Mgr->RecvBuf;
	auto& packetTable = Packet_Mgr->PacketTable;

	static UINT savedBytes = 0;
	DWORD receivedBytes, ioflag = 0;

	while (1) {
		if (WSARecv(socket, &recvBuf, 1, &receivedBytes, &ioflag, NULL, NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
				Packet_Mgr->err_display(L"WSARecv() Error");
		}

		if (receivedBytes == 0)
			std::cout << "receivedData == 0 " << std::endl;

		while (1)
		{
			auto header = reinterpret_cast<HEADER*>(recvBuf.buf);

			UINT tableNum(0);
			UINT currPacketSize = header->Size;

			if (header->Type == eSC::PerFrame) {
				for (UINT i = 3; i < MAX_TABLE; ++i)
				{
					if (packetTable[i].On == false) {
						packetTable[i].On = true;
						tableNum = i;
					}
				}
			}
			else {
				for (UINT i = 0; i < 3; ++i) 
				{
					if (packetTable[i].On == false) {
						packetTable[i].On = true;
						tableNum = i;
					}
				}
			}

			if (currPacketSize >= MAX_PACKET_SIZE)
				break;

			if (receivedBytes + savedBytes >= currPacketSize)
			{
				UINT remainingData = currPacketSize - savedBytes;
				// mPacketBuf의 사이즈보다 큰 패킷을 memcpy하면 메모리 오염!!
				memcpy(packetTable[tableNum].buf + savedBytes, recvBuf.buf, remainingData);
				ProcessPacket();
				savedBytes = 0;
				break;
			}
			else
			{
				memcpy(packetTable[tableNum].buf + savedBytes, recvBuf.buf, receivedBytes);
				savedBytes += receivedBytes;
			}
		}
	}
}

void PacketMgr::ProcessPacket()
{
	int& clientID = Packet_Mgr->ClientID;
	auto packetTable = Packet_Mgr->PacketTable;

	char* packet;
	for (UINT i = 0; i < MAX_TABLE; ++i)
	{
		if (packetTable[i].On) {
			packet = packetTable[i].buf;
			packetTable[i].On = false;
			break;
		}
	}

	assert(packet);
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
			if (Packet_Mgr->Connected[i] && i != Packet_Mgr->ClientID)
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

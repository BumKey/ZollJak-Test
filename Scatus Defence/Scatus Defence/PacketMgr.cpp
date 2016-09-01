#include "PacketMgr.h"

PacketMgr::PacketMgr() : ClientID(-1), PacketReceived(false)
{
	SendBuf.buf = new char[MAX_BUFF_SIZE];
	SendBuf.len = MAX_BUFF_SIZE;
	RecvBuf.buf = new char[MAX_BUFF_SIZE];
	RecvBuf.len = MAX_BUFF_SIZE;

	PacketBuf = new char[MAX_PACKET_SIZE];

	for (auto& d : Connected)
		d = false;
}

PacketMgr::~PacketMgr()
{
	delete[] SendBuf.buf;
	delete[] RecvBuf.buf;
	delete[] PacketBuf;

	mWorkerThread->join();

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

	mWorkerThread = new std::thread(ReadPacket);

	std::cout << "Server Connect Success" << std::endl;
}

void PacketMgr::ReadPacket()
{
	SOCKET& socket = Packet_Mgr->Socket;
	WSABUF& recvBuf = Packet_Mgr->RecvBuf;
	char* packetBuf = Packet_Mgr->PacketBuf;

	static UINT savedBytes = 0;
	DWORD receivedBytes, ioflag = 0;

	while (1)
	{
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

			UINT currPacketSize = header->Size;
			if (currPacketSize >= MAX_PACKET_SIZE)
				break;

			if (receivedBytes + savedBytes >= currPacketSize)
			{
				UINT remainingData = currPacketSize - savedBytes;
				// mPacketBuf의 사이즈보다 큰 패킷을 memcpy하면 메모리 오염!!
				memcpy(packetBuf + savedBytes, recvBuf.buf, remainingData);

				Packet_Mgr->PacketReceived = true;
				savedBytes = 0;
				break;
			}
			else
			{
				memcpy(packetBuf + savedBytes, recvBuf.buf, receivedBytes);
				savedBytes += receivedBytes;
			}
		}
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

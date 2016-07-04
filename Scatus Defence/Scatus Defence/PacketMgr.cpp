#include "PacketMgr.h"

PacketMgr::PacketMgr()
{
	mSend_wsabuf.buf = mSend_buffer;
	mSend_wsabuf.len = MAX_BUFF_SIZE;
	mRecv_wsabuf.buf = mRecv_buffer;
	mRecv_wsabuf.len = MAX_BUFF_SIZE;
}

PacketMgr::~PacketMgr()
{
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
	recv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	recv_addr.sin_port = htons(SERVER_PORT);

	// connect()
	if (connect(mSocket, (SOCKADDR*)&recv_addr, sizeof(recv_addr)) == SOCKET_ERROR)
		err_display(L"connect() Error");

	std::cout << "Server Connect Success" << std::endl;
}

void PacketMgr::SendPacket(cs_packet &packet)
{
	unsigned char *packet_buf;
	char buf[MAX_BUFF_SIZE] = { 0, };

	packet.size = sizeof(packet);
	packet_buf = reinterpret_cast<unsigned char*>(&packet);
	memcpy(buf, packet_buf, packet_buf[0]);
	mSend_wsabuf.len = packet.size;
	mSend_wsabuf.buf = buf;

	int outBytes = 0;
	WSABUF temp;
	temp.buf = mSend_wsabuf.buf;
	temp.len = mSend_wsabuf.len;
	if (WSASend(mSocket, &temp, 1, (LPDWORD)&outBytes, 0, NULL, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
			err_display(L"WSASend() Error");
	}
}

void PacketMgr::SendPacket(cs_packet_success &packet)
{
	unsigned char *packet_buf;
	char buf[MAX_BUFF_SIZE] = { 0, };

	packet.size = sizeof(packet);
	packet_buf = reinterpret_cast<unsigned char*>(&packet);
	memcpy(buf, packet_buf, packet_buf[0]);
	mSend_wsabuf.len = packet.size;
	mSend_wsabuf.buf = buf;

	int outBytes = 0;
	WSABUF temp;
	temp.buf = mSend_wsabuf.buf;
	temp.len = mSend_wsabuf.len;
	if (WSASend(mSocket, &temp, 1, (LPDWORD)&outBytes, 0, NULL, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
			err_display(L"WSASend() Error");
	}
}

void PacketMgr::ReadPacket()
{
	DWORD iobytes, ioflag = 0;
	
	WSABUF temp;
	temp.buf = mRecv_wsabuf.buf;
	temp.len = mRecv_wsabuf.len;
	if(WSARecv(mSocket, &temp, 1, &iobytes, &ioflag, NULL, NULL)==SOCKET_ERROR)
	{ 
		if (WSAGetLastError() != WSA_IO_PENDING)
			err_display(L"WSARecv() Error");
	}
	
	BYTE *ptr = reinterpret_cast<BYTE *>(mRecv_buffer);

	while (0 != iobytes)
	{
		if (0 == in_packet_size) in_packet_size = ptr[0];
		if (iobytes + saved_packet_size >= in_packet_size)
		{
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			ProcessPacket(packet_buffer);
			ptr += in_packet_size - saved_packet_size;
			iobytes -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else
		{
			memcpy(packet_buffer + saved_packet_size, ptr, iobytes);
			saved_packet_size += iobytes;
			iobytes = 0;
		}
	}
}

void PacketMgr::ProcessPacket(char* ptr)
{
	switch (ptr[1])
	{
	case SC_PUT_PLAYER:
		std::cout << "packet type : " << (int)ptr[1] << std::endl;
		break;
	case SC_POS:
		std::cout << "packet type : " << (int)ptr[1] << std::endl;
		break;
	case SC_REMOVE_PLAYER:
		std::cout << "packet type : " << (int)ptr[1] << std::endl;
		break;
	case CS_TEST:
		std::cout << "packet success" << std::endl;
		break;
	default:
		std::cout << "Unknown packet type : " << (int)ptr[1] << std::endl;
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

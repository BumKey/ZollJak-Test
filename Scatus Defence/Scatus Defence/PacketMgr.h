#pragma once

#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <Windows.h>
#include <assert.h>
#include <iostream>
#include <thread>
#include <vector>

#include "protocol.h"
#include "Singletone.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "RogicTimer.h"
#pragma comment (lib, "ws2_32.lib")

#define Packet_Mgr PacketMgr::GetInstance()
#define MAX_TABLE 10
class PacketMgr : public Singletone<PacketMgr>
{
public:
	PacketMgr();
	~PacketMgr();

public:
	void Init();

	template <class T>
	void SendPacket(T& packet);

	void err_display(wchar_t *msg);

private:
	static void ProcessPacket();
	static void ReadPacket();

public:
	int ClientID;
	bool Connected[MAX_USER];

	SOCKET	Socket;
	WSABUF	SendBuf;
	WSABUF	RecvBuf;
	eGameState CurrGameState;

	struct PacketTable {
		bool On;
		char* buf;
		PacketTable() : On(false) { buf = new char[MAX_PACKET_SIZE]; }
	};

	PacketTable PacketTable[MAX_TABLE];

private:
	std::thread* mWorkerThreads[2];
};

template<class T>
inline void PacketMgr::SendPacket(T & packet)
{
	char *packet_buf;

	packet.ClientID = Packet_Mgr->ClientID;
	packet_buf = reinterpret_cast<char*>(&packet);

	SendBuf.len = packet.Size;
	memcpy(SendBuf.buf, packet_buf, packet.Size);

	int outBytes = 0;
	if (WSASend(Socket, &SendBuf, 1, (LPDWORD)&outBytes, 0, NULL, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
			err_display(L"WSASend() Error");
	}
}

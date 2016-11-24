#pragma once

#define WIN32_LEAN_AND_MEAN
#define WM_SOCKET WM_USER+1
#include <WinSock2.h>
#include <Windows.h>
#include <assert.h>
#include <iostream>
#include <thread>
#include <vector>

#include "protocol.h"
#include "Singletone.h"
#include "ObjectMgr.h"
#include "CollisionMgr.h"
#include "Player.h"
#include "RogicTimer.h"
#pragma comment (lib, "ws2_32.lib")

#define Packet_Mgr PacketMgr::GetInstance()

class PacketMgr : public Singletone<PacketMgr>
{
public:
	PacketMgr();
	~PacketMgr();

public:
	void Init(HWND mainHwnd);

	template <class T>
	void SendPacket(T& packet);

	void err_display(wchar_t *msg);

	void ProcessPacket();
	void ReadPacket(SOCKET socket);

public:
	int ClientID;
	bool Connected[MAX_USER];

private:
	SOCKET	mSocket;
	WSABUF	mSendBuf;
	WSABUF	mRecvBuf;
	eGameState mCurrGameState;

	char* mPacketBuf;
};

template<class T>
inline void PacketMgr::SendPacket(T & packet)
{
	char *packet_buf;

	packet.ClientID = ClientID;
	packet_buf = reinterpret_cast<char*>(&packet);

	mSendBuf.len = packet.Size;
	memcpy(mSendBuf.buf, packet_buf, packet.Size);

	int outBytes = 0;
	if (WSASend(mSocket, &mSendBuf, 1, (LPDWORD)&outBytes, 0, NULL, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
			err_display(L"WSASend() Error");
	}
}

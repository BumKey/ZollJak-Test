#pragma once

#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <Windows.h>
#include <assert.h>
#include "protocol.h"
#include "Singletone.h"
#include "ObjectMgr.h"
#include "SceneMgr.h"
#include "Warrior.h"
#include <iostream>
#include <vector>

#pragma comment (lib, "ws2_32.lib")

#define Packet_Mgr PacketMgr::GetInstance()

class PacketMgr : public Singletone<PacketMgr>
{
public:
	PacketMgr();
	~PacketMgr();

public:
	void Init();
	void Update();

	template <class T>
	void SendPacket(T& packet);
	char* GetPacket()				{ return mPacketBuf; }
	int GetClientID() const { return mClientID; }
private:
	bool ReadPacket();
	void ProcessPacket(char* packet);

	void err_display(wchar_t *msg);

private:
	int mClientID;
	bool mConnected[MAX_USER];
	bool mSended;

	SOCKET	mSocket;
	WSABUF	mRecvBuf;
	WSABUF	mSendBuf;
	eGameState mCurrGameState;

	char* mPacketBuf;
};

template<class T>
inline void PacketMgr::SendPacket(T & packet)
{
	char *packet_buf;

	packet.ClientID = mClientID;
	packet_buf = reinterpret_cast<char*>(&packet);

	mSendBuf.len = packet.Size;
	memcpy(mSendBuf.buf, packet_buf, packet.Size);

	if (mSended == false) {
		int outBytes = 0;
		if (WSASend(mSocket, &mSendBuf, 1, (LPDWORD)&outBytes, 0, NULL, NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
				err_display(L"WSASend() Error");
		}
	}
	mSended = true;
}

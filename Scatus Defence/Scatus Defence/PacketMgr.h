#pragma once

#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <Windows.h>
#include <assert.h>
#include "protocol.h"
#include "Singletone.h"
#include <iostream>
#include <vector>

#pragma comment (lib, "ws2_32.lib")

#define Packet_Mgr PacketMgr::GetInstance()

struct WSABuf {
	ULONG len;
	CHAR* buf;
};


class PacketMgr : public Singletone<PacketMgr>
{
public:
	PacketMgr();
	~PacketMgr();

public:
	void Init();

	void SendPacket(cs_packet_move& packet);
	void SendPacket(cs_packet_attack& packet);
	void SendPacket(cs_packet_success& packet);
	bool ReadPacket();
	void ProcessPacket(char* ptr);

private:
	void err_display(wchar_t *msg);

private:
	int mClientID;

	SOCKET	mSocket;
	WSABuf	mRecv_wsabuf;
	char	mSend_buffer[MAX_BUFF_SIZE];
	WSABuf	mSend_wsabuf;
	char	mRecv_buffer[MAX_BUFF_SIZE];
	char	packet_buffer[MAX_PACKET_SIZE];
	DWORD	in_packet_size = 0;
	int		saved_packet_size = 0;
};


#pragma once
#include "protocol.h"
#include "Singletone.h"
#include <vector>

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

	void SendPacket(cs_packet& packet);
	void SendPacket(cs_packet_success& packet);
	void ReadPacket();
	void ProcessPacket(char* ptr);

private:
	void err_display(wchar_t *msg);

private:
	SOCKET	mSocket;
	WSABuf	mRecv_wsabuf;
	char	mSend_buffer[MAX_BUFF_SIZE];
	WSABuf	mSend_wsabuf;
	char	mRecv_buffer[MAX_BUFF_SIZE];
	char	packet_buffer[MAX_PACKET_SIZE];
	DWORD	in_packet_size = 0;
	int		saved_packet_size = 0;
	int		my_id;
};


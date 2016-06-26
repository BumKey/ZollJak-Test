#pragma once

#include "stdafx.h"
#include "ObjectMgr.h"
#include "CollisionMgr.h"

struct Overlap_ex
{
	WSAOVERLAPPED original_Overlap;
	int operation; // OP_SEND AND OP_RECV
	WSABUF wsabuf;
	unsigned char iocp_buffer[MAX_BUFF_SIZE];
};

struct Player {
	XMFLOAT3 pos;
	XMFLOAT3 rot;
	FLOAT scale;
};

struct Client {
	SOCKET s;
	bool is_connected;
	Player avatar;
	Overlap_ex recv_overlap;
	int packet_size;
	int previous_size;
	unsigned char packet_buff[MAX_PACKET_SIZE];
};

// server class
class Server
{
public:
	Server();
	~Server();

	void Initialize();
	void Accept_Thread();
	void Worker_Thread();
	void Send_Packet(DWORD id, unsigned char *packet);
	void Process_Packet(DWORD id, unsigned char buf[]);

private:
	WSADATA wsadata;
	static HANDLE g_hIocp;
	static BOOL g_isshutdown;

	static Client clients[MAX_OBJECT];

	std::vector <std::thread*> worker_threads;
	std::thread* accept_threads;

	ObjectMgr mObjectMgr;
};

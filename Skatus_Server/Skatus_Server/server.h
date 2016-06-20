#pragma once

#include "stdafx.h"

struct Overlap_ex
{
	WSAOVERLAPPED original_Overlap;
	int operation; // OP_SEND AND OP_RECV
	WSABUF wsabuf;
	unsigned char iocp_buffer[MAX_BUFF_SIZE];
};

struct Player {
	XMFLOAT3 pos;
	FLOAT rot;
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
	WSADATA wsadata;
	static HANDLE g_hIocp;
	static BOOL g_isshutdown;

	static Client clients[MAX_OBJECT];

	std::vector <std::thread*> worker_threads;
	std::thread* accept_threads;

public:
	Server();
	~Server();

	static void Initialize();
	static void Accept_Thread();
	static void Worker_Thread();
	static void Send_Packet(DWORD id, unsigned char *packet);
	static void Process_Packet(DWORD id, unsigned char buf[]);
};

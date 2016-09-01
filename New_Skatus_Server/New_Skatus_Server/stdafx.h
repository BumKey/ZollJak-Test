#pragma once
#pragma warning (disable : 4996)

#include <WinSock2.h>
#include <Windows.h>

#include <vector>
#include <thread>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <assert.h>
#include <mutex>

#include "protocol.h"

#include "MathHelper.h"
#include "Utility.h"

#include "GameTimer.h"


using namespace std;

/* 
	SERVER_STRUCTURE
*/

struct Overlap_ex {
	WSAOVERLAPPED original_overlap;
	int operation;
	WSABUF wsabuf;
	unsigned char iocp_buffer[MAX_BUFF_SIZE];
};

struct Player {
	int x;
	int y;
};

struct Client {
	SOCKET s;
	bool is_connected;
	unordered_set <int> view_list;
	mutex vl_lock;
	SO_InitDesc avatar;
	Overlap_ex recv_overlap;
	int	packet_size;
	int previous_size;
	unsigned char packet_buff[MAX_PACKET_SIZE];
};


/*
	extern variable
*/

extern Client clients[MAX_USER];
extern HANDLE g_hIocp;
extern bool g_isshutdown;

/*
	extern function
*/

void SendPacket(int id, unsigned char *packet);
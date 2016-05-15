#pragma once
#pragma warning(disable : 4996)

#include <WinSock2.h>
#include <Windows.h>

#include <vector>
#include <thread>
#include <iostream>
#include <unordered_set>
#include <mutex>

#include "Protocol.h"

#define NUM_THREAD 6

#define OP_RECV 1
#define OP_SEND 2

using namespace std;

struct Overlap_ex {
	WSAOVERLAPPED original_overlap;
	int operation;
	WSABUF wsabuf;
	unsigned char iocp_buff[MAX_BUFF_SIZE]; // buff size : 4000
};

struct Player {
	int x;
	int y;
};

struct Client {
	SOCKET sock;
	bool is_connected;
	unordered_set <int> view_list;
	mutex vl_lock; // view_list lock
	Player avatar;
	Overlap_ex recv_overlap;
	int packet_size;
	int previous_size;
	unsigned char packet_buff[MAX_PACKET_SIZE];
};
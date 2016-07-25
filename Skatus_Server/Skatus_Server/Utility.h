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
	ObjectInfo avatar;
	Overlap_ex recv_overlap;
	int packet_size;
	int previous_size;
	unsigned char packet_buff[MAX_PACKET_SIZE];
};

void error_display(char *msg, int err_no);

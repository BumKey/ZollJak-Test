#pragma once
#include "stdafx.h"

XMFLOAT4 operator+(XMFLOAT4 l, XMFLOAT4 r);

XMFLOAT3 operator+(XMFLOAT3 l, XMFLOAT3 r);
XMFLOAT3 operator-(XMFLOAT3 l, XMFLOAT3 r);
XMFLOAT3 operator/(XMFLOAT3 l, XMFLOAT3 r);
XMFLOAT3 operator*(XMFLOAT3 l, FLOAT r);
XMFLOAT3 operator/(XMFLOAT3 l, FLOAT r);
bool operator==(XMFLOAT3 l, XMFLOAT3 r);


XMFLOAT2 operator+(XMFLOAT2 l, XMFLOAT2 r);
XMFLOAT2 operator-(XMFLOAT2 l, XMFLOAT2 r);
bool operator==(XMFLOAT2 l, XMFLOAT2 r);
bool operator!=(XMFLOAT2 l, XMFLOAT2 r);

XMFLOAT4 Float4Normalize(const XMFLOAT4& in);
XMFLOAT3 Float3Normalize(const XMFLOAT3& in);
XMFLOAT2 Float2Normalize(const XMFLOAT2& in);

struct Overlap_ex
{
	WSAOVERLAPPED original_Overlap;
	int operation; // OP_SEND AND OP_RECV
	WSABUF wsabuf;
	char iocp_buffer[MAX_BUFF_SIZE];
};

struct Client {
	SOCKET socket;
	bool is_connected;
	SO_InitDesc avatar;
	Overlap_ex recv_overlap;
	int packet_size;
	int previous_size;
	char packet_buff[MAX_PACKET_SIZE];
};

void error_display(char *msg, int err_no);

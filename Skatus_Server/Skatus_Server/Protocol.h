#pragma once

#include "stdafx.h"

// server -> client
#define SC_POS				1
#define SC_PUT_PLAYER		2
#define SC_REMOVE_PLAYER	3

// client -> server
#define CS_TEST				0
#define CS_UP				1
#define CS_DOWN				2
#define CS_LEFT				3
#define CS_RIGHT			4
#define CS_UP_LEFT			5
#define CS_UP_RIGHT			6
#define CS_DOWN_LEFT		7
#define CS_DOWN_RIGHT		8
#define CS_SUCCESS			9

#define SERVER_PORT			4000
#define MAX_BUFF_SIZE		4000
#define MAX_PACKET_SIZE		255
#define MAX_USER			10
#define MAX_MONSTER			100
#define MAX_OBJECT			1000
#define MAX_NPC				100
#define WORLDSIZE			100
#define MONSTER_DURATION	1000

#define OP_RECV				1
#define OP_SEND				2

#pragma pack(push, 1)


// server -> client
struct ForClientInfo
{
	XMFLOAT3 pos;
	FLOAT rot;
	FLOAT scale;
};

struct sc_packet_pos
{
	BYTE size;
	BYTE type;
	DWORD client_id;

	ForClientInfo cInfo;
};

struct sc_packet_put_player
{
	BYTE size;
	BYTE type;
	DWORD client_id;

	ForClientInfo cInfo;
};

struct sc_packet_remove_player
{
	BYTE size;
	BYTE type;
	DWORD client_id;
};

// client -> server
struct cs_packet
{
	BYTE size;
	BYTE type;
	ForClientInfo cInfo;
};

struct cs_packet_success
{
	BYTE size;
	BYTE type;
	bool success;
};

#pragma pack(pop)
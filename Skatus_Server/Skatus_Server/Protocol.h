#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>
#include <d3dx11.h>
#include <xnamath.h>
#include <unordered_map>

#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dx11.lib")
#pragma comment (lib, "d3dx10.lib")

// server -> client
#define SC_POS				1
#define SC_PUT_PLAYER		2
#define SC_REMOVE_PLAYER	3
#define SC_INIT				4

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

namespace AttackType {
	enum Types
	{
		type_shortdistance,
		type_longdistance
	};
}

namespace ActionState {
	enum States
	{
		Idle,
		Battle,
		Walk,
		Run,
		Die,
		Build,
		Attack,
		Damage
	};
}

namespace AI_State {
	enum States
	{
		None,
		MovingToTarget,
		AttackToTarget
	};
}

namespace CollisionState {
	enum States
	{
		None,
		MovingCollision,
		AttackCollision
	};
}

namespace ObjectType {
	enum Types
	{
		None = 0,

		Player,
		Warrior, // 캐릭터 전사
		Archer, // 캐릭터 아처
		Builder, // 캐릭터 건축가

		Monster,
		Goblin, // 적- 고블린
		Cyclop,

		Obstacle,
		Tree,
		Base,
		Stairs,
		Pillar1,
		Pillar2,
		Pillar3,
		Pillar4,
		Rock,
		Temple
	};
}

struct InstanceDesc
{
	XMFLOAT3 Pos;
	XMFLOAT3 Rot;
	FLOAT Scale;

	InstanceDesc() {
		Pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		Rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		Scale = 1.0f;
	}
};

// server -> client
struct ForClientInfo
{
	ObjectType::Types oType;
	XMFLOAT3 pos;
	XMFLOAT3 rot;
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

struct sc_packetForClient_Init
{
	BYTE size;
	BYTE type;
	std::unordered_map<ObjectType::Types, InstanceDesc> data;
};

// packet for rendering in client.
struct packetForClient
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
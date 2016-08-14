#pragma once
#include <d3dx11.h>
#include <xnamath.h>
#include <vector>

// server -> client
#define SC_POS				1
#define SC_PUT_PLAYER		2
#define SC_REMOVE_PLAYER	3
#define SC_PER_FRAME		4

// client -> server
#define CS_KEYINPUT			0
#define CS_MOUSEINPUT		1
//#define CS_UP				1
//#define CS_DOWN				2
//#define CS_LEFT				3
//#define CS_RIGHT			4
//#define CS_UP_LEFT			5
//#define CS_UP_RIGHT			6
//#define CS_DOWN_LEFT		7
//#define CS_DOWN_RIGHT		8
#define CS_SUCCESS			9

#define SERVER_PORT			4000
#define MAX_BUFF_SIZE		4000
#define MAX_PACKET_SIZE		255
#define MAX_USER			1
#define MAX_MONSTER			100
#define MAX_OBJECT			1000
#define MAX_NPC				100
#define WORLDSIZE			100
#define MONSTER_DURATION	1000

#define OP_RECV				1
#define OP_SEND				2

#define PI					3.1415926535f
#pragma pack(push, 1)

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

namespace ActionState {
	enum States
	{
		Idle,
		Attack,
		Walk,
		Run,
		Die,
		Build,
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
enum eGameState {
	GameWaiting,
	WaveWaiting,
	WaveStart,
	Waving,
	GameOver
};

struct ObjectInitInfo
{


};

struct ObjectInfo
{
	BYTE ActionState;
	BYTE ObjectType;

	DWORD Hp;

	FLOAT AttackSpeed;
	FLOAT MoveSpeed;

	XMFLOAT3 Pos;
	XMFLOAT3 Rot;
	FLOAT Scale;

};

// server -> client
struct sc_packet_PerFrame
{
	BYTE size;
	BYTE type;
	DWORD time;

	std::vector<ObjectInfo> cInfos;
};

struct sc_packet_put_player
{
	BYTE size;
	BYTE type;
	DWORD client_id;

	ObjectInfo cInfo;
};

struct sc_packet_remove_player
{
	BYTE size;
	BYTE type;
	DWORD client_id;
};

// client -> server
struct cs_packet_move
{
	BYTE size;
	BYTE type;
	DWORD client_id;

	XMFLOAT3 pos;
};

struct cs_packet_attack
{
	BYTE size;
	BYTE type;
	DWORD client_id;
};

struct cs_packet_success
{
	BYTE size;
	BYTE type;
};

#pragma pack(pop)
#pragma once
#include <d3dx11.h>
#include <xnamath.h>
#include <vector>

#define SERVER_PORT			4000
#define MAX_BUFF_SIZE		4000
#define MAX_PACKET_SIZE		4000
#define MAX_USER			2
#define MAX_MONSTER			50
#define MAX_OBJECT			100
#define MAX_NPC				100
#define WORLDSIZE			100
#define MONSTER_DURATION	1000

#define OP_RECV				1
#define OP_SEND				2

#define PI					3.1415926535f
#pragma pack(push, 1)

// server -> client
enum eSC {
	InitPlayer,
	PutOtherPlayers,
	AddMonsters,
	PerFrame
};

enum eCS {
	KeyInput,
	MouseInput
};

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

enum eMonsterTarget {
	Temple,
	Player0,
	Player1,
	Player2
};

enum eGameState {
	GameWaiting,
	WaveWaiting,
	WaveStart,
	Waving,
	GameOver
};

struct BO_InitDesc
{
	BO_InitDesc() : Pos(0.0f, 0.0f, 0.0f), Rot(0.0f, 0.0f, 0.0f), Scale(0.0f)
	{
		ObjectType = ObjectType::None;
	}
	ObjectType::Types ObjectType;

	XMFLOAT3 Pos;
	XMFLOAT3 Rot;
	FLOAT Scale;
};

struct SO_InitDesc : public BO_InitDesc
{
	SO_InitDesc() : BO_InitDesc(), Hp(100), AttackSpeed(0.0f), MoveSpeed(0.0f)
	{
		ActionState = ActionState::Idle;
	}
	int Hp;
	ActionState::States ActionState;

	FLOAT AttackPoint;
	FLOAT AttackSpeed;
	FLOAT MoveSpeed;
};

struct ObjectInfo
{
	ActionState::States ActionState;
	XMFLOAT3 Pos;
	XMFLOAT3 Rot;
};

struct HEADER
{
	UINT Size;
	DWORD Type;
};

// server -> client
struct SC_PerFrame : public HEADER
{
	SC_PerFrame() {
		Size = sizeof(*this); Type = eSC::PerFrame;
	}
	eGameState GameState;
	UINT Time;
	UINT NumOfObjects;
	UINT Roundlevel;
	ObjectInfo Players[MAX_USER];
	ObjectInfo Monsters[MAX_MONSTER];
	eMonsterTarget Target[MAX_MONSTER];
};

struct SC_AddMonster : public HEADER
{
	SC_AddMonster() {
		Size = sizeof(*this); Type = eSC::AddMonsters;
	}
	UINT NumOfObjects;
	SO_InitDesc InitInfos[MAX_MONSTER];
};

struct SC_InitPlayer : public HEADER
{
	SC_InitPlayer() {
		Size = sizeof(*this); Type = eSC::InitPlayer;
	}

	BYTE ClientID;
	BYTE CurrPlayerNum;
	SO_InitDesc Player[MAX_USER];
	UINT NumOfObjects;
	BO_InitDesc MapInfo[50];
};

struct SC_PutOtherPlayers : public HEADER
{
	SC_PutOtherPlayers() {
		Size = sizeof(*this); Type = eSC::PutOtherPlayers;
	}

	BYTE CurrPlayerNum;
	SO_InitDesc Player[MAX_USER];
};

// client -> server
struct CS_Move : public HEADER
{
	CS_Move() {
		Size = sizeof(*this); Type = eCS::KeyInput;
	}

	BYTE ClientID;
	XMFLOAT3 Pos;
	XMFLOAT3 Rot;
	ActionState::States ActionState;
	FLOAT MoveSpeed;
	FLOAT DeltaTime;
};

struct CS_Attack : public HEADER
{
	CS_Attack() {
		Size = sizeof(*this); Type = eCS::MouseInput;
	}
	BYTE ClientID;
};

#pragma pack(pop) 
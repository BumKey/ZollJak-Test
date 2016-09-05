#pragma once
#include <d3dx11.h>
#include <xnamath.h>
#include <vector>

#define SERVER_PORT			4000
#define MAX_BUFF_SIZE		4000
#define MAX_PACKET_SIZE		4000
#define MAX_USER			1
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
	RemovePlayer,
	PutOtherPlayers,
	AddMonsters,
	ReleaseAllMons,
	PlayerInfo,
	MonInfo
};

enum eCS {
	KeyInput,
	MouseInput,
	Damage
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

struct PlayerInfos
{
	ActionState::States ActionState;
	XMFLOAT3 Pos;
	XMFLOAT3 Rot;
};

struct MonInfos
{
	ActionState::States ActionState;
	BYTE TargetID;
};

struct HEADER
{
	UINT Size;
	DWORD Type;
};

// server -> client
struct SC_PlayerInfo : public HEADER
{
	SC_PlayerInfo() {
		Size = sizeof(*this); Type = eSC::PlayerInfo;
	}
	PlayerInfos Players[MAX_USER];
};

struct SC_MonInfo : public HEADER
{
	SC_MonInfo() {
		Size = sizeof(*this); Type = eSC::MonInfo;
	}
	eGameState GameState;
	UINT Time;
	UINT NumOfObjects;
	UINT Roundlevel;
	MonInfos Monsters[MAX_MONSTER];
};

struct SC_AddMonster : public HEADER
{
	SC_AddMonster() {
		Size = sizeof(*this); Type = eSC::AddMonsters;
	}
	UINT NumOfObjects;
	SO_InitDesc InitInfos[MAX_MONSTER];
};

struct SC_ReleaseAllMonsters : public HEADER
{
	SC_ReleaseAllMonsters() {
		Size = sizeof(*this); Type = eSC::ReleaseAllMons;
	}
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

struct SC_RemovePlayer : public HEADER
{
	SC_RemovePlayer() {
		Size = sizeof(*this); Type = eSC::RemovePlayer;
	}
	BYTE ClientID;
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
	BYTE Mon_Num;
	DWORD Mon_HP[MAX_MONSTER];
};

struct CS_Damage : public HEADER
{
	CS_Damage() {
		Size = sizeof(*this); Type = eCS::Damage;
	}
	BYTE ClientID;
	BYTE MonID;
	BYTE Damage;
};

#pragma pack(pop) 
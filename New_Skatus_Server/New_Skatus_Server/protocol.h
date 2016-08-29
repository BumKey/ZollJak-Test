/*
	Protocol header
*/
#include <D3DX11.h>
#include <xnamath.h>
#include <vector>

#define MAX_BUFF_SIZE		4000
#define MAX_PACKET_SIZE		4000

#define BOARD_WIDTH			100
#define BOARD_HEIGHT		100

#define VIEW_RADIUS			4

#define MAX_USER			10
#define MAX_MONSTER			50

#define NPC_START			500
#define NUM_OF_NPC			3000

#define MY_SERVER_PORT		4000

#define MAX_STR_SIZE		100

#define CS_UP				1
#define CS_DOWN				2
#define CS_LEFT				3
#define CS_RIGHT			4
#define CS_CHAT				5

#define SC_POS				1
#define SC_PUT_PLAYER		2
#define SC_REMOVE_PLAYER	3
#define SC_CHAT				4

#define NUM_THREADS			4

#define OP_RECV				1
#define OP_SEND				2

#define PI					3.1415926535f

#pragma pack (push, 1)

enum eSC {
	InitPlayer,
	PutOtherPlayer,
	AddMonsters,
	PerFrame // 사용할 지, 안할 지는 추후에 결정.
};

enum eCS {
	Success,
	KeyInput,
	MouseInput
};

namespace ObjectType {
	enum Types
	{
		None = 0,
		
		Player,
		Warrior,
		Archer,
		Builder,

		Monster,
		Goblin,
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
	enum States {
		None,
		MovingToTarget,
		AttackToTarget
	};
}

namespace CollisionState {
	enum States {
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

/*
	from server to client
*/

struct SC_PerFrame : public HEADER
{
	SC_PerFrame() {
		Size = sizeof(*this); Type = eSC::PerFrame;
	}
	eGameState GameState;
	UINT Time;
	UINT NumOfObject;
	UINT Roundlevel;
	ObjectInfo Players[MAX_USER];
	ObjectInfo Monsters[MAX_MONSTER];
	eMonsterTarget target[MAX_MONSTER];
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
		Size = sizeof(*this); Type = eSC::PutOtherPlayer;
	}

	BYTE CurrPlayerNum;
	SO_InitDesc Player[MAX_USER];
};

/*
	from client to server
*/

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

struct CS_Success : public HEADER
{
	CS_Success()
	{
		Size = sizeof(*this); Type = eCS::Success;
	}
	BYTE ClientID;
};

/* 
	과거에 쓰던 프로토콜 
	어떻게 활용할 지 고민...
*/
struct cs_packet_up {
	BYTE size;
	BYTE type;
};

struct cs_packet_down {
	BYTE size;
	BYTE type;
};

struct cs_packet_left {
	BYTE size;
	BYTE type;
};

struct cs_packet_right {
	BYTE size;
	BYTE type;
};

struct cs_packet_chat {
	BYTE size;
	BYTE type;
	WCHAR message[MAX_STR_SIZE];
};

struct sc_packet_pos {
	BYTE size;
	BYTE type;
	WORD id;
	BYTE x;
	BYTE y;
};

struct sc_packet_put_player {
	BYTE size;
	BYTE type;
	WORD id;
	BYTE x;
	BYTE y;
};
struct sc_packet_remove_player {
	BYTE size;
	BYTE type;
	WORD id;
};

struct sc_packet_chat {
	BYTE size;
	BYTE type;
	WORD id;
	WCHAR message[MAX_STR_SIZE];
};

#pragma pack (pop)
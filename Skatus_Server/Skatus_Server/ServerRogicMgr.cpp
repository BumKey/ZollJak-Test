#include "ServerRogicMgr.h"

ServerRogicMgr::ServerRogicMgr() : mCurrWaveLevel(1), mCurrPlayerNum(0),
mNewID(-1)
{
	mPerWaveMonsterNum[1][ObjectType::Goblin] = 15;
	mPerWaveMonsterNum[1][ObjectType::Cyclop] = 5;

	mPerWaveMonsterNum[2][ObjectType::Goblin] = 20;
	mPerWaveMonsterNum[2][ObjectType::Cyclop] = 5;

	mPerWaveMonsterNum[3][ObjectType::Goblin] = 40;
	mPerWaveMonsterNum[3][ObjectType::Cyclop] = 10;

	mGameTimer.Reset();
}


ServerRogicMgr::~ServerRogicMgr()
{
}

void ServerRogicMgr::WaveStart()
{
	// 현재 모든 플레이어 나간 후 초기상태로 갱신이 안되기 때문에 여기서 오류발생
	// assert(mGameStateMgr.GetCurrState() == eGameState::GameWaiting);

	mGameStateMgr.Reset();
	mGameStateMgr.FlowAdvance();
	mRogicTimer.Reset();

	DEBUG_MSG("Entering WaveWaiting state...");
}

/// <summary> 서버의 한 프레임: 
/// 서버가 가지고 있는 데이터를 처리하고
/// 그 데이트들을 접속한 클라들에게 보내기까지의 과정.
/// 플레이어의 입력(이벤트) 처리는 비동기적으로 구현. (ProcessKeyInput 함수 참조)
/// 이벤트 발생 시 해당 프레임에 갱신된 데이터를 같이 넣어 패킷 발송
/// </summary>
void ServerRogicMgr::Update()
{
	// 전체 게임시간은 Pause하지 않는 이상 계속 계산.
	mGameTimer.Tick();

	// 조건 인원이 접속 했을 때 Wave시작.
	if (mCurrPlayerNum >= MAX_USER && mGameStateMgr.GetCurrState() == eGameState::GameWaiting)
		WaveStart();

	if (mGameStateMgr.GetCurrState() == eGameState::WaveWaiting)
	{
		// =========== WaveWaiting 상태일 때 =============
		// 1. 로직타이머 시작, 모든 오브젝트 리셋
		// 2. 5초의 시간동안 대기, 시간 정보 패킷 전송
		// 3. 게임상태 진행
		// 4. 로직 타이머 리셋
		if (mRogicTimer.TotalTime() == 0.0f)
			mObjectMgr.ReleaseAllMonsters();


		if (mRogicTimer.TotalTime() > 5.0f)
		{
			mGameStateMgr.FlowAdvance();


		}

		DEBUG_MSG("WaveWaiting...");
	}
	else if (mGameStateMgr.GetCurrState() == eGameState::WaveStart)
	{
		// =========== WaveStart 상태일 때 =============
		// 1. 옵젝 생성
		// 2. 옵젝 정보 패킷 전송
		// 3. 게임상태 진행
		// 4. 로직 타이머 리셋
		mCurrWaveLevel++;

		for (auto oType : mPerWaveMonsterNum[mCurrWaveLevel]) {
				for (UINT i = 0; i < oType.second; ++i)
				mObjectMgr.AddObject(oType.first);
		}
		SendPacketToCreateMonsters();
		mGameStateMgr.FlowAdvance();

		mRogicTimer.Reset();

		DEBUG_MSG("WaveStart...");
	}
	else if (mGameStateMgr.GetCurrState() == eGameState::Waving)
	{
		// =========== Waving 상태일 때 =============
		// 1. 죽은 몬스터들 처리
		// 2. 게임 상태들(남은 몬스터, 남은시간들 등..) 처리
		// 3. 로직 타이머 리셋

		if (mRogicTimer.TotalTime() > 100.0f)
		{
			mGameStateMgr.FlowAdvance();
			mRogicTimer.Reset();

		}
		DEBUG_MSG("Waving...");
	}

	mRogicTimer.Tick();
}

void ServerRogicMgr::AddPlayer(const SOCKET& socket, const ObjectType::Types& oType, const UINT& newID)
{
	++mCurrPlayerNum;
	mNewID = newID;

	mObjectMgr.AddPlayer(oType, newID);
	auto player = mObjectMgr.GetPlayer(newID);

	// 클라이언트 정보 초기화
	g_clients[newID].socket = socket;
	g_clients[newID].avatar = player;
	g_clients[newID].packet_size = 0;
	g_clients[newID].previous_size = 0;
	g_clients[newID].is_connected = true;
	memset(&g_clients[newID].recv_overlap.original_Overlap, 0,
		sizeof(g_clients[newID].recv_overlap.original_Overlap));

	SC_InitPlayer packet;
	packet.ClientID = newID;
	packet.CurrPlayerNum = mCurrPlayerNum;
	for (int i = 0; i < mCurrPlayerNum; ++i)
		packet.Player[i] = mObjectMgr.GetPlayer(i);

	packet.NumOfObjects = mObjectMgr.GetAllBasicObjects().size();
	for (UINT i = 0; i < packet.NumOfObjects; ++i)
		packet.MapInfo[i] = mObjectMgr.GetAllBasicObjects()[i];

	// 모든 플레이어에게 갱신된 플레이어 정보를 전송한다.
	for (UINT i = 0; i < MAX_USER; ++i)
	{
		if(g_clients[i].is_connected)
			MyServer::Send_Packet(i, reinterpret_cast<char *>(&packet));
	}
}

void ServerRogicMgr::RemovePlayer(const UINT & id)
{
	--mCurrPlayerNum;
	mObjectMgr.RemovePlayer(id);
	g_clients[id].is_connected = false;

	if (mCurrPlayerNum <= 0)
		mGameStateMgr.Reset();
	else
	{
		SC_RemovePlayer packet;
		packet.ClientID = id;
		for (UINT i = 0; i < MAX_USER; ++i) {
			if (g_clients[i].is_connected)
				MyServer::Send_Packet(i, reinterpret_cast<char*>(&packet));
		}
	}
}

/// <summary> 현재는 충돌체크 미구현
/// 이것이 데드레커닝인진 모르지만 플레이어의 위치를
/// 5프레임 정도 추정해서 설정한다.
/// </summary>
void ServerRogicMgr::ProcessKeyInput(CS_Move & inPacket)
{
	auto player = mObjectMgr.GetPlayer(inPacket.ClientID);

	float destY = inPacket.Pos.y;
	float currY = player.Pos.y;
	float yDiff = destY - currY;
	float slowFactor = 5.0f * (yDiff);

	XMFLOAT3 dir, pos;

	if (currY < 0.0f)
	{
		dir = Float3Normalize(inPacket.Pos - player.Pos);
		pos = inPacket.Pos + dir*inPacket.MoveSpeed*inPacket.DeltaTime*5.0f;
		pos.y = destY;
	}
	else if (yDiff >= 0.3f && yDiff < 0.6f)
	{
		dir = Float3Normalize(inPacket.Pos - player.Pos);
		pos = inPacket.Pos + dir*inPacket.MoveSpeed*inPacket.DeltaTime*5.0f/slowFactor;
		pos.y = destY;
	}
	else if (yDiff >= 0.8f)
	{
		pos = player.Pos;
	}
	else 
	{
		dir = Float3Normalize(inPacket.Pos - player.Pos);
		pos = inPacket.Pos + dir*inPacket.MoveSpeed*inPacket.DeltaTime*5.0f;
		pos.y = destY;
	}

	mObjectMgr.SetPlayerRot(inPacket.ClientID, inPacket.Rot);
	mObjectMgr.SetPlayerPos(inPacket.ClientID, pos);
}

/// <summary> 현재는 충돌체크 미구현
/// 따라서 공격시 몬스터들과의 거리로 상태변화 구현
/// </summary>
void ServerRogicMgr::ProcessMouseInput(CS_Attack & inPacket)
{
	auto& player = mObjectMgr.GetPlayer(inPacket.ClientID);
	auto& monsters = mObjectMgr.GetMonsters();

	player.ActionState = ActionState::Attack;
	for (UINT i = 0; i < inPacket.Mon_Num; ++i) {
		monsters[i].Hp = inPacket.Mon_HP[i];
		if (monsters[i].Hp <= 0)
			monsters[i].ActionState = ActionState::Die;
	}
}

FLOAT ServerRogicMgr::Distance2D(const XMFLOAT3 & a, const XMFLOAT3 & b)
{
	float x = a.x - b.x;
	float y = a.z - b.z;

	return sqrtf(x*x + y*y);
}

void ServerRogicMgr::SendPacketMonInfo()
{
	auto players = mObjectMgr.GetPlayers();
	auto monsters = mObjectMgr.GetMonsters();

	SC_MonInfo packet;
	packet.GameState = mGameStateMgr.GetCurrState();
	packet.Time = mRogicTimer.TotalTime();
	packet.Roundlevel = mCurrWaveLevel;
	packet.NumOfObjects = mObjectMgr.GetCurrPlayerNum() + monsters.size();

	for (auto m : monsters) {
		packet.Monsters[m.first].TargetID = mObjectMgr.SetMonstersTarget();
		packet.Monsters[m.first].ActionState = m.second.ActionState;
	}

	for (UINT i = 0; i < MAX_USER; ++i) {
		if (g_clients[i].is_connected) 
			MyServer::Send_Packet(i, reinterpret_cast<char*>(&packet));

		players[i].ActionState = ActionState::Idle;
	}
}

void ServerRogicMgr::SendPacektPlayerInfo()
{
	SC_PlayerInfo packet;

	auto& players = mObjectMgr.GetPlayers();
	for (auto p : players) {
		UINT id = p.first;
		packet.Players[id].Pos = p.second.Pos;
		packet.Players[id].Rot = p.second.Rot;
		packet.Players[id].ActionState = p.second.ActionState;
	}

	for (UINT i = 0; i < MAX_USER; ++i) {
		if (g_clients[i].is_connected)
			MyServer::Send_Packet(i, reinterpret_cast<char*>(&packet));
	}
}

void ServerRogicMgr::SendPacketToCreateMonsters()
{
	auto monsters = mObjectMgr.GetMonsters();

	SC_AddMonster packet;
	packet.NumOfObjects = monsters.size();
	
	UINT count(0);
	for (auto m : monsters) {
		packet.InitInfos[m.first] = m.second;
	}

	for (UINT i = 0; i < MAX_USER; ++i) {
		if (g_clients[i].is_connected)
			MyServer::Send_Packet(i, reinterpret_cast<char*>(&packet));
	}
}

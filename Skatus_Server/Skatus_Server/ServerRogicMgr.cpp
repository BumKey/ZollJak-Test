#include "ServerRogicMgr.h"

ServerRogicMgr::ServerRogicMgr() : mCurrWaveLevel(0), mCurrPlayerNum(0),
mNewID(-1), mAddMonPacketSended(false)
{
	srand(time(NULL));

	mPerWaveMonsterNum[1][ObjectType::Goblin] = 25;
	mPerWaveMonsterNum[1][ObjectType::Cyclop] = 5;

	mPerWaveMonsterNum[2][ObjectType::Goblin] = 10;
	mPerWaveMonsterNum[2][ObjectType::Cyclop] = 2;

	mPerWaveMonsterNum[3][ObjectType::Goblin] = 15;
	mPerWaveMonsterNum[3][ObjectType::Cyclop] = 3;

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

	if (mGameStateMgr.GetCurrState() == eGameState::WaveWaiting)
	{
		// =========== WaveWaiting 상태일 때 =============
		// 1. 로직타이머 시작, 모든 오브젝트 리셋
		// 2. 5초의 시간동안 대기, 시간 정보 패킷 전송
		// 3. 게임상태 진행
		// 4. 로직 타이머 리셋
		if (mRogicTimer.TotalTime() == 0.0f) {
			mObjectMgr.ReleaseDeadMonsters();
			SendPacketReleaseDeadMonsters();
		}

		if (mRogicTimer.TotalTime() > 5.0f)
		{
			mGameStateMgr.FlowAdvance();
		}
		else
			mRogicTimer.Tick();

		//DEBUG_MSG("WaveWaiting...");
	}
	else if (mGameStateMgr.GetCurrState() == eGameState::WaveStart)
	{
		// =========== WaveStart 상태일 때 =============
		// 1. 옵젝 생성
		// 2. 옵젝 정보 패킷 전송
		// 3. 게임상태 진행
		// 4. 로직 타이머 리셋

		++mCurrWaveLevel;

		for (auto oType : mPerWaveMonsterNum[mCurrWaveLevel]) {
				for (UINT i = 0; i < oType.second; ++i)
				mObjectMgr.AddObject(oType.first);
		}

		mAddMonPacketSended = false;

		SendPacketToCreateMonsters();
		mGameStateMgr.FlowAdvance();

		mRogicTimer.Reset();

		//DEBUG_MSG("WaveStart...");
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
		else {
			mRogicTimer.Tick();
		}

		//DEBUG_MSG("Waving...");
	}
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

	// 조건 인원이 접속 했을 때 Wave시작.
	if (mCurrPlayerNum >= MAX_USER && mGameStateMgr.GetCurrState() == eGameState::GameWaiting)
		WaveStart();
}

void ServerRogicMgr::AddPlayer(const UINT & id)
{
	SC_InitPlayer packet;
	packet.ClientID = id;
	packet.CurrPlayerNum = mCurrPlayerNum;
	for (int i = 0; i < mCurrPlayerNum; ++i)
		packet.Player[i] = mObjectMgr.GetPlayer(i);

	packet.NumOfObjects = mObjectMgr.GetAllBasicObjects().size();
	for (UINT i = 0; i < packet.NumOfObjects; ++i)
		packet.MapInfo[i] = mObjectMgr.GetAllBasicObjects()[i];

	MyServer::Send_Packet(id, reinterpret_cast<char *>(&packet));
}

void ServerRogicMgr::RemovePlayer(const UINT & id)
{
	--mCurrPlayerNum;
	mObjectMgr.RemovePlayer(id);
	g_clients[id].is_connected = false;

	if (mCurrPlayerNum <= 0) {
		Reset();
	}
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

	mObjectMgr.SetPlayerRot(inPacket.ClientID, inPacket.Rot);
	mObjectMgr.SetCollsion(inPacket.ClientID, inPacket.Pos);
	mObjectMgr.SetPlayerPos(inPacket.ClientID, inPacket.Pos);

	SC_CollisionInfo packet;
	for (UINT i = 0; i < COLL_OBJ_NUM; ++i)
		packet.CollisionPos[i] = mObjectMgr.GetCollisionPos()[i];

	MyServer::Send_Packet(inPacket.ClientID, reinterpret_cast<char*>(&packet));
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

void ServerRogicMgr::Reset()
{
	mCurrWaveLevel = 0;
	mGameTimer.Reset();
	mRogicTimer.Reset();
	mGameStateMgr.Reset();
	mObjectMgr.Reset();
}

FLOAT ServerRogicMgr::Distance2D(const XMFLOAT3 & a, const XMFLOAT3 & b)
{
	float x = a.x - b.x;
	float y = a.z - b.z;

	return sqrtf(x*x + y*y);
}

void ServerRogicMgr::SendPacketFrameInfo()
{
	auto players = mObjectMgr.GetPlayers();
	auto monsters = mObjectMgr.GetMonsters();

	SC_FrameInfo packet;

	if (mGameStateMgr.GetCurrState() == eGameState::WaveWaiting)
		packet.Time = 5.0f - mRogicTimer.TotalTime();
	else if (mGameStateMgr.GetCurrState() == eGameState::Waving)
		packet.Time = 100.0f - mRogicTimer.TotalTime();
	else
		packet.Time = mRogicTimer.TotalTime();

	packet.GameState = mGameStateMgr.GetCurrState();
	packet.Roundlevel = mCurrWaveLevel;
	packet.NumOfPlayers = mObjectMgr.GetCurrPlayerNum();

	for (UINT i = 0; i < MAX_USER; ++i) {
		if (g_clients[i].is_connected) 
			MyServer::Send_Packet(i, reinterpret_cast<char*>(&packet));
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
		if (g_clients[i].is_connected) {
			MyServer::Send_Packet(i, reinterpret_cast<char*>(&packet));
			mObjectMgr.GetPlayer(i).ActionState = ActionState::Idle;
		}
	}
}

void ServerRogicMgr::SendPacketMonInfo()
{
	if (mObjectMgr.GetMonsters().size() > 0)
	{
		mObjectMgr.UpdateMonsters();

		SC_MonInfo packet;
		for (auto m : mObjectMgr.GetMonsters()) {
			packet.Monsters[m.first].TargetPos = m.second.Pos;
			packet.Monsters[m.first].TargetID = m.second.TargetID;
			packet.Monsters[m.first].ActionState = m.second.ActionState;
		}

		int count(0);
		for (auto m : mObjectMgr.GetMonsters()) {
			count = 0;
			for (auto o : mObjectMgr.GetAllBasicObjects())
			{
				if (MathHelper::DistanceVector(m.second.Pos, o.Pos) <= 10.0f) {
					packet.Coll_Ob_Mon[m.first][count++] = o.Pos;
					if (count > COLL_OBJ_NUM-1)
						break;
				}
			}
		}

		packet.NumOfMonsters = mObjectMgr.GetMonsters().size();
		for (UINT i = 0; i < MAX_USER; ++i) {
			if (g_clients[i].is_connected)
				MyServer::Send_Packet(i, reinterpret_cast<char*>(&packet));
		}
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

void ServerRogicMgr::SendPacketToCreateMonsters(const UINT& id)
{
	if (mAddMonPacketSended == false) {
		auto monsters = mObjectMgr.GetMonsters();

		SC_AddMonster packet;
		packet.NumOfObjects = monsters.size();

		UINT count(0);
		for (auto m : monsters) {
			packet.InitInfos[m.first] = m.second;
		}

		MyServer::Send_Packet(id, reinterpret_cast<char*>(&packet));
		mAddMonPacketSended = true;
	}
}

void ServerRogicMgr::SendPacketReleaseDeadMonsters()
{
	SC_ReleaseDeadMonsters packet;
	for (UINT i = 0; i < MAX_USER; ++i) {
		if (g_clients[i].is_connected)
			MyServer::Send_Packet(i, reinterpret_cast<char*>(&packet));
	}
}

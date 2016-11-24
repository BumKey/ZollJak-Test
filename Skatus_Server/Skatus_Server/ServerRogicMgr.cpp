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
	// ���� ��� �÷��̾� ���� �� �ʱ���·� ������ �ȵǱ� ������ ���⼭ �����߻�
	// assert(mGameStateMgr.GetCurrState() == eGameState::GameWaiting);

	mGameStateMgr.Reset();
	mGameStateMgr.FlowAdvance();
	mRogicTimer.Reset();

	DEBUG_MSG("Entering WaveWaiting state...");
}

/// <summary> ������ �� ������: 
/// ������ ������ �ִ� �����͸� ó���ϰ�
/// �� ����Ʈ���� ������ Ŭ��鿡�� ����������� ����.
/// �÷��̾��� �Է�(�̺�Ʈ) ó���� �񵿱������� ����. (ProcessKeyInput �Լ� ����)
/// �̺�Ʈ �߻� �� �ش� �����ӿ� ���ŵ� �����͸� ���� �־� ��Ŷ �߼�
/// </summary>
void ServerRogicMgr::Update()
{
	// ��ü ���ӽð��� Pause���� �ʴ� �̻� ��� ���.
	mGameTimer.Tick();

	if (mGameStateMgr.GetCurrState() == eGameState::WaveWaiting)
	{
		// =========== WaveWaiting ������ �� =============
		// 1. ����Ÿ�̸� ����, ��� ������Ʈ ����
		// 2. 5���� �ð����� ���, �ð� ���� ��Ŷ ����
		// 3. ���ӻ��� ����
		// 4. ���� Ÿ�̸� ����
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
		// =========== WaveStart ������ �� =============
		// 1. ���� ����
		// 2. ���� ���� ��Ŷ ����
		// 3. ���ӻ��� ����
		// 4. ���� Ÿ�̸� ����

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
		// =========== Waving ������ �� =============
		// 1. ���� ���͵� ó��
		// 2. ���� ���µ�(���� ����, �����ð��� ��..) ó��
		// 3. ���� Ÿ�̸� ����

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

	// Ŭ���̾�Ʈ ���� �ʱ�ȭ
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

	// ��� �÷��̾�� ���ŵ� �÷��̾� ������ �����Ѵ�.
	for (UINT i = 0; i < MAX_USER; ++i)
	{
		if(g_clients[i].is_connected)
			MyServer::Send_Packet(i, reinterpret_cast<char *>(&packet));
	}

	// ���� �ο��� ���� ���� �� Wave����.
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

/// <summary> ����� �浹üũ �̱���
/// �̰��� ���巹Ŀ������ ������ �÷��̾��� ��ġ��
/// 5������ ���� �����ؼ� �����Ѵ�.
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

/// <summary> ����� �浹üũ �̱���
/// ���� ���ݽ� ���͵���� �Ÿ��� ���º�ȭ ����
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

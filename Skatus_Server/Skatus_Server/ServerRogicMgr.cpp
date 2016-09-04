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
	// ���� ��� �÷��̾� ���� �� �ʱ���·� ������ �ȵǱ� ������ ���⼭ �����߻�
	// assert(mGameStateMgr.GetCurrState() == eGameState::GameWaiting);

	mGameStateMgr.Reset();
	mGameStateMgr.FlowAdvance();
	mRogicTimer.Reset();

	std::cout << "Entering WaveWaiting state..." << std::endl;
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

	// ���� �ο��� ���� ���� �� Wave����.
	if (mCurrPlayerNum >= MAX_USER && mGameStateMgr.GetCurrState() == eGameState::GameWaiting)
		WaveStart();

	if (mGameStateMgr.GetCurrState() == eGameState::WaveWaiting)
	{
		// =========== WaveWaiting ������ �� =============
		// 1. ����Ÿ�̸� ����, ��� ������Ʈ ����
		// 2. 5���� �ð����� ���, �ð� ���� ��Ŷ ����
		// 3. ���ӻ��� ����
		// 4. ���� Ÿ�̸� ����
		if (mRogicTimer.TotalTime() == 0.0f)
			mObjectMgr.ReleaseAllMonsters();


		if (mRogicTimer.TotalTime() > 5.0f)
		{
			mGameStateMgr.FlowAdvance();


		}

		std::cout << "WaveWaiting..." << std::endl;
	}
	else if (mGameStateMgr.GetCurrState() == eGameState::WaveStart)
	{
		// =========== WaveStart ������ �� =============
		// 1. ���� ����
		// 2. ���� ���� ��Ŷ ����
		// 3. ���ӻ��� ����
		// 4. ���� Ÿ�̸� ����
		mCurrWaveLevel++;

		for (auto oType : mPerWaveMonsterNum[mCurrWaveLevel]) {
				for (UINT i = 0; i < oType.second; ++i)
				mObjectMgr.AddObject(oType.first);
		}
		SendPacketToCreateMonsters();
		mGameStateMgr.FlowAdvance();

		mRogicTimer.Reset();


		std::cout << "WaveStart..." << std::endl;
	}
	else if (mGameStateMgr.GetCurrState() == eGameState::Waving)
	{
		// =========== Waving ������ �� =============
		// 1. ���� ���͵� ó��
		// 2. ���� ���µ�(���� ����, �����ð��� ��..) ó��
		// 3. ���� Ÿ�̸� ����

		SetMonstersTarget();

		if (mRogicTimer.TotalTime() > 100.0f)
		{
			mGameStateMgr.FlowAdvance();
			mRogicTimer.Reset();

		}
		std::cout << "Waving..." << std::endl;
	}

	mRogicTimer.Tick();

	SendPacketPerFrame();
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
}

void ServerRogicMgr::RemovePlayer(const UINT & id)
{
	--mCurrPlayerNum;
	mObjectMgr.RemovePlayer(id);

	if (mCurrPlayerNum <= 0)
		mGameStateMgr.Reset();
}

/// <summary> ����� �浹üũ �̱���
/// �̰��� ���巹Ŀ������ ������ �÷��̾��� ��ġ��
/// 5������ ���� �����ؼ� �����Ѵ�.
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

/// <summary> ����� �浹üũ �̱���
/// ���� ���ݽ� ���͵���� �Ÿ��� ���º�ȭ ����
/// </summary>
void ServerRogicMgr::ProcessMouseInput(CS_Attack & inPacket)
{
	auto& player = mObjectMgr.GetPlayer(inPacket.ClientID);
	player.ActionState = ActionState::Attack;
}

FLOAT ServerRogicMgr::Distance2D(const XMFLOAT3 & a, const XMFLOAT3 & b)
{
	float x = a.x - b.x;
	float y = a.z - b.z;

	return sqrtf(x*x + y*y);
}

void ServerRogicMgr::SetMonstersTarget()
{
	mObjectMgr.SetMonstersTarget();
}

void ServerRogicMgr::SendPacketPerFrame()
{
	auto players = mObjectMgr.GetPlayers();
	auto monsters = mObjectMgr.GetMonsters();

	SC_PerFrame packet;
	packet.GameState = mGameStateMgr.GetCurrState();
	packet.Time = mRogicTimer.TotalTime();
	packet.Roundlevel = mCurrWaveLevel;
	packet.NumOfObjects = mObjectMgr.GetCurrPlayerNum() + monsters.size();

	for (auto p : players) {
		UINT id = p.first;
		packet.Players[id].Pos = p.second.Pos;
		packet.Players[id].Rot = p.second.Rot;
		packet.Players[id].ActionState = p.second.ActionState;
	}

	for (auto m : monsters) {
		packet.Target[m.first] = eMonsterTarget::Temple;
		packet.Monsters[m.first].Pos = m.second.Pos;
		packet.Monsters[m.first].Rot = m.second.Rot;
		packet.Monsters[m.first].ActionState = m.second.ActionState;
	}

	for (UINT i = 0; i < MAX_USER; ++i) {
		if (g_clients[i].is_connected) 
			MyServer::Send_Packet(i, reinterpret_cast<char*>(&packet));

		players[i].ActionState = ActionState::Idle;
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

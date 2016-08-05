#include "ServerRogicMgr.h"

ServerRogicMgr::ServerRogicMgr() : mCurrWaveLevel(0), mCurrPlayerNum(0)
{
	mPerWaveMonsterNum[1][ObjectType::Goblin] = 15;
	mPerWaveMonsterNum[1][ObjectType::Cyclop] = 2;

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

		mRogicTimer.Tick();
		
		if (mRogicTimer.TotalTime() > 5.0f)
		{
			mGameStateMgr.FlowAdvance();
			mRogicTimer.Reset();
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

		for (auto oType : mPerWaveMonsterNum[mCurrWaveLevel]) {
			for (UINT i = 0; i < oType.second; ++i)
				mObjectMgr.AddObject(oType.first);
		}

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

		std::cout << "Waving..." << std::endl;
	}

	if (mGameStateMgr.GetCurrState() == eGameState::WaveStart)
		SendPacketToCreateMonsters();
	else
		SendPacketPerFrame();
}

void ServerRogicMgr::AddPlayer(const SOCKET& socket, const ObjectType::Types& oType, const UINT& id)
{
	++mCurrPlayerNum;

	mObjectMgr.AddPlayer(oType, id);
	auto player = mObjectMgr.GetPlayer();

	// Ŭ���̾�Ʈ ���� �ʱ�ȭ
	g_clients[id].socket = socket;
	g_clients[id].avatar = player[id];
	g_clients[id].packet_size = 0;
	g_clients[id].previous_size = 0;
	memset(&g_clients[id].recv_overlap.original_Overlap, 0,
		sizeof(g_clients[id].recv_overlap.original_Overlap));

	SC_PutPlayer packet;
	packet.ClientID = id;
	packet.CurrPlayerNum = mCurrPlayerNum;
	for (int i = 0; i < mCurrPlayerNum; ++i)
		packet.Player[i] = player[i];

	packet.NumOfObjects = mObjectMgr.GetAllBasicObjects().size();
	for (UINT i = 0; i < packet.NumOfObjects; ++i)
		packet.MapInfo[i] = mObjectMgr.GetAllBasicObjects()[i];

	// �̹� ������ �÷��̾�鿡�Ե� ���ο� �÷��̾ ���Դٴ� ��Ŷ�� ����
	for (int i = 0; i < mCurrPlayerNum; ++i) 
			MyServer::Send_Packet(i, reinterpret_cast<char *>(&packet));

	//// �̹� ������ �÷��̾�鿡 ���� ���� ����
	//for (int i = 0; i < MAX_USER; ++i)
	//{
	//	// ������ ������ ó�� ������ �������, for���� �� �ʿ䰡 �����Ƿ�, break.
	//	if (g_clients[i].is_connected == true && i != new_id)
	//	{
	//		sc_packet_put_player ex_enter_packet;
	//		ex_enter_packet.size = sizeof(ex_enter_packet);
	//		ex_enter_packet.type = SC_PUT_PLAYER;
	//		ex_enter_packet.client_id = i;
	//		ex_enter_packet.Player.ObjectType = g_clients[i].avatar.ObjectType;
	//		ex_enter_packet.Player.Pos.x = g_clients[i].avatar.Pos.x;
	//		ex_enter_packet.Player.Rot = g_clients[i].avatar.Rot;
	//		ex_enter_packet.Player.Scale = g_clients[i].avatar.Scale;

	//		// �÷��̾� ���ο���, �̹� ������ �����鿡 ���� ������ �����Ѵ�.
	//		MyServer::Send_Packet(new_id, reinterpret_cast<char*>(&ex_enter_packet));
	//	}
	//}
}

/// <summary> ����� �浹üũ �̱���
/// �̰��� ���巹Ŀ������ ������ �÷��̾��� ��ġ��
/// 0.2�� ���� �����ؼ� �����Ѵ�.
/// </summary>
void ServerRogicMgr::ProcessKeyInput(CS_Move & inPacket)
{
	XMFLOAT3 dir = Float3Normalize(inPacket.Pos - mObjectMgr.GetPlayer()[inPacket.ClientID].Pos);
	XMFLOAT3 pos = inPacket.Pos + dir;
	mObjectMgr.SetPlayerPos(inPacket.ClientID, pos);
}

/// <summary> ����� �浹üũ �̱���
/// ���� ���ݽ� ���͵���� �Ÿ��� ���º�ȭ ����
/// </summary>
void ServerRogicMgr::ProcessMouseInput(CS_Attack & inPacket)
{
	/*auto player = mObjectMgr.GetPlayer(inPacket.ClientID);
	auto monsters = mObjectMgr.GetMonsters();
	for (auto m : monsters)
	{
		if (Distance2D(player.Pos, m.Pos) < 3.5f)
			m.ActionState = ActionState::Damage;
	}*/
}

FLOAT ServerRogicMgr::Distance2D(const XMFLOAT3 & a, const XMFLOAT3 & b)
{
	float x = a.x - b.x;
	float y = a.z - b.z;

	return sqrtf(x*x + y*y);
}

void ServerRogicMgr::SendPacketPerFrame()
{
	auto objects = mObjectMgr.GetAllSkinnedObjects();

	SC_PerFrame packet;
	packet.GameState = mGameStateMgr.GetCurrState();
	packet.Time = mRogicTimer.TotalTime();
	packet.NumOfObjects = objects.size();

	UINT count(0);
	for (auto o : objects) {
		packet.ID[count] = o.first;
		packet.Objects[count].Pos = o.second.Pos;
		++count;
	}

	for (UINT i = 0; i < mCurrPlayerNum; ++i)
		MyServer::Send_Packet(i, reinterpret_cast<char*>(&packet));
}

void ServerRogicMgr::SendPacketToCreateMonsters()
{
	auto monsters = mObjectMgr.GetMonsters();

	SC_AddMonster packet;
	packet.NumOfObjects = monsters.size();
	
	UINT count(0);
	for (auto m : monsters) {
		packet.ID[count] = m.first;
		packet.InitInfos[count] = m.second;
		++count;
	}

	for (UINT i = 0; i < mCurrPlayerNum; ++i)
		MyServer::Send_Packet(i, reinterpret_cast<char*>(&packet));
}

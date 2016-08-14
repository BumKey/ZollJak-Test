#include "ServerRogicMgr.h"

ServerRogicMgr::ServerRogicMgr() : mCurrWaveLevel(0)
{
	mPerWaveMonsterNum[1][ObjectType::Goblin] = 15;
	mPerWaveMonsterNum[1][ObjectType::Cyclop] = 2;

	mPerWaveMonsterNum[2][ObjectType::Goblin] = 20;
	mPerWaveMonsterNum[2][ObjectType::Cyclop] = 5;

	mPerWaveMonsterNum[3][ObjectType::Goblin] = 40;
	mPerWaveMonsterNum[3][ObjectType::Cyclop] = 10;

	mGameTimer.Reset();
	mRogicTimer.Reset();
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
	}
	else if (mGameStateMgr.GetCurrState() == eGameState::Waving)
	{
		// =========== Waving ������ �� =============
		// 1. ���� ���͵� ó��
		// 2. ���� ���µ�(���� ����, �����ð��� ��..) ó��
		// 3. ���� Ÿ�̸� ����
	}

	SendPacket();
}

/// <summary> ����� �浹üũ �̱���
/// ���� ���� ��Ŷ�� �ƹ� ó������ ����.
/// </summary>
void ServerRogicMgr::ProcessKeyInput(cs_packet_move & inPacket)
{
	auto player = mObjectMgr.GetPlayer(inPacket.client_id);
	player.Pos = inPacket.pos;
}

/// <summary> ����� �浹üũ �̱���
/// ���� ���ݽ� ���͵���� �Ÿ��� ���º�ȭ ����
/// </summary>
void ServerRogicMgr::ProcessMouseInput(cs_packet_attack & inPacket)
{
	auto player = mObjectMgr.GetPlayer(inPacket.client_id);
	auto monsters = mObjectMgr.GetMonsters();
	for (auto m : monsters)
	{
		if (Distance2D(player.Pos, m.Pos) < 3.5f)
			m.ActionState = ActionState::Damage;
	}
}

FLOAT ServerRogicMgr::Distance2D(const XMFLOAT3 & a, const XMFLOAT3 & b)
{
	float x = a.x - b.x;
	float y = a.z - b.z;

	return sqrtf(x*x + y*y);
}

void ServerRogicMgr::SendPacket()
{
	sc_packet_PerFrame packet;
	packet.size = sizeof(packet);
	packet.type = SC_PER_FRAME;
	packet.time = mRogicTimer.TotalTime();
	packet.cInfos = mObjectMgr.GetAllObjects();

	for (UINT i = 0; i < MAX_USER; ++i)
		MyServer::Send_Packet(i, reinterpret_cast<UCHAR*>(&packet));
}

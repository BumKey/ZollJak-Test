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
	// 현재 모든 플레이어 나간 후 초기상태로 갱신이 안되기 때문에 여기서 오류발생
	// assert(mGameStateMgr.GetCurrState() == eGameState::GameWaiting);

	mGameStateMgr.Reset();
	mGameStateMgr.FlowAdvance();
	std::cout << "Entering WaveWaiting state..." << std::endl;
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
		// =========== WaveStart 상태일 때 =============
		// 1. 옵젝 생성
		// 2. 옵젝 정보 패킷 전송
		// 3. 게임상태 진행
		// 4. 로직 타이머 리셋

		for (auto oType : mPerWaveMonsterNum[mCurrWaveLevel]) {
			for (UINT i = 0; i < oType.second; ++i)
				mObjectMgr.AddObject(oType.first);
		}

		mGameStateMgr.FlowAdvance();
		mRogicTimer.Reset();
	}
	else if (mGameStateMgr.GetCurrState() == eGameState::Waving)
	{
		// =========== Waving 상태일 때 =============
		// 1. 죽은 몬스터들 처리
		// 2. 게임 상태들(남은 몬스터, 남은시간들 등..) 처리
		// 3. 로직 타이머 리셋
	}

	SendPacket();
}

/// <summary> 현재는 충돌체크 미구현
/// 따라서 들어온 패킷을 아무 처리없이 갱신.
/// </summary>
void ServerRogicMgr::ProcessKeyInput(cs_packet_move & inPacket)
{
	auto player = mObjectMgr.GetPlayer(inPacket.client_id);
	player.Pos = inPacket.pos;
}

/// <summary> 현재는 충돌체크 미구현
/// 따라서 공격시 몬스터들과의 거리로 상태변화 구현
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

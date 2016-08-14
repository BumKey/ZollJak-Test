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
	// 현재 모든 플레이어 나간 후 초기상태로 갱신이 안되기 때문에 여기서 오류발생
	// assert(mGameStateMgr.GetCurrState() == eGameState::GameWaiting);

	mGameStateMgr.Reset();
	mGameStateMgr.FlowAdvance();
	mRogicTimer.Reset();

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

		std::cout << "WaveStart..." << std::endl;
	}
	else if (mGameStateMgr.GetCurrState() == eGameState::Waving)
	{
		// =========== Waving 상태일 때 =============
		// 1. 죽은 몬스터들 처리
		// 2. 게임 상태들(남은 몬스터, 남은시간들 등..) 처리
		// 3. 로직 타이머 리셋

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

	// 클라이언트 정보 초기화
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

	// 이미 접속한 플레이어들에게도 새로운 플레이어가 들어왔다는 패킷을 전송
	for (int i = 0; i < mCurrPlayerNum; ++i) 
			MyServer::Send_Packet(i, reinterpret_cast<char *>(&packet));

	//// 이미 접속한 플레이어들에 대한 정보 전송
	//for (int i = 0; i < MAX_USER; ++i)
	//{
	//	// 접속한 유저가 처음 접속한 유저라면, for문을 돌 필요가 없으므로, break.
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

	//		// 플레이어 본인에게, 이미 접속한 유저들에 대한 정보를 전송한다.
	//		MyServer::Send_Packet(new_id, reinterpret_cast<char*>(&ex_enter_packet));
	//	}
	//}
}

/// <summary> 현재는 충돌체크 미구현
/// 이것이 데드레커닝인진 모르지만 플레이어의 위치를
/// 0.2초 정도 추정해서 설정한다.
/// </summary>
void ServerRogicMgr::ProcessKeyInput(CS_Move & inPacket)
{
	XMFLOAT3 dir = Float3Normalize(inPacket.Pos - mObjectMgr.GetPlayer()[inPacket.ClientID].Pos);
	XMFLOAT3 pos = inPacket.Pos + dir;
	mObjectMgr.SetPlayerPos(inPacket.ClientID, pos);
}

/// <summary> 현재는 충돌체크 미구현
/// 따라서 공격시 몬스터들과의 거리로 상태변화 구현
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

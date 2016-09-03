#include "stdafx.h"
#include "ObjectMgr.h"
#include "ServerRogicMgr.h"

Client clients[MAX_USER];

HANDLE g_hIocp;
bool	g_isshutdown = false;
ObjectMgr g_objectmgr;
ServerRogicMgr g_serverRogicmgr;

#pragma comment (lib, "ws2_32.lib")

void Initialize()
{
	for (auto i = 0; i < MAX_USER; ++i) {
		clients[i].is_connected = false;
		clients[i].recv_overlap.operation = OP_RECV;
		clients[i].recv_overlap.wsabuf.buf =
			reinterpret_cast<CHAR *>(clients[i].recv_overlap.iocp_buffer);
		clients[i].recv_overlap.wsabuf.len =
			sizeof(clients[i].recv_overlap.iocp_buffer);
	}

	_wsetlocale(LC_ALL, L"korean");

	WSADATA	wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	g_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
}

void SendPacket(int id, unsigned char *packet)
{
	Overlap_ex *over = new Overlap_ex;
	memset(over, 0, sizeof(Overlap_ex));
	over->operation = OP_SEND;

	over->wsabuf.buf = reinterpret_cast<CHAR *>(over->iocp_buffer);
	over->wsabuf.len = packet[0];
	memcpy(over->iocp_buffer, packet, packet[0]);

	int ret = WSASend(clients[id].s, &over->wsabuf, 1, NULL, 0,
		&over->original_overlap, NULL);
	if (0 != ret) {
		int error_no = WSAGetLastError();
		error_display("SendPacket:WSASend", error_no);
		while (true);
	}
}

void SendPutPlayerPacket(int client, int object)
{
	sc_packet_put_player packet;
	packet.id = object;
	packet.size = sizeof(packet);
	packet.type = SC_PUT_PLAYER;
	packet.x = clients[object].avatar.Pos.x;
	packet.y = clients[object].avatar.Pos.y;

	SendPacket(client, reinterpret_cast<unsigned char *>(&packet));
}

void SendRemovePlayerPacket(int client, int object)
{
	sc_packet_remove_player packet;
	packet.id = object;
	packet.size = sizeof(packet);
	packet.type = SC_REMOVE_PLAYER;

	SendPacket(client, reinterpret_cast<unsigned char *>(&packet));
}

void ProcessKeyInput(CS_Move &inPacket)
{
	auto player = g_objectmgr.GetPlayer(inPacket.ClientID);

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
		pos = inPacket.Pos + dir*inPacket.MoveSpeed*inPacket.DeltaTime*5.0f / slowFactor;
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

	g_objectmgr.SetPlayerRot(inPacket.ClientID, inPacket.Rot);
	g_objectmgr.SetPlayerPosXZ(inPacket.ClientID, pos);
}

void ProcessMouseInput(CS_Attack &inPacket)
{
	/*
	auto player = mObjectMgr.GetPlayer(inPacket.ClientID);
	auto monsters = mObjectMgr.GetMonsters();
	for (auto m : monsters)
	{
	if (Distance2D(player.Pos, m.Pos) < 3.5f)
	m.ActionState = ActionState::Damage;
	}*/
}

void ProcessPacket(int id, unsigned char *packet)
{
	/*
	int x = clients[id].avatar.Pos.x;
	int y = clients[id].avatar.Pos.y;

	switch (buf[1])
	{
	case CS_UP: y--; break;
	case CS_DOWN: y++; break;
	case CS_LEFT: x--; break;
	case CS_RIGHT: x++; break;
	default: cout << "Unknown type packet received!\n";
		while (true);
	}
	if (y < 0) y = 0;
	if (y >= BOARD_HEIGHT) y = BOARD_HEIGHT - 1;
	if (x < 0) x = 0;
	if (x >= BOARD_WIDTH) x = BOARD_WIDTH - 1;

	clients[id].avatar.Pos.x = x;
	clients[id].avatar.Pos.y = y;

	sc_packet_pos mov_packet;
	mov_packet.id = id;
	mov_packet.size = sizeof(mov_packet);
	mov_packet.type = SC_POS;
	mov_packet.x = x;
	mov_packet.y = y;

	SendPacket(id, reinterpret_cast<unsigned char *>(&mov_packet));

	printf("id = %d, 좌표 : %d,%d <X,Y> - 패킷 전송완료 \n", id, clients[id].avatar.Pos.x, clients[id].avatar.Pos.y);
	*/
	HEADER *header = reinterpret_cast<HEADER*>(packet);
	switch (header->Type)
	{
	case eCS::Success: {
		auto info = reinterpret_cast<CS_Success*>(packet);
		cout << "CS_SUCCESS, ID : " << (int)info->ClientID << endl;
		g_serverRogicmgr.UnLock(info->ClientID);
		g_serverRogicmgr.Update(info->ClientID);
		break;
	}
	case eCS::KeyInput: {
		auto info = reinterpret_cast<CS_Move*>(packet);
		std::cout << "CS_KEYINPUT, POS : " << info->Pos.x << ", "
			<< info->Pos.y << ", " << info->Pos.z << std::endl;
		ProcessKeyInput(*info);
		g_serverRogicmgr.UnLock(info->ClientID);
		g_serverRogicmgr.Update(info->ClientID);
		break;
	}
	case eCS::MouseInput: {
		auto info = reinterpret_cast<CS_Attack*>(packet);
		std::cout << "CS_MOUSEINPUT, ID : " << info->ClientID << std::endl;
		ProcessMouseInput(*info);
		g_serverRogicmgr.UnLock(info->ClientID);
		g_serverRogicmgr.Update(info->ClientID);
		break;
	}
	default:
		break;
	}

	//unordered_set <int> new_list;
	//for (auto i = 0; i < MAX_USER; ++i) {
	//	if (false == clients[i].is_connected) continue;
	//	if (i == id) continue;
	//	if (false == Is_InRange(i, id)) continue;
	//	new_list.insert(i);
	//}

	//for (auto i : new_list) {
	//	clients[id].vl_lock.lock();
	//	if (0 == clients[id].view_list.count(i)) { // 새로 뷰리스트에 들어오는 객체 처리
	//		clients[id].view_list.insert(i);
	//		clients[id].vl_lock.unlock();
	//		SendPutPlayerPacket(id, i);

	//		clients[i].vl_lock.lock();
	//		if (0 == clients[i].view_list.count(id)) {
	//			clients[i].view_list.insert(id);
	//			clients[i].vl_lock.unlock();
	//			SendPutPlayerPacket(i, id);
	//		}
	//		else {
	//			clients[i].vl_lock.unlock();
	//			SendPacket(i, reinterpret_cast<unsigned char *>(&mov_packet));
	//		}
	//	}
	//	else { // 뷰리스트에 계속 유지되어 있는 객체 처리
	//		clients[id].vl_lock.unlock();
	//		clients[i].vl_lock.lock();
	//		if (1 == clients[i].view_list.count(id)) {
	//			clients[i].vl_lock.unlock();
	//			SendPacket(i, reinterpret_cast<unsigned char *>(&mov_packet));
	//		}
	//		else {
	//			clients[i].view_list.insert(id);
	//			clients[i].vl_lock.unlock();
	//			SendPutPlayerPacket(i, id);
	//		}
	//	}
	//}

	//// 뷰리스트에서 나가는 객체 처리
	//vector <int> remove_list;
	//clients[id].vl_lock.lock();
	//for (auto i : clients[id].view_list) {
	//	if (0 != new_list.count(i)) continue;
	//	remove_list.push_back(i);
	//}
	//for (auto i : remove_list) clients[id].view_list.erase(i);
	//clients[id].vl_lock.unlock();

	//for (auto i : remove_list)
	//	SendRemovePlayerPacket(id, i);

	//for (auto i : remove_list) {
	//	clients[i].vl_lock.lock();
	//	if (0 != clients[i].view_list.count(id)) {
	//		clients[i].view_list.erase(id);
	//		clients[i].vl_lock.unlock();
	//		SendRemovePlayerPacket(i, id);
	//	}
	//	else 	clients[i].vl_lock.unlock();
	//}
}

void WorkerThreadStart()
{
	while (false == g_isshutdown)
	{
		DWORD iosize;
		DWORD key;
		Overlap_ex *my_overlap;

		BOOL result = GetQueuedCompletionStatus(g_hIocp,
			&iosize, &key, reinterpret_cast<LPOVERLAPPED *>(&my_overlap), INFINITE);
		if (FALSE == result) {
			// 에러 처리
		}
		if (0 == iosize) {
			closesocket(clients[key].s);
			/*
				제거하는 패킷 구현해야함.
			*/
			/*sc_packet_remove_player discon;
			discon.id = key;
			discon.size = sizeof(discon);
			discon.type = SC_REMOVE_PLAYER;
			for (auto i = 0; i < MAX_USER; ++i) {
				if (false == clients[i].is_connected) continue;
				if (key == i) continue;
				SendPacket(i, reinterpret_cast<unsigned char*>(&discon));
			}
			printf("id : %d가 접속을 종료하였습니다. \n", key);*/
			clients[key].is_connected = false;
		}

		if (OP_RECV == my_overlap->operation) {
			unsigned char *buf_ptr = clients[key].recv_overlap.iocp_buffer;
			int remained = iosize;
			while (0 < remained) {
				if (0 == clients[key].packet_size)
					clients[key].packet_size = buf_ptr[0];
				int required = clients[key].packet_size
					- clients[key].previous_size;
				if (remained >= required) {
					memcpy(
						clients[key].packet_buff + clients[key].previous_size,
						buf_ptr, required);
					ProcessPacket(key, clients[key].packet_buff);
					buf_ptr += required;
					remained -= required;
					clients[key].packet_size = 0;
					clients[key].previous_size = 0;
				}
				else {
					memcpy(clients[key].packet_buff
						+ clients[key].previous_size,
						buf_ptr, remained);
					buf_ptr += remained;
					clients[key].previous_size += remained;
					remained = 0;
				}
			}
			DWORD flags = 0;
			WSARecv(clients[key].s,
				&clients[key].recv_overlap.wsabuf, 1, NULL, &flags,
				&clients[key].recv_overlap.original_overlap, NULL);
		}
		else if (OP_SEND == my_overlap->operation) {
			delete my_overlap;
		}
		else {
			cout << "Unknown IOCP event!\n";
			exit(-1);
		}
	}
}

void AcceptThreadStart()
{
	struct sockaddr_in listen_addr;

	SOCKET accept_socket = WSASocket(AF_INET, SOCK_STREAM,
		IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = htonl(ADDR_ANY);
	listen_addr.sin_port = htons(MY_SERVER_PORT);

	::bind(accept_socket,
		reinterpret_cast<sockaddr *>(&listen_addr), sizeof(listen_addr));
	listen(accept_socket, 10);

	// listen
	while (false == g_isshutdown)
	{
		struct sockaddr_in client_addr;
		int addr_size = sizeof(client_addr);
		SOCKET new_client = WSAAccept(accept_socket,
			reinterpret_cast<sockaddr *>(&client_addr), &addr_size,
			NULL, NULL);

		if (INVALID_SOCKET == new_client) {
			int error_no = WSAGetLastError();
			error_display("Accept::WSAAccept", error_no);
			while (true);
		}
		else
			cout << "Server Connect Success" << endl;

		int new_id = -1;
		for (auto i = 0; i < MAX_USER; ++i)
			if (false == clients[i].is_connected) {
				new_id = i;
				break;
			}

		if (-1 == new_id) {
			cout << "Max Concurrent User excceded!\n";
			closesocket(new_client);
			continue;
		}
		
		// 새로운 클라이언트가 들어오면 그에 대한 정보를 전역변수인 클라이언트에 넣어준다.
		clients[new_id].s = new_client;
		clients[new_id].avatar.ObjectType = ObjectType::Warrior;
		clients[new_id].avatar.Pos = XMFLOAT3(rand()%20+200.0f, -0.1f, rand()%20-280.0f);
		clients[new_id].avatar.Rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		clients[new_id].avatar.Scale = 0.05f;
		clients[new_id].avatar.MoveSpeed = 9.0f;
		clients[new_id].avatar.AttackPoint = 50.0f;
		clients[new_id].avatar.AttackSpeed = 2.0f;
		clients[new_id].avatar.Hp = 100;

		clients[new_id].packet_size = 0;
		clients[new_id].previous_size = 0;

		memset(&clients[new_id].recv_overlap.original_overlap, 0,
			sizeof(clients[new_id].recv_overlap.original_overlap));

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(new_client),
			g_hIocp, new_id, 0);

		clients[new_id].vl_lock.lock();
		clients[new_id].view_list.clear();
		clients[new_id].vl_lock.unlock();

		/*sc_packet_put_player enter_packet;
		enter_packet.id = new_id;
		enter_packet.size = sizeof(enter_packet);
		enter_packet.type = SC_PUT_PLAYER;
		enter_packet.x = clients[new_id].avatar.Pos.x;
		enter_packet.y = clients[new_id].avatar.Pos.y;*/

		SC_InitPlayer enter_packet;
		enter_packet.ClientID = new_id;
		enter_packet.CurrPlayerNum = new_id + 1;

		// 쓸모가 없을 것 같은 서버 패킷 메모리 낭비가 될 여지가 있는 코드 - 변경할 필요 있음.
		for (int i = 0; i < new_id + 1; ++i)
			enter_packet.Player[i] = g_objectmgr.GetPlayer(i);

		enter_packet.NumOfObjects = g_objectmgr.GetAllBasicObjects().size();
		for (UINT i = 0; i < enter_packet.NumOfObjects; ++i)
			enter_packet.MapInfo[i] = g_objectmgr.GetAllBasicObjects()[i];

		SendPacket(new_id, reinterpret_cast<unsigned char *>(&enter_packet));

		printf("id : %d가 접속 하였습니다. \n", new_id);

		// 본인을 제외한 나머지 인원들에게 자신이 들어왔음을 보내는 패킷
		for (auto i = 0; i < MAX_USER; ++i) {
			if (false == clients[i].is_connected) continue;
			if (i == new_id) continue;
			if (false == Is_InRange(i, new_id)) continue;
			clients[i].vl_lock.lock();
			clients[i].view_list.insert(new_id);
			clients[i].vl_lock.unlock();
			SendPacket(i, reinterpret_cast<unsigned char *>(&enter_packet));
		}

		//// 본인을 제외하고 기존에 들어와 있는 인원들에 대한 정보를 보내는 패킷
		//for (auto i = 0; i < MAX_USER; ++i) {
		//	if (false == clients[i].is_connected) continue;
		//	if (i == new_id) continue;
		//	if (false == Is_InRange(i, new_id)) continue;
		//	clients[new_id].vl_lock.lock();
		//	clients[new_id].view_list.insert(i);
		//	clients[new_id].vl_lock.unlock();
		//	/*enter_packet.id = i;
		//	enter_packet.x = clients[i].avatar.Pos.x;
		//	enter_packet.y = clients[i].avatar.Pos.y;*/
		//	/* 
		//		어떻게 기존의 코드를 추가할지 고민해야 되는 부분 
		//	*/
		//	SendPacket(new_id, reinterpret_cast<unsigned char *>(&enter_packet));
		//}

		clients[new_id].is_connected = true;
		DWORD flags = 0;
		int ret = WSARecv(new_client, &clients[new_id].recv_overlap.wsabuf, 1, NULL,
			&flags, &clients[new_id].recv_overlap.original_overlap, NULL);
		if (0 != ret)
		{
			int error_no = WSAGetLastError();
			if (WSA_IO_PENDING != error_no)
				error_display("Accept:WSARecv", error_no);
		}
	}
}

void Cleanup()
{
	WSACleanup();
}

int main()
{
	vector <thread *> worker_threads;

	Initialize();

	for (auto i = 0; i < NUM_THREADS; ++i)
		worker_threads.push_back(new thread{ WorkerThreadStart });

	thread accept_thread{ AcceptThreadStart };

	while (false == g_isshutdown)
	{
		Sleep(1000);
	}

	for (auto th : worker_threads) {
		th->join();
		delete th;
	}
	accept_thread.join();
	Cleanup();
}

/*
	변경이 필요한 함수
*/


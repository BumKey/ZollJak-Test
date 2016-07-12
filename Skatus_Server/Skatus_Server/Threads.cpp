#include "Threads.h"

// 어셉트 스레드
void MyThreads::Accept_Thread()
{
	// 로컬 주소 정보 초기화
	struct sockaddr_in listen_addr;
	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = htonl(ADDR_ANY);
	listen_addr.sin_port = htons(SERVER_PORT);

	// 어셉트 소켓 생성
	SOCKET accept_socket = WSASocket(AF_INET, SOCK_STREAM,
		IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	// bind
	bind(accept_socket, reinterpret_cast<sockaddr *>(&listen_addr), sizeof(listen_addr));

	// 접속 요청 받기
	listen(accept_socket, 10); 

	// listen
	while (g_isshutdown == FALSE)
	{
		// 클라이언트 주소 정보
		struct sockaddr_in client_addr;
		int addr_size = sizeof(client_addr);

		// accept
		SOCKET new_client = WSAAccept(accept_socket,
			reinterpret_cast<sockaddr *>(&client_addr), &addr_size, NULL, NULL);

		// 소켓 에러
		if (new_client == INVALID_SOCKET) {
			int error_no = WSAGetLastError();
			error_display("Accept::WSAAccept", error_no);
			while (TRUE);
		}
		else
		{
			// 소켓 성공
			std::cout << "server connect success" << std::endl;
		}

		// 클라이언트에게 ID를 할당
		int new_id = -1;
		for (auto i = 0; i < MAX_USER; ++i) {
			if (FALSE == g_clients[i].is_connected) {
				new_id = i;
				std::cout << new_id << std::endl;
				break;
			}
		}

		// 클라이언트가 가득 차 더 이상 ID를 할당 받을 수 없을 때,
		if (new_id == -1) {
			std::cout << "Max Concurrent User excceded!\n";
			closesocket(new_client);
			continue;
		}

		// 조건 인원이 접속 했을 때 게임상태 바꾸기
		if (new_id >= MAX_USER - 1)
		{
			assert(g_StateMgr.GetCurrState() == GameState::GameWaiting);
			g_StateMgr.FlowAdvance();
			std::cout << "Entering WaveWaiting state..." << std::endl;
		}

		// 클라이언트 정보 초기화
		g_clients[new_id].s = new_client;
		g_clients[new_id].avatar.Pos.x = 0.0f;
		g_clients[new_id].avatar.Pos.y = 0.0f;
		g_clients[new_id].avatar.Pos.z = 0.0f;
		g_clients[new_id].avatar.Rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_clients[new_id].avatar.Scale = 0.0f;
		g_clients[new_id].packet_size = 0;
		g_clients[new_id].previous_size = 0;
		memset(&g_clients[new_id].recv_overlap.original_Overlap, 0,
			sizeof(g_clients[new_id].recv_overlap.original_Overlap));

		// IOCP 연결
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(new_client), g_hIocp, new_id, 0);

		// 플레이어 추가 
		sc_packet_put_player enter_packet;
		enter_packet.size = sizeof(enter_packet);
		enter_packet.type = SC_PUT_PLAYER;
		enter_packet.client_id = new_id;
		enter_packet.cInfo.Pos.x = g_clients[new_id].avatar.Pos.x;
		enter_packet.cInfo.Pos.y = g_clients[new_id].avatar.Pos.y;
		enter_packet.cInfo.Pos.z = g_clients[new_id].avatar.Pos.z;
		enter_packet.cInfo.Rot = g_clients[new_id].avatar.Rot;
		enter_packet.cInfo.Scale = g_clients[new_id].avatar.Scale;

		// 접속한 본인에게 초기 정보를 전송한다.
		MyServer::Send_Packet(new_id, reinterpret_cast<unsigned char *>(&enter_packet));

		// 이미 접속한 플레이어들에게 새로운 플레이어가 들어왔다는 패킷을 전송
		for (int i = 0; i < MAX_USER; ++i) {
			if (i == new_id) continue; // 본인을 제외한
			if (TRUE == g_clients[i].is_connected) // 접속한 플레이어들에게 패킷 전송 
				MyServer::Send_Packet(i, reinterpret_cast<unsigned char *>(&enter_packet));
		}

		// 이미 접속한 플레이어들에 대한 정보 전송
		for (int i = 0; i < MAX_USER; ++i)
		{
			// 접속한 유저가 처음 접속한 유저라면, for문을 돌 필요가 없으므로, break.
			if (0 == new_id) break;
			if (i == new_id) continue;
			if (FALSE == g_clients[i].is_connected) continue;

			sc_packet_put_player ex_enter_packet;
			ex_enter_packet.size = sizeof(ex_enter_packet);
			ex_enter_packet.type = SC_PUT_PLAYER;
			ex_enter_packet.client_id = i;
			ex_enter_packet.cInfo.Pos.x = g_clients[i].avatar.Pos.x;
			ex_enter_packet.cInfo.Pos.y = g_clients[i].avatar.Pos.y;
			ex_enter_packet.cInfo.Pos.z = g_clients[i].avatar.Pos.z;
			ex_enter_packet.cInfo.Rot = g_clients[i].avatar.Rot;
			ex_enter_packet.cInfo.Scale = g_clients[i].avatar.Scale;

			// 플레이어 본인에게, 이미 접속한 유저들에 대한 정보를 전송한다.
			MyServer::Send_Packet(new_id, reinterpret_cast<unsigned char*>(&ex_enter_packet));
		}

		// 클라이언트 연결 정보 갱신
		g_clients[new_id].is_connected = TRUE;

		// 연속되는 작업을 위한 WSARecv 호출
		DWORD flags = 0;
		int ret = WSARecv(new_client, &g_clients[new_id].recv_overlap.wsabuf, 1, NULL,
			&flags, &g_clients[new_id].recv_overlap.original_Overlap, NULL);

		// 오류 처리
		if (0 != ret) {
			int error_no = WSAGetLastError();
			if (WSA_IO_PENDING != error_no)
				error_display("Accept:WSARecv()", error_no);
		}
	}
}

// 워커 스레드
void MyThreads::Worker_Thread()
{
	while (g_isshutdown == FALSE)
	{
		DWORD iosize;			// 전송된 데이터의 크기
		DWORD key;				// Completion Key : 현재 넘겨받는 데이터는 플레이어의 ID값
		Overlap_ex *my_overlap; // Ex_Overlap 구조체

		BOOL result = GetQueuedCompletionStatus(g_hIocp, &iosize, &key,
			reinterpret_cast<LPOVERLAPPED *>(&my_overlap), INFINITE);

		if (FALSE == result)
		{
			// 에러 처리
		}

		// 전송된 데이터가 0이라는 것은 접속을 종료 했다는 것을 의미
		if (0 == iosize)
		{
			// 소켓 종료
			closesocket(g_clients[key].s);

			// 접속을 종료 했다는 것을 다른 플레이어들에게 알리기 위한 패킷
			sc_packet_remove_player disconnect;
			disconnect.type = SC_REMOVE_PLAYER;
			disconnect.size = sizeof(disconnect);
			disconnect.client_id = key;

			// 현재 접속 중인 모든 플레이어들에게 패킷을 전송
			for (auto i = 0; i < MAX_USER; ++i) {
				if (FALSE == g_clients[i].is_connected) continue; // 접속x
				if (key == i) continue; // 본인은 제외하고
				MyServer::Send_Packet(i, reinterpret_cast<unsigned char*>(&disconnect));
			}
			g_clients[key].is_connected = FALSE;
		}

		if (OP_RECV == my_overlap->operation) { // 리시브 일때
			unsigned char *buf_ptr = g_clients[key].recv_overlap.iocp_buffer; // IOCP 버퍼
			int remained = iosize; // 처리할 양
			while (0 < remained) { // 처리 할 양이 있는 경우
				if (0 == g_clients[key].packet_size) // 새로 패킷이 넘어오면
					g_clients[key].packet_size = buf_ptr[0]; // 전송된 패킷의 사이즈를 저장

				int required = g_clients[key].packet_size - g_clients[key].previous_size; // 요구량 = 패킷 사이즈 - 이전에 넘어온 양

				if (remained >= required) { // 처리할 양 >= 요구량 -> 패킷 완성 가능
											// 패킷버퍼+이전에 넘어온 량 <- 요구량
					memcpy(g_clients[key].packet_buff + g_clients[key].previous_size, buf_ptr, required);
					auto resultPacket = MyServer::Pharse_Packet(key, g_clients[key].packet_buff); // 패킷 처리 함수 호출
					MyServer::Send_Packet(key, resultPacket);
					buf_ptr += required; // 이번에 처리한 양 만큼 포인터 이동
					remained -= required; // 전체 패킷사이즈 - 처리한 양
										  // 패킷 처리를 완료함
					g_clients[key].packet_size = 0;
					g_clients[key].previous_size = 0;
				}
				else { // 전송이 덜됨
					   // 패킷버퍼 + 이전 패킷 사이즈 <- 
					memcpy(g_clients[key].packet_buff + g_clients[key].previous_size, buf_ptr, remained);
					buf_ptr += remained; // 작업한만큼 포인터 이동
					g_clients[key].previous_size += remained; // 작업한 양
					remained = 0; // 다 처리했음
				}
			}
			DWORD flags = 0;
			// (소켓, WSABUF구조체 포인터, WSABUF구조체 개수, NULL, 받아온 양, 오버랩 구조체의 포인터, 완료루틴의 포인터)
			WSARecv(g_clients[key].s, &g_clients[key].recv_overlap.wsabuf, 1, NULL, &flags,
				&g_clients[key].recv_overlap.original_Overlap, NULL);
		}
		else if (OP_SEND == my_overlap->operation) { // 샌드 일때
			delete my_overlap; // 확장 오버랩 구조체 메모리 반환
		}
		else {
			std::cout << "Unknown IOCP event!\n";
			exit(-1);
		}
	}
}
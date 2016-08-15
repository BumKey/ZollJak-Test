#include "Threads.h"

ServerRogicMgr	g_RogicMgr;

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
		SOCKET newSocket = WSAAccept(accept_socket,
			reinterpret_cast<sockaddr *>(&client_addr), &addr_size, NULL, NULL);

		// 소켓 에러
		if (newSocket == INVALID_SOCKET) {
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
			if (g_clients[i].is_connected == false) {
				new_id = i;
				g_clients[i].is_connected = true;
				std::cout << "New client is connected, ID : " << new_id << std::endl;
				break;
			}
		}

		// 클라이언트가 가득 차 더 이상 ID를 할당 받을 수 없을 때,
		if (new_id == -1) {
			std::cout << "Max Concurrent User excceded!\n";
			closesocket(newSocket);
			continue;
		}

		// IOCP 연결
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(newSocket), g_hIocp, new_id, 0);

		g_RogicMgr.AddPlayer(newSocket, ObjectType::Warrior, new_id);

		// 클라이언트 연결 정보 갱신
		g_clients[new_id].is_connected = TRUE;
		g_RogicMgr.UnLock(new_id);

		// 연속되는 작업을 위한 WSARecv 호출
		DWORD flags = 0;
		int ret = WSARecv(newSocket, &g_clients[new_id].recv_overlap.wsabuf, 1, NULL,
			&flags, &g_clients[new_id].recv_overlap.original_Overlap, NULL);

		// 오류 처리
		if (ret != 0) {
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
		DWORD id;				// Completion id : 현재 넘겨받는 데이터는 플레이어의 ID값
		Overlap_ex *my_overlap; // Ex_Overlap 구조체

		BOOL result = GetQueuedCompletionStatus(g_hIocp, &iosize, &id,
			reinterpret_cast<LPOVERLAPPED *>(&my_overlap), INFINITE);

		if (FALSE == result)
		{
			// 에러 처리
		}

		// 전송된 데이터가 0이라는 것은 접속을 종료 했다는 것을 의미
		if (iosize == 0)
		{
			g_RogicMgr.RemovePlayer(id);
			// 소켓 종료
			closesocket(g_clients[id].socket);
			g_clients[id].is_connected = false;
		}
		else if (my_overlap->operation == OP_RECV) { // 리시브 일때
			char *buf_ptr = g_clients[id].recv_overlap.iocp_buffer; // IOCP 버퍼
			int remained = iosize; // 처리할 양
			while (0 < remained) { // 처리 할 양이 있는 경우
				if (0 == g_clients[id].packet_size) // 새로 패킷이 넘어오면
					g_clients[id].packet_size = buf_ptr[0]; // 전송된 패킷의 사이즈를 저장

				int required = g_clients[id].packet_size - g_clients[id].previous_size; // 요구량 = 패킷 사이즈 - 이전에 넘어온 양

				if (remained >= required) { // 처리할 양 >= 요구량 -> 패킷 완성 가능
											// 패킷버퍼+이전에 넘어온 량 <- 요구량
					memcpy(g_clients[id].packet_buff + g_clients[id].previous_size, buf_ptr, required);
					MyServer::Process_Packet(g_clients[id].packet_buff, g_RogicMgr); // 패킷 처리 함수 호출
					//MyServer::Send_Packet(id, g_clients[id].packet_buff);
					buf_ptr += required; // 이번에 처리한 양 만큼 포인터 이동
					remained -= required; // 전체 패킷사이즈 - 처리한 양
										  // 패킷 처리를 완료함
					g_clients[id].packet_size = 0;
					g_clients[id].previous_size = 0;
				}
				else { // 전송이 덜됨
					   // 패킷버퍼 + 이전 패킷 사이즈 <- 
					memcpy(g_clients[id].packet_buff + g_clients[id].previous_size, buf_ptr, remained);
					buf_ptr += remained; // 작업한만큼 포인터 이동
					g_clients[id].previous_size += remained; // 작업한 양
					remained = 0; // 다 처리했음
				}
			}
	
			/* ==================================================================================================
			이 워크쓰레드는 클라쪽에서 패킷이 왔을 때 실행된다고 가정.

			현재 서버가 패킷을 보내는 구조는 
			서버 로직으로 가지고있는 데이터들을 갱신하고
			그 갱신된 데이터들을 접속된 모든 클라들한테서 잘 받았다는
			답신 패킷을 받았을 때를 한 프레임으로 설정.
			
			답신 패킷을 모두 받은 후엔 다음 프레임을 수행한다.
			자세한 사항은 ServerRogicMgr의 주석 참조.

			위에 사항에 비효율적이거나 잘못된 부분 있다면 연락바람.

			-----------------------------------------------------------------
			현재 ServerRogicMgr의 사용예)

			KeyInput타입의 패킷이 왔다면 cs_packet_move 구조체 채우고
			g_RogicMgr.ProcessidInput(cs_packet_move);

			MouseInput타입의 패킷이 왔다면 cs_packet_attack 구조체 채우고
			g_RogicMgr.ProcessMouseInput(cs_packet_attack);

			CS_SUCCES 패킷이 왔다면 <이것은 항상 와야함>
			g_RogicMgr.Update() 수행.
			-----------------------------------------------------------------

			변동 사항은 테스트를 해봐야 알 수 있을 것 같음.
			================================================================================================== */

			DWORD flags = 0;
			// (소켓, WSABUF구조체 포인터, WSABUF구조체 개수, NULL, 받아온 양, 오버랩 구조체의 포인터, 완료루틴의 포인터)
			WSARecv(g_clients[id].socket, &g_clients[id].recv_overlap.wsabuf, 1, NULL, &flags,
				&g_clients[id].recv_overlap.original_Overlap, NULL);
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
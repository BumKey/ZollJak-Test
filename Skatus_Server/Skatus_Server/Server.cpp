#pragma once
#include "server.h"

HANDLE Server::g_hIocp;				// IOCP 핸들
Client Server::clients[MAX_OBJECT]; // 클라이언트의 정보를 가지고 오는 구조체
BOOL Server::g_isshutdown = FALSE;

// Socket 관련 에러 처리 함수
void error_display(char *msg, int err_no)
{
	WCHAR *lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("%s", msg);
	wprintf(L"에러%s\n", lpMsgBuf);
	LocalFree(lpMsgBuf);
}

// 서버 생성
Server::Server() {

	// 서버 초기화
	Initialize();

	// Load to Window Socket 2.2
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	// 한글을 사용하기 위한 환경 설정 함수
	_wsetlocale(LC_ALL, L"korean");

	// IOCP 생성
	g_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);

	// 해당 PC의 CPU 개수 확인
	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);

	// 워커 스레드 생성
	for (auto i = 0; i < systemInfo.dwNumberOfProcessors; ++i)
		worker_threads.push_back(new std::thread{ Worker_Thread });

	// 어셉트 스레드 생성
	std::thread accept_threads{ Accept_Thread };

	while (false == g_isshutdown) {
		Sleep(1000);
	}
}

// 서버 초기화 함수
void Server::Initialize()
{
	// 오브젝트의 수만큼 루프를 돌면서, 구조체의 정보들을 초기화한다.
	for (auto i = 0; i < MAX_OBJECT; ++i) {
		clients[i].is_connected = FALSE; // 접속x
		clients[i].recv_overlap.operation = OP_RECV;
		clients[i].recv_overlap.wsabuf.buf = reinterpret_cast<CHAR *>(clients[i].recv_overlap.iocp_buffer);
		clients[i].recv_overlap.wsabuf.len = sizeof(clients[i].recv_overlap.iocp_buffer);
	}
}

// 어셉트 스레드
void Server::Accept_Thread()
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
	::bind(accept_socket, reinterpret_cast<sockaddr *>(&listen_addr), sizeof(listen_addr));

	// 접속 요청 받기
	listen(accept_socket, 10);

	// listen
	while (FALSE == g_isshutdown)
	{
		// 클라이언트 주소 정보
		struct sockaddr_in client_addr;
		int addr_size = sizeof(client_addr);

		// accept
		SOCKET new_client = WSAAccept(accept_socket,
			reinterpret_cast<sockaddr *>(&client_addr), &addr_size, NULL, NULL);

		// 소켓 에러
		if (INVALID_SOCKET == new_client) {
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
			if (FALSE == clients[i].is_connected) {
				new_id = i;
				std::cout << new_id << std::endl;
				break;
			}
		}

		// 클라이언트가 가득 차 더 이상 ID를 할당 받을 수 없을 때,
		if (-1 == new_id) {
			std::cout << "Max Concurrent User excceded!\n";
			closesocket(new_client);
			continue;
		}

		// 클라이언트 정보 초기화
		clients[new_id].s = new_client;
		clients[new_id].avatar.pos.x = 0.0f;
		clients[new_id].avatar.pos.y = 0.0f;
		clients[new_id].avatar.pos.z = 0.0f;
		clients[new_id].avatar.rot = 0.0f;
		clients[new_id].avatar.scale = 0.0f;
		clients[new_id].packet_size = 0;
		clients[new_id].previous_size = 0;
		memset(&clients[new_id].recv_overlap.original_Overlap, 0,
			sizeof(clients[new_id].recv_overlap.original_Overlap));

		// IOCP 연결
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(new_client), g_hIocp, new_id, 0);

		// 플레이어 추가 
		sc_packet_put_player enter_packet;
		enter_packet.size = sizeof(enter_packet);
		enter_packet.type = SC_PUT_PLAYER;
		enter_packet.client_id = new_id;
		enter_packet.cInfo.pos.x = clients[new_id].avatar.pos.x;
		enter_packet.cInfo.pos.y = clients[new_id].avatar.pos.y;
		enter_packet.cInfo.pos.z = clients[new_id].avatar.pos.z;
		enter_packet.cInfo.rot = clients[new_id].avatar.rot;
		enter_packet.cInfo.scale = clients[new_id].avatar.scale;

		// 접속한 본인에게 초기 정보를 전송한다.
		Send_Packet(new_id, reinterpret_cast<unsigned char *>(&enter_packet));

		// 이미 접속한 플레이어들에게 새로운 플레이어가 들어왔다는 패킷을 전송
		for (int i = 0; i < MAX_USER; ++i) {
			if (i == new_id) continue; // 본인을 제외한
			if (TRUE == clients[i].is_connected) // 접속한 플레이어들에게 패킷 전송 
				Send_Packet(i, reinterpret_cast<unsigned char *>(&enter_packet));
		}

		// 이미 접속한 플레이어들에 대한 정보 전송
		for (int i = 0; i < MAX_USER; ++i)
		{
			// 접속한 유저가 처음 접속한 유저라면, for문을 돌 필요가 없으므로, break.
			if (0 == new_id) break;
			if (i == new_id) continue;
			if (FALSE == clients[i].is_connected) continue;

			sc_packet_put_player ex_enter_packet;
			ex_enter_packet.size = sizeof(ex_enter_packet);
			ex_enter_packet.type = SC_PUT_PLAYER;
			ex_enter_packet.client_id = i;
			ex_enter_packet.cInfo.pos.x = clients[i].avatar.pos.x;
			ex_enter_packet.cInfo.pos.y = clients[i].avatar.pos.y;
			ex_enter_packet.cInfo.pos.z = clients[i].avatar.pos.z;
			ex_enter_packet.cInfo.rot = clients[i].avatar.rot;
			ex_enter_packet.cInfo.scale = clients[i].avatar.scale;

			// 플레이어 본인에게, 이미 접속한 유저들에 대한 정보를 전송한다.
			Send_Packet(new_id, reinterpret_cast<unsigned char*>(&ex_enter_packet));
		}

		// 클라이언트 연결 정보 갱신
		clients[new_id].is_connected = TRUE;

		// 연속되는 작업을 위한 WSARecv 호출
		DWORD flags = 0;
		int ret = WSARecv(new_client, &clients[new_id].recv_overlap.wsabuf, 1, NULL,
			&flags, &clients[new_id].recv_overlap.original_Overlap, NULL);

		// 오류 처리
		if (0 != ret) {
			int error_no = WSAGetLastError();
			if (WSA_IO_PENDING != error_no)
				error_display("Accept:WSARecv()", error_no);
		}
	}
}

// 워커 스레드
void Server::Worker_Thread()
{
	while (FALSE == g_isshutdown)
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
			closesocket(clients[key].s);

			// 접속을 종료 했다는 것을 다른 플레이어들에게 알리기 위한 패킷
			sc_packet_remove_player disconnect;
			disconnect.type = SC_REMOVE_PLAYER;
			disconnect.size = sizeof(disconnect);
			disconnect.client_id = key;

			// 현재 접속 중인 모든 플레이어들에게 패킷을 전송
			for (auto i = 0; i < MAX_USER; ++i) {
				if (FALSE == clients[i].is_connected) continue; // 접속x
				if (key == i) continue; // 본인은 제외하고
				Send_Packet(i, reinterpret_cast<unsigned char*>(&disconnect));
			}
			clients[key].is_connected = FALSE;
		}

		if (OP_RECV == my_overlap->operation) { // 리시브 일때
			unsigned char *buf_ptr = clients[key].recv_overlap.iocp_buffer; // IOCP 버퍼
			int remained = iosize; // 처리할 양
			while (0 < remained) { // 처리 할 양이 있는 경우
				if (0 == clients[key].packet_size) // 새로 패킷이 넘어오면
					clients[key].packet_size = buf_ptr[0]; // 전송된 패킷의 사이즈를 저장

				int required = clients[key].packet_size - clients[key].previous_size; // 요구량 = 패킷 사이즈 - 이전에 넘어온 양

				if (remained >= required) { // 처리할 양 >= 요구량 -> 패킷 완성 가능
											// 패킷버퍼+이전에 넘어온 량 <- 요구량
					memcpy(clients[key].packet_buff + clients[key].previous_size, buf_ptr, required);
					Process_Packet(key, clients[key].packet_buff); // 패킷 처리 함수 호출
					buf_ptr += required; // 이번에 처리한 양 만큼 포인터 이동
					remained -= required; // 전체 패킷사이즈 - 처리한 양
										  // 패킷 처리를 완료함
					clients[key].packet_size = 0;
					clients[key].previous_size = 0;
				}
				else { // 전송이 덜됨
					   // 패킷버퍼 + 이전 패킷 사이즈 <- 
					memcpy(clients[key].packet_buff + clients[key].previous_size, buf_ptr, remained);
					buf_ptr += remained; // 작업한만큼 포인터 이동
					clients[key].previous_size += remained; // 작업한 양
					remained = 0; // 다 처리했음
				}
			}
			DWORD flags = 0;
			// (소켓, WSABUF구조체 포인터, WSABUF구조체 개수, NULL, 받아온 양, 오버랩 구조체의 포인터, 완료루틴의 포인터)
			WSARecv(clients[key].s, &clients[key].recv_overlap.wsabuf, 1, NULL, &flags,
				&clients[key].recv_overlap.original_Overlap, NULL);
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

// 패킷 전송 함수
void Server::Send_Packet(DWORD id, unsigned char *packet) {
	Overlap_ex *over = new Overlap_ex; // 동적 할당
	memset(over, 0, sizeof(Overlap_ex)); // 메모리 초기화
	over->operation = OP_SEND;
	over->wsabuf.buf = reinterpret_cast<CHAR *>(over->iocp_buffer); //
	over->wsabuf.len = packet[0]; // 패킷의 첫칸 = 사이즈
	memcpy(over->iocp_buffer, packet, packet[0]); // IOCP버퍼에 패킷 사이즈를 복사

												  // (소켓, WSABUF구조체 포인터, WSABUF구조체 개수, NULL, 호출 방식, 오버랩 구조체의 포인터, 완료루틴의 포인터)
	int ret = WSASend(clients[id].s, &over->wsabuf, 1, NULL, 0, &over->original_Overlap, NULL);
	// 에러 처리
	if (0 != ret) { // 데이터 전송을 완료했다면 0
		int err_no = WSAGetLastError();
		error_display("Send_Packet:WSASend", err_no);
		while (TRUE);
	}
}

// 패킷 처리 함수
void Server::Process_Packet(DWORD id, unsigned char buf[]) {

	switch (buf[1])
	{
	case CS_TEST:
	{
		ForClientInfo info = *(reinterpret_cast<ForClientInfo*>(&buf[2]));
		std::cout << "success : " << info.pos.x << " " << info.pos.y << " " << info.pos.z << std::endl;
		break;
	}
	case CS_SUCCESS:
		std::cout << "success, data value : " << buf[2] << std::endl;
		break;
	default:
		break;
	}
	
	cs_packet packet;
	packet.cInfo.pos.x = 0.0;
	packet.cInfo.pos.y = 0.0;
	packet.cInfo.pos.z = 0.0;
	packet.cInfo.rot = 0.0;
	packet.cInfo.scale = 0.0;
	packet.size = sizeof(packet);
	packet.type = CS_TEST;

	Send_Packet(0, reinterpret_cast<unsigned char*>(&packet));
}

// 서버 소멸
Server::~Server() {
	// 쓰레드 종료
	for (auto& thread : worker_threads) thread->join();
	accept_threads->join();

	// 쓰레드 제거
	for (auto& data : worker_threads) delete data;
	delete accept_threads;

	WSACleanup();
}
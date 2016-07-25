#pragma once
#include "Server.h"

void MyServer::Initialize()
{
	WSADATA wsadata;

	// Load to Window Socket 2.2
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	// 한글을 사용하기 위한 환경 설정 함수
	_wsetlocale(LC_ALL, L"korean");

	// IOCP 생성
	g_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);

	// 오브젝트의 수만큼 루프를 돌면서, 구조체의 정보들을 초기화한다.
	for (auto i = 0; i < MAX_USER; ++i) {
		g_clients[i].is_connected = FALSE; // 접속x
		g_clients[i].recv_overlap.operation = OP_RECV;
		g_clients[i].recv_overlap.wsabuf.buf = reinterpret_cast<CHAR *>(g_clients[i].recv_overlap.iocp_buffer);
		g_clients[i].recv_overlap.wsabuf.len = sizeof(g_clients[i].recv_overlap.iocp_buffer);
	}
}

// 패킷 전송 함수
void MyServer::Send_Packet(DWORD id, unsigned char *packet) {
	Overlap_ex *over = new Overlap_ex; // 동적 할당
	memset(over, 0, sizeof(Overlap_ex)); // 메모리 초기화
	over->operation = OP_SEND;
	over->wsabuf.buf = reinterpret_cast<CHAR *>(over->iocp_buffer); //
	over->wsabuf.len = packet[0]; // 패킷의 첫칸 = 사이즈
	memcpy(over->iocp_buffer, packet, packet[0]); // IOCP버퍼에 패킷 사이즈를 복사

				  // (소켓, WSABUF구조체 포인터, WSABUF구조체 개수, NULL, 호출 방식, 오버랩 구조체의 포인터, 완료루틴의 포인터)
	int ret = WSASend(g_clients[id].s, &over->wsabuf, 1, NULL, 0, &over->original_Overlap, NULL);
	// 에러 처리
	if (0 != ret) { // 데이터 전송을 완료했다면 0
		int err_no = WSAGetLastError();
		error_display("Send_Packet:WSASend", err_no);
		while (TRUE);
	}
}

// 패킷 처리 함수
void MyServer::Pharse_Packet(DWORD id, unsigned char buf[]) 
{
	switch (buf[1])
	{
	case CS_SUCCESS:
		std::cout << "CS_SUCCESS, ID : " << id << std::endl;
		break;

	case CS_KEYINPUT: {
		auto packet = reinterpret_cast<cs_packet_move*>(buf);
		std::cout << "CS_KEYINPUT, POS : " << (*packet).pos.x << ", "
			<< (*packet).pos.y << ", " << (*packet).pos.z << std::endl;
		break;
	}
	default:
		break;
	}
}

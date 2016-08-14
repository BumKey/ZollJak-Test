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
void MyServer::Send_Packet(DWORD id, char *packet) {
	Overlap_ex *over = new Overlap_ex; // 동적 할당
	memset(over, 0, sizeof(Overlap_ex)); // 메모리 초기화
	over->operation = OP_SEND;

	HEADER *header = reinterpret_cast<HEADER*>(packet);
	over->wsabuf.buf = over->iocp_buffer; 
	over->wsabuf.len = header->Size;
	memcpy(over->iocp_buffer, packet, header->Size); // IOCP버퍼에 패킷 사이즈를 복사
	std::cout << "SendPacket: " << header->Type << std::endl;

				  // (소켓, WSABUF구조체 포인터, WSABUF구조체 개수, NULL, 호출 방식, 오버랩 구조체의 포인터, 완료루틴의 포인터)
	int ret = WSASend(g_clients[id].socket, &over->wsabuf, 1, NULL, 0, &over->original_Overlap, NULL);
	// 에러 처리
	if (ret != 0) { // 데이터 전송을 완료했다면 0
		int err_no = WSAGetLastError();
		error_display("Send_Packet:WSASend", err_no);
		while (TRUE);
	}
}

// 패킷 처리 함수
void MyServer::Process_Packet(char* packet, ServerRogicMgr& rogicMgr)
{
	HEADER *header = reinterpret_cast<HEADER*>(packet);
	switch (header->Type)
	{
	case eCS::Success: {
		auto info = reinterpret_cast<CS_Success*>(packet);
		std::cout << "CS_SUCCESS, ID : " << (int)info->ClientID << std::endl;
		rogicMgr.Update();
		break;
	}
	case eCS::KeyInput: {
		auto info = reinterpret_cast<CS_Move*>(packet);
		std::cout << "CS_KEYINPUT, POS : " << info->Pos.x << ", "
			<< info->Pos.y << ", " << info->Pos.z << std::endl;
		rogicMgr.ProcessKeyInput(*info);
		break;
	}
	case eCS::MouseInput: {
		auto info = reinterpret_cast<CS_Attack*>(packet);
		std::cout << "CS_MOUSEINPUT, ID : " << info->ClientID << std::endl;
		rogicMgr.ProcessMouseInput(*info);
		break;
	}
	default:
		break;
	}
}

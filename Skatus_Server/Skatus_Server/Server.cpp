#pragma once
#include "Server.h"

void MyServer::Initialize()
{
	WSADATA wsadata;

	// Load to Window Socket 2.2
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	// �ѱ��� ����ϱ� ���� ȯ�� ���� �Լ�
	_wsetlocale(LC_ALL, L"korean");

	// IOCP ����
	g_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);

	// ������Ʈ�� ����ŭ ������ ���鼭, ����ü�� �������� �ʱ�ȭ�Ѵ�.
	for (auto i = 0; i < MAX_USER; ++i) {
		g_clients[i].is_connected = FALSE; // ����x
		g_clients[i].recv_overlap.operation = OP_RECV;
		g_clients[i].recv_overlap.wsabuf.buf = reinterpret_cast<CHAR *>(g_clients[i].recv_overlap.iocp_buffer);
		g_clients[i].recv_overlap.wsabuf.len = sizeof(g_clients[i].recv_overlap.iocp_buffer);
	}
}

// ��Ŷ ���� �Լ�
void MyServer::Send_Packet(DWORD id, char *packet) {
	Overlap_ex *over = new Overlap_ex; // ���� �Ҵ�
	memset(over, 0, sizeof(Overlap_ex)); // �޸� �ʱ�ȭ
	over->operation = OP_SEND;

	HEADER *header = reinterpret_cast<HEADER*>(packet);
	over->wsabuf.buf = over->iocp_buffer; 
	over->wsabuf.len = header->Size;
	memcpy(over->iocp_buffer, packet, header->Size); // IOCP���ۿ� ��Ŷ ����� ����
	std::cout << "SendPacket: " << header->Type << std::endl;

				  // (����, WSABUF����ü ������, WSABUF����ü ����, NULL, ȣ�� ���, ������ ����ü�� ������, �Ϸ��ƾ�� ������)
	int ret = WSASend(g_clients[id].socket, &over->wsabuf, 1, NULL, 0, &over->original_Overlap, NULL);
	// ���� ó��
	if (ret != 0) { // ������ ������ �Ϸ��ߴٸ� 0
		int err_no = WSAGetLastError();
		error_display("Send_Packet:WSASend", err_no);
		while (TRUE);
	}
}

// ��Ŷ ó�� �Լ�
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

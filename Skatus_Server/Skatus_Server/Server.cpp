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
void MyServer::Send_Packet(DWORD id, unsigned char *packet) {
	Overlap_ex *over = new Overlap_ex; // ���� �Ҵ�
	memset(over, 0, sizeof(Overlap_ex)); // �޸� �ʱ�ȭ
	over->operation = OP_SEND;
	over->wsabuf.buf = reinterpret_cast<CHAR *>(over->iocp_buffer); //
	over->wsabuf.len = packet[0]; // ��Ŷ�� ùĭ = ������
	memcpy(over->iocp_buffer, packet, packet[0]); // IOCP���ۿ� ��Ŷ ����� ����

				  // (����, WSABUF����ü ������, WSABUF����ü ����, NULL, ȣ�� ���, ������ ����ü�� ������, �Ϸ��ƾ�� ������)
	int ret = WSASend(g_clients[id].s, &over->wsabuf, 1, NULL, 0, &over->original_Overlap, NULL);
	// ���� ó��
	if (0 != ret) { // ������ ������ �Ϸ��ߴٸ� 0
		int err_no = WSAGetLastError();
		error_display("Send_Packet:WSASend", err_no);
		while (TRUE);
	}
}

// ��Ŷ ó�� �Լ�
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

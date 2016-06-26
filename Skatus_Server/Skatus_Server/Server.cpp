#pragma once
#include "server.h"

HANDLE Server::g_hIocp;				// IOCP �ڵ�
Client Server::clients[MAX_OBJECT]; // Ŭ���̾�Ʈ�� ������ ������ ���� ����ü
BOOL Server::g_isshutdown = FALSE;

// Socket ���� ���� ó�� �Լ�
void error_display(char *msg, int err_no)
{
	WCHAR *lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("%s", msg);
	wprintf(L"����%s\n", lpMsgBuf);
	LocalFree(lpMsgBuf);
}

// ���� ����
Server::Server() {

	// ���� �ʱ�ȭ
	Initialize();

	// Load to Window Socket 2.2
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	// �ѱ��� ����ϱ� ���� ȯ�� ���� �Լ�
	_wsetlocale(LC_ALL, L"korean");

	// IOCP ����
	g_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);

	// �ش� PC�� CPU ���� Ȯ��
	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);

	// ��Ŀ ������ ����
	for (auto i = 0; i < systemInfo.dwNumberOfProcessors; ++i)
		worker_threads.push_back(new std::thread{ Worker_Thread });

	// ���Ʈ ������ ����
	std::thread accept_threads{ Accept_Thread };

	while (false == g_isshutdown) {
		Sleep(1000);
	}
}

// ���� �ʱ�ȭ �Լ�
void Server::Initialize()
{
	// ������Ʈ�� ����ŭ ������ ���鼭, ����ü�� �������� �ʱ�ȭ�Ѵ�.
	for (auto i = 0; i < MAX_OBJECT; ++i) {
		clients[i].is_connected = FALSE; // ����x
		clients[i].recv_overlap.operation = OP_RECV;
		clients[i].recv_overlap.wsabuf.buf = reinterpret_cast<CHAR *>(clients[i].recv_overlap.iocp_buffer);
		clients[i].recv_overlap.wsabuf.len = sizeof(clients[i].recv_overlap.iocp_buffer);
	}
}

// ���Ʈ ������
void Server::Accept_Thread()
{
	// ���� �ּ� ���� �ʱ�ȭ
	struct sockaddr_in listen_addr;
	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = htonl(ADDR_ANY);
	listen_addr.sin_port = htons(SERVER_PORT);

	// ���Ʈ ���� ����
	SOCKET accept_socket = WSASocket(AF_INET, SOCK_STREAM,
		IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	// bind
	::bind(accept_socket, reinterpret_cast<sockaddr *>(&listen_addr), sizeof(listen_addr));

	// ���� ��û �ޱ�
	listen(accept_socket, 10);

	// listen
	while (FALSE == g_isshutdown)
	{
		// Ŭ���̾�Ʈ �ּ� ����
		struct sockaddr_in client_addr;
		int addr_size = sizeof(client_addr);

		// accept
		SOCKET new_client = WSAAccept(accept_socket,
			reinterpret_cast<sockaddr *>(&client_addr), &addr_size, NULL, NULL);

		// ���� ����
		if (INVALID_SOCKET == new_client) {
			int error_no = WSAGetLastError();
			error_display("Accept::WSAAccept", error_no);
			while (TRUE);
		}
		else
		{
			// ���� ����
			std::cout << "server connect success" << std::endl;
		}

		// Ŭ���̾�Ʈ���� ID�� �Ҵ�
		int new_id = -1;
		for (auto i = 0; i < MAX_USER; ++i) {
			if (FALSE == clients[i].is_connected) {
				new_id = i;
				std::cout << new_id << std::endl;
				break;
			}
		}

		// Ŭ���̾�Ʈ�� ���� �� �� �̻� ID�� �Ҵ� ���� �� ���� ��,
		if (-1 == new_id) {
			std::cout << "Max Concurrent User excceded!\n";
			closesocket(new_client);
			continue;
		}

		// Ŭ���̾�Ʈ ���� �ʱ�ȭ
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

		// IOCP ����
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(new_client), g_hIocp, new_id, 0);

		// �÷��̾� �߰� 
		sc_packet_put_player enter_packet;
		enter_packet.size = sizeof(enter_packet);
		enter_packet.type = SC_PUT_PLAYER;
		enter_packet.client_id = new_id;
		enter_packet.cInfo.pos.x = clients[new_id].avatar.pos.x;
		enter_packet.cInfo.pos.y = clients[new_id].avatar.pos.y;
		enter_packet.cInfo.pos.z = clients[new_id].avatar.pos.z;
		enter_packet.cInfo.rot = clients[new_id].avatar.rot;
		enter_packet.cInfo.scale = clients[new_id].avatar.scale;

		// ������ ���ο��� �ʱ� ������ �����Ѵ�.
		Send_Packet(new_id, reinterpret_cast<unsigned char *>(&enter_packet));

		// �̹� ������ �÷��̾�鿡�� ���ο� �÷��̾ ���Դٴ� ��Ŷ�� ����
		for (int i = 0; i < MAX_USER; ++i) {
			if (i == new_id) continue; // ������ ������
			if (TRUE == clients[i].is_connected) // ������ �÷��̾�鿡�� ��Ŷ ���� 
				Send_Packet(i, reinterpret_cast<unsigned char *>(&enter_packet));
		}

		// �̹� ������ �÷��̾�鿡 ���� ���� ����
		for (int i = 0; i < MAX_USER; ++i)
		{
			// ������ ������ ó�� ������ �������, for���� �� �ʿ䰡 �����Ƿ�, break.
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

			// �÷��̾� ���ο���, �̹� ������ �����鿡 ���� ������ �����Ѵ�.
			Send_Packet(new_id, reinterpret_cast<unsigned char*>(&ex_enter_packet));
		}

		// Ŭ���̾�Ʈ ���� ���� ����
		clients[new_id].is_connected = TRUE;

		// ���ӵǴ� �۾��� ���� WSARecv ȣ��
		DWORD flags = 0;
		int ret = WSARecv(new_client, &clients[new_id].recv_overlap.wsabuf, 1, NULL,
			&flags, &clients[new_id].recv_overlap.original_Overlap, NULL);

		// ���� ó��
		if (0 != ret) {
			int error_no = WSAGetLastError();
			if (WSA_IO_PENDING != error_no)
				error_display("Accept:WSARecv()", error_no);
		}
	}
}

// ��Ŀ ������
void Server::Worker_Thread()
{
	while (FALSE == g_isshutdown)
	{
		DWORD iosize;			// ���۵� �������� ũ��
		DWORD key;				// Completion Key : ���� �Ѱܹ޴� �����ʹ� �÷��̾��� ID��
		Overlap_ex *my_overlap; // Ex_Overlap ����ü

		BOOL result = GetQueuedCompletionStatus(g_hIocp, &iosize, &key,
			reinterpret_cast<LPOVERLAPPED *>(&my_overlap), INFINITE);

		if (FALSE == result)
		{
			// ���� ó��
		}

		// ���۵� �����Ͱ� 0�̶�� ���� ������ ���� �ߴٴ� ���� �ǹ�
		if (0 == iosize)
		{
			// ���� ����
			closesocket(clients[key].s);

			// ������ ���� �ߴٴ� ���� �ٸ� �÷��̾�鿡�� �˸��� ���� ��Ŷ
			sc_packet_remove_player disconnect;
			disconnect.type = SC_REMOVE_PLAYER;
			disconnect.size = sizeof(disconnect);
			disconnect.client_id = key;

			// ���� ���� ���� ��� �÷��̾�鿡�� ��Ŷ�� ����
			for (auto i = 0; i < MAX_USER; ++i) {
				if (FALSE == clients[i].is_connected) continue; // ����x
				if (key == i) continue; // ������ �����ϰ�
				Send_Packet(i, reinterpret_cast<unsigned char*>(&disconnect));
			}
			clients[key].is_connected = FALSE;
		}

		if (OP_RECV == my_overlap->operation) { // ���ú� �϶�
			unsigned char *buf_ptr = clients[key].recv_overlap.iocp_buffer; // IOCP ����
			int remained = iosize; // ó���� ��
			while (0 < remained) { // ó�� �� ���� �ִ� ���
				if (0 == clients[key].packet_size) // ���� ��Ŷ�� �Ѿ����
					clients[key].packet_size = buf_ptr[0]; // ���۵� ��Ŷ�� ����� ����

				int required = clients[key].packet_size - clients[key].previous_size; // �䱸�� = ��Ŷ ������ - ������ �Ѿ�� ��

				if (remained >= required) { // ó���� �� >= �䱸�� -> ��Ŷ �ϼ� ����
											// ��Ŷ����+������ �Ѿ�� �� <- �䱸��
					memcpy(clients[key].packet_buff + clients[key].previous_size, buf_ptr, required);
					Process_Packet(key, clients[key].packet_buff); // ��Ŷ ó�� �Լ� ȣ��
					buf_ptr += required; // �̹��� ó���� �� ��ŭ ������ �̵�
					remained -= required; // ��ü ��Ŷ������ - ó���� ��
										  // ��Ŷ ó���� �Ϸ���
					clients[key].packet_size = 0;
					clients[key].previous_size = 0;
				}
				else { // ������ ����
					   // ��Ŷ���� + ���� ��Ŷ ������ <- 
					memcpy(clients[key].packet_buff + clients[key].previous_size, buf_ptr, remained);
					buf_ptr += remained; // �۾��Ѹ�ŭ ������ �̵�
					clients[key].previous_size += remained; // �۾��� ��
					remained = 0; // �� ó������
				}
			}
			DWORD flags = 0;
			// (����, WSABUF����ü ������, WSABUF����ü ����, NULL, �޾ƿ� ��, ������ ����ü�� ������, �Ϸ��ƾ�� ������)
			WSARecv(clients[key].s, &clients[key].recv_overlap.wsabuf, 1, NULL, &flags,
				&clients[key].recv_overlap.original_Overlap, NULL);
		}
		else if (OP_SEND == my_overlap->operation) { // ���� �϶�
			delete my_overlap; // Ȯ�� ������ ����ü �޸� ��ȯ
		}
		else {
			std::cout << "Unknown IOCP event!\n";
			exit(-1);
		}
	}
}

// ��Ŷ ���� �Լ�
void Server::Send_Packet(DWORD id, unsigned char *packet) {
	Overlap_ex *over = new Overlap_ex; // ���� �Ҵ�
	memset(over, 0, sizeof(Overlap_ex)); // �޸� �ʱ�ȭ
	over->operation = OP_SEND;
	over->wsabuf.buf = reinterpret_cast<CHAR *>(over->iocp_buffer); //
	over->wsabuf.len = packet[0]; // ��Ŷ�� ùĭ = ������
	memcpy(over->iocp_buffer, packet, packet[0]); // IOCP���ۿ� ��Ŷ ����� ����

												  // (����, WSABUF����ü ������, WSABUF����ü ����, NULL, ȣ�� ���, ������ ����ü�� ������, �Ϸ��ƾ�� ������)
	int ret = WSASend(clients[id].s, &over->wsabuf, 1, NULL, 0, &over->original_Overlap, NULL);
	// ���� ó��
	if (0 != ret) { // ������ ������ �Ϸ��ߴٸ� 0
		int err_no = WSAGetLastError();
		error_display("Send_Packet:WSASend", err_no);
		while (TRUE);
	}
}

// ��Ŷ ó�� �Լ�
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

// ���� �Ҹ�
Server::~Server() {
	// ������ ����
	for (auto& thread : worker_threads) thread->join();
	accept_threads->join();

	// ������ ����
	for (auto& data : worker_threads) delete data;
	delete accept_threads;

	WSACleanup();
}
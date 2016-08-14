#include "Threads.h"

ServerRogicMgr	g_RogicMgr;

// ���Ʈ ������
void MyThreads::Accept_Thread()
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
	bind(accept_socket, reinterpret_cast<sockaddr *>(&listen_addr), sizeof(listen_addr));

	// ���� ��û �ޱ�
	listen(accept_socket, 10); 

	// listen
	while (g_isshutdown == FALSE)
	{
		// Ŭ���̾�Ʈ �ּ� ����
		struct sockaddr_in client_addr;
		int addr_size = sizeof(client_addr);

		// accept
		SOCKET new_client = WSAAccept(accept_socket,
			reinterpret_cast<sockaddr *>(&client_addr), &addr_size, NULL, NULL);

		// ���� ����
		if (new_client == INVALID_SOCKET) {
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
			if (FALSE == g_clients[i].is_connected) {
				new_id = i;
				std::cout << new_id << std::endl;
				break;
			}
		}

		// Ŭ���̾�Ʈ�� ���� �� �� �̻� ID�� �Ҵ� ���� �� ���� ��,
		if (new_id == -1) {
			std::cout << "Max Concurrent User excceded!\n";
			closesocket(new_client);
			continue;
		}

		// ���� �ο��� ���� ���� �� Wave����.
		if (new_id >= MAX_USER - 1)
			g_RogicMgr.WaveStart();

		// Ŭ���̾�Ʈ ���� �ʱ�ȭ
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

		// IOCP ����
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(new_client), g_hIocp, new_id, 0);

		// �÷��̾� �߰� 
		sc_packet_put_player enter_packet;
		enter_packet.size = sizeof(enter_packet);
		enter_packet.type = SC_PUT_PLAYER;
		enter_packet.client_id = new_id;
		enter_packet.cInfo.Pos.x = g_clients[new_id].avatar.Pos.x;
		enter_packet.cInfo.Pos.y = g_clients[new_id].avatar.Pos.y;
		enter_packet.cInfo.Pos.z = g_clients[new_id].avatar.Pos.z;
		enter_packet.cInfo.Rot = g_clients[new_id].avatar.Rot;
		enter_packet.cInfo.Scale = g_clients[new_id].avatar.Scale;

		// ������ ���ο��� �ʱ� ������ �����Ѵ�.
		MyServer::Send_Packet(new_id, reinterpret_cast<unsigned char *>(&enter_packet));

		// �̹� ������ �÷��̾�鿡�� ���ο� �÷��̾ ���Դٴ� ��Ŷ�� ����
		for (int i = 0; i < MAX_USER; ++i) {
			if (i == new_id) continue; // ������ ������
			if (TRUE == g_clients[i].is_connected) // ������ �÷��̾�鿡�� ��Ŷ ���� 
				MyServer::Send_Packet(i, reinterpret_cast<unsigned char *>(&enter_packet));
		}

		// �̹� ������ �÷��̾�鿡 ���� ���� ����
		for (int i = 0; i < MAX_USER; ++i)
		{
			// ������ ������ ó�� ������ �������, for���� �� �ʿ䰡 �����Ƿ�, break.
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

			// �÷��̾� ���ο���, �̹� ������ �����鿡 ���� ������ �����Ѵ�.
			MyServer::Send_Packet(new_id, reinterpret_cast<unsigned char*>(&ex_enter_packet));
		}

		// Ŭ���̾�Ʈ ���� ���� ����
		g_clients[new_id].is_connected = TRUE;

		// ���ӵǴ� �۾��� ���� WSARecv ȣ��
		DWORD flags = 0;
		int ret = WSARecv(new_client, &g_clients[new_id].recv_overlap.wsabuf, 1, NULL,
			&flags, &g_clients[new_id].recv_overlap.original_Overlap, NULL);

		// ���� ó��
		if (0 != ret) {
			int error_no = WSAGetLastError();
			if (WSA_IO_PENDING != error_no)
				error_display("Accept:WSARecv()", error_no);
		}
	}
}

// ��Ŀ ������
void MyThreads::Worker_Thread()
{
	while (g_isshutdown == FALSE)
	{
		DWORD iosize;			// ���۵� �������� ũ��
		DWORD id;				// Completion id : ���� �Ѱܹ޴� �����ʹ� �÷��̾��� ID��
		Overlap_ex *my_overlap; // Ex_Overlap ����ü

		BOOL result = GetQueuedCompletionStatus(g_hIocp, &iosize, &id,
			reinterpret_cast<LPOVERLAPPED *>(&my_overlap), INFINITE);

		if (FALSE == result)
		{
			// ���� ó��
		}

		// ���۵� �����Ͱ� 0�̶�� ���� ������ ���� �ߴٴ� ���� �ǹ�
		if (0 == iosize)
		{
			// ���� ����
			closesocket(g_clients[id].s);

			// ������ ���� �ߴٴ� ���� �ٸ� �÷��̾�鿡�� �˸��� ���� ��Ŷ
			sc_packet_remove_player disconnect;
			disconnect.type = SC_REMOVE_PLAYER;
			disconnect.size = sizeof(disconnect);
			disconnect.client_id = id;

			// ���� ���� ���� ��� �÷��̾�鿡�� ��Ŷ�� ����
			for (auto i = 0; i < MAX_USER; ++i) {
				if (FALSE == g_clients[i].is_connected) continue; // ����x
				if (id == i) continue; // ������ �����ϰ�
				MyServer::Send_Packet(i, reinterpret_cast<unsigned char*>(&disconnect));
			}
			g_clients[id].is_connected = FALSE;
		}

		if (OP_RECV == my_overlap->operation) { // ���ú� �϶�
			unsigned char *buf_ptr = g_clients[id].recv_overlap.iocp_buffer; // IOCP ����
			int remained = iosize; // ó���� ��
			while (0 < remained) { // ó�� �� ���� �ִ� ���
				if (0 == g_clients[id].packet_size) // ���� ��Ŷ�� �Ѿ����
					g_clients[id].packet_size = buf_ptr[0]; // ���۵� ��Ŷ�� ����� ����

				int required = g_clients[id].packet_size - g_clients[id].previous_size; // �䱸�� = ��Ŷ ������ - ������ �Ѿ�� ��

				if (remained >= required) { // ó���� �� >= �䱸�� -> ��Ŷ �ϼ� ����
											// ��Ŷ����+������ �Ѿ�� �� <- �䱸��
					memcpy(g_clients[id].packet_buff + g_clients[id].previous_size, buf_ptr, required);
					MyServer::Pharse_Packet(id, g_clients[id].packet_buff); // ��Ŷ ó�� �Լ� ȣ��
					MyServer::Send_Packet(id, g_clients[id].packet_buff);
					buf_ptr += required; // �̹��� ó���� �� ��ŭ ������ �̵�
					remained -= required; // ��ü ��Ŷ������ - ó���� ��
										  // ��Ŷ ó���� �Ϸ���
					g_clients[id].packet_size = 0;
					g_clients[id].previous_size = 0;
				}
				else { // ������ ����
					   // ��Ŷ���� + ���� ��Ŷ ������ <- 
					memcpy(g_clients[id].packet_buff + g_clients[id].previous_size, buf_ptr, remained);
					buf_ptr += remained; // �۾��Ѹ�ŭ ������ �̵�
					g_clients[id].previous_size += remained; // �۾��� ��
					remained = 0; // �� ó������
				}
			}

			switch (g_clients[id].packet_buff[1])
			{
				case CS_SUCCESS:
					g_RogicMgr.Update();
					break;
				case CS_KEYINPUT:
				{
					cs_packet_move* packet_m = reinterpret_cast<cs_packet_move*>(g_clients[id].packet_buff);
					g_RogicMgr.ProcessKeyInput(*packet_m);
					break;
				}
				case CS_MOUSEINPUT:
				{
					cs_packet_attack* packet_a = reinterpret_cast<cs_packet_attack*>(g_clients[id].packet_buff);
					g_RogicMgr.ProcessMouseInput(*packet_a);
					break;
				}
			}
			/* ==================================================================================================
			�� ��ũ������� Ŭ���ʿ��� ��Ŷ�� ���� �� ����ȴٰ� ����.

			���� ������ ��Ŷ�� ������ ������ 
			���� �������� �������ִ� �����͵��� �����ϰ�
			�� ���ŵ� �����͵��� ���ӵ� ��� Ŭ������׼� �� �޾Ҵٴ�
			��� ��Ŷ�� �޾��� ���� �� ���������� ����.
			
			��� ��Ŷ�� ��� ���� �Ŀ� ���� �������� �����Ѵ�.
			�ڼ��� ������ ServerRogicMgr�� �ּ� ����.

			���� ���׿� ��ȿ�����̰ų� �߸��� �κ� �ִٸ� �����ٶ�.

			-----------------------------------------------------------------
			���� ServerRogicMgr�� ��뿹)

			KeyInputŸ���� ��Ŷ�� �Դٸ� cs_packet_move ����ü ä���
			g_RogicMgr.ProcessidInput(cs_packet_move);

			MouseInputŸ���� ��Ŷ�� �Դٸ� cs_packet_attack ����ü ä���
			g_RogicMgr.ProcessMouseInput(cs_packet_attack);

			CS_SUCCES ��Ŷ�� �Դٸ� <�̰��� �׻� �;���>
			g_RogicMgr.Update() ����.
			-----------------------------------------------------------------

			���� ������ �׽�Ʈ�� �غ��� �� �� ���� �� ����.
			================================================================================================== */

			DWORD flags = 0;
			// (����, WSABUF����ü ������, WSABUF����ü ����, NULL, �޾ƿ� ��, ������ ����ü�� ������, �Ϸ��ƾ�� ������)
			WSARecv(g_clients[id].s, &g_clients[id].recv_overlap.wsabuf, 1, NULL, &flags,
				&g_clients[id].recv_overlap.original_Overlap, NULL);
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
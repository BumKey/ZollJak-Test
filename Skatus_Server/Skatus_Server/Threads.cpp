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
		SOCKET newSocket = WSAAccept(accept_socket,
			reinterpret_cast<sockaddr *>(&client_addr), &addr_size, NULL, NULL);

		// ���� ����
		if (newSocket == INVALID_SOCKET) {
			int error_no = WSAGetLastError();
			error_display("Accept::WSAAccept", error_no);
			while (TRUE);
		}
		else
		{
			// ���� ����
			DEBUG_MSG("server connect success");
		}

		// Ŭ���̾�Ʈ���� ID�� �Ҵ�
		int new_id = -1;
		for (auto i = 0; i < MAX_USER; ++i) {
			if (g_clients[i].is_connected == false) {
				new_id = i;
				g_clients[i].is_connected = true;
				DEBUG_MSG("New client is connected, ID : " << new_id);
				break;
			}
		}

		// Ŭ���̾�Ʈ�� ���� �� �� �̻� ID�� �Ҵ� ���� �� ���� ��,
		if (new_id == -1) {
			DEBUG_MSG("Max Concurrent User excceded!");
			closesocket(newSocket);
			continue;
		}

		// IOCP ����
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(newSocket), g_hIocp, new_id, 0);

		g_RogicMgr.AddPlayer(newSocket, ObjectType::Warrior, new_id);

		// ���ӵǴ� �۾��� ���� WSARecv ȣ��
		DWORD flags = 0;
		int ret = WSARecv(newSocket, &g_clients[new_id].recv_overlap.wsabuf, 1, NULL,
			&flags, &g_clients[new_id].recv_overlap.original_Overlap, NULL);

		// ���� ó��
		if (ret != 0) {
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
		if (iosize == 0)
		{
			g_RogicMgr.RemovePlayer(id);
			// ���� ����
			closesocket(g_clients[id].socket);
		}
		else if (my_overlap->operation == OP_RECV) { // ���ú� �϶�
			char *buf_ptr = g_clients[id].recv_overlap.iocp_buffer; // IOCP ����
			int remained = iosize; // ó���� ��
			while (0 < remained) { // ó�� �� ���� �ִ� ���
				if (g_clients[id].packet_size == 0) // ���� ��Ŷ�� �Ѿ����
					g_clients[id].packet_size = reinterpret_cast<HEADER*>(buf_ptr)->Size; // ���۵� ��Ŷ�� ����� ����

				int required = g_clients[id].packet_size - g_clients[id].previous_size; // �䱸�� = ��Ŷ ������ - ������ �Ѿ�� ��

				if (remained >= required) { // ó���� �� >= �䱸�� -> ��Ŷ �ϼ� ����
											// ��Ŷ����+������ �Ѿ�� �� <- �䱸��
					memcpy(g_clients[id].packet_buff + g_clients[id].previous_size, buf_ptr, required);
					MyServer::Process_Packet(g_clients[id].packet_buff, g_RogicMgr); // ��Ŷ ó�� �Լ� ȣ��
					//MyServer::Send_Packet(id, g_clients[id].packet_buff);
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
			WSARecv(g_clients[id].socket, &g_clients[id].recv_overlap.wsabuf, 1, NULL, &flags,
				&g_clients[id].recv_overlap.original_Overlap, NULL);
		}
		else if (OP_SEND == my_overlap->operation) { // ���� �϶�
			delete my_overlap; // Ȯ�� ������ ����ü �޸� ��ȯ
		}
		else {
			DEBUG_MSG("Unknown IOCP event!");
			exit(-1);
		}
	}
}

void MyThreads::Rogic_Thread()
{
	GameTimer timer[3];
	enum eTimer { MonInfo = 0, PlayerInfo, FrameInfo };
	timer[MonInfo].Reset();
	timer[FrameInfo].Reset();
	timer[PlayerInfo].Reset();

	while (1)
	{
		if (g_RogicMgr.GetPlayerNum() > 0) {

			for (int i = 0; i < 3; ++i)
				timer[i].Tick();

			g_RogicMgr.Update();

			if (timer[PlayerInfo].TotalTime() > 0.2f) {
				g_RogicMgr.SendPacektPlayerInfo();
				timer[PlayerInfo].Reset();
			}

			if (timer[MonInfo].TotalTime() > 0.5f) {
				g_RogicMgr.SendPacketMonInfo();
				timer[MonInfo].Reset();
			}

			if (timer[FrameInfo].TotalTime() > 1.0f) {
				g_RogicMgr.SendPacketFrameInfo();
				timer[FrameInfo].Reset();
			}
		}
	}
}

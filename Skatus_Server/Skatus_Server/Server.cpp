#pragma comment (lib, "ws2_32.lib")
#include "stdafx.h"

// global variable
Client clients[MAX_USER];

HANDLE g_hIocp;
bool g_isshutdown = false;

void error_display(char* msg, int err_no);
bool Is_InRange(int a, int b);
void Initialize();
void WorkerThreadStart();
void SendPacket(int id, unsigned char* packet);
void ProcessPacket(int id, unsigned char buf[]);
void AcceptThreadStart();

int main(int argc, char* argv[])
{
	// Workethread에 대한 스레드들을 컨테이너 vector에 저장하기 위해 선언.
	vector <thread*> worker_thread;

	// 초기화
	Initialize();

	// 스레드 수에 맞춰 WorkerThread를 생성, vector에서 관리한다.
	for (auto i = 0; i < NUM_THREAD; ++i)
		worker_thread.push_back(new thread{ WorkerThreadStart });

	thread accept_thread{ AcceptThreadStart };

	while (false == g_isshutdown)
	{
		Sleep(1000);
	}

	for (auto th : worker_thread)
	{
		// exit thread
		th->join();
		delete th;
	}
	accept_thread.join();

	WSACleanup();
}

void AcceptThreadStart()
{
	struct sockaddr_in listen_addr;

	SOCKET accept_socket = WSASocket(AF_INET, SOCK_STREAM,
		IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	listen_addr.sin_port = htons(MY_SERVER_PORT);

	::bind(accept_socket, reinterpret_cast<sockaddr*>(&listen_addr), sizeof(listen_addr));
	listen(accept_socket, 10);

	while (false == g_isshutdown)
	{
		struct sockaddr_in	client_addr;
		int addr_size = sizeof(client_addr);
		SOCKET new_client = WSAAccept(accept_socket, reinterpret_cast<sockaddr*>(&client_addr),
			&addr_size, NULL, NULL);

		if (INVALID_SOCKET == new_client)
		{
			int err_no = WSAGetLastError();
			error_display("AcceptThreadStart::WSAAccept()", err_no);
			while (true);
		}

		int new_id = -1;
		for (auto i = 0; i < MAX_USER; ++i)
		{
			if (false == clients[i].is_connected)
			{
				new_id = i;
				break;
			}
		}
		if (new_id == -1)
		{
			cout << "Max Concurrent User excceded!\n";
			closesocket(new_client);
			continue;
		}

		clients[new_id].sock = new_client;
		clients[new_id].avatar.x = 4;
		clients[new_id].avatar.y = 4;
		clients[new_id].packet_size = 0;
		clients[new_id].previous_size = 0;
		memset(&clients[new_id].recv_overlap.original_overlap, 0,
			sizeof(clients[new_id].recv_overlap.original_overlap));


		// IOCP 연결
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(new_client),
			g_hIocp, new_id, 0);

		// 시야처리.. 아직 미적용x

		sc_packet_put_player enter_packet;
		enter_packet.id = new_id;
		enter_packet.size = sizeof(enter_packet);
		enter_packet.type = SC_PUT_PLAYER;
		enter_packet.x = clients[new_id].avatar.x;
		enter_packet.y = clients[new_id].avatar.y;

		SendPacket(new_id, reinterpret_cast<unsigned char*>(&enter_packet));

		// 시야처리.. 아직 미적용x

		clients[new_id].is_connected = true;

		DWORD flags = 0;

		int retval = WSARecv(new_client, &clients[new_id].recv_overlap.wsabuf,
			1, NULL, &flags, &clients[new_id].recv_overlap.original_overlap, NULL);

		if (retval != 0)
		{
			int err_no = WSAGetLastError();
			if (WSA_IO_PENDING != err_no)
				error_display("AcceptThreadStart::WSARecv()", err_no);
		}
	}
}

void WorkerThreadStart()
{
	while (false == g_isshutdown)
	{
		DWORD iosize;
		DWORD key;
		Overlap_ex *my_overlap;

		BOOL result = GetQueuedCompletionStatus(g_hIocp,
			&iosize, &key, reinterpret_cast<LPOVERLAPPED*>(&my_overlap), INFINITE);

		if (FALSE == result)
		{
			// 에러처리
		}
		// IOCP에 등록된 소켓이 close된 경우,
		if (0 == iosize)
		{
			closesocket(clients[key].sock);
			sc_packet_remove_player disconnected;
			disconnected.id = key;
			disconnected.size = sizeof(disconnected);
			disconnected.type = SC_REMOVE_PLAYER;
			for (auto i = 0; i < MAX_USER; ++i)
			{
				if (false == clients[i].is_connected) continue;
				if (key == i) continue;
				SendPacket(i, reinterpret_cast<unsigned char*>(&disconnected));
			}
		}

		if (OP_RECV == my_overlap->operation)
		{
			unsigned char *buf_ptr = clients[key].recv_overlap.iocp_buff;
			int remained = iosize;
			while (0 < remained)
			{
				if (0 == clients[key].packet_size)
					clients[key].packet_size = buf_ptr[0];
				int required = clients[key].packet_size - clients[key].previous_size;
				if (remained >= required)
				{
					memcpy(clients[key].packet_buff + clients[key].previous_size, buf_ptr, required);
					ProcessPacket(key, clients[key].packet_buff);
					buf_ptr += required;
					remained -= required;
					clients[key].packet_size = 0;
					clients[key].previous_size = 0;
				}
				else
				{
					memcpy(clients[key].packet_buff + clients[key].previous_size, buf_ptr, remained);
					buf_ptr += remained;
					clients[key].previous_size += remained;
					remained = 0;
				}
			}
			DWORD flags = 0;
			WSARecv(clients[key].sock, &my_overlap->wsabuf, 1, NULL, &flags,
				&my_overlap->original_overlap, NULL);
		}
		else if (OP_SEND == my_overlap->operation)
		{
			delete my_overlap;
		}
		else
		{
			cout << "Unknown IOCP Event!\n";
			exit(-1);
		}
	}
}

void ProcessPacket(int id, unsigned char buf[])
{
	int x = clients[id].avatar.x;
	int y = clients[id].avatar.y;

	switch (buf[1])
	{
	case CS_UP: y--; break;
	case CS_DOWN: y++; break;
	case CS_LEFT: x--; break;
	case CS_RIGHT: x++; break;
	default: cout << "Unknown type packet received!\n";
		while (true);
	}

	if (y < 0) y = 0;
	if (y >= BOARD_HEIGHT) y = BOARD_HEIGHT - 1;
	if (x < 0) x = 0;
	if (x >= BOARD_WIDTH) x = BOARD_WIDTH - 1;

	clients[id].avatar.x = x;
	clients[id].avatar.y = y;

	sc_packet_pos move_packet;
	move_packet.id = id;
	move_packet.size = sizeof(move_packet);
	move_packet.type = SC_POS;
	move_packet.x = x;
	move_packet.y = y;

	SendPacket(id, reinterpret_cast<unsigned char*>(&move_packet));

	// 시야 처리 부분... 아직 구현 X
	// Create view_list ... ing ....
}


void SendPacket(int id, unsigned char* packet)
{
	Overlap_ex *over = new Overlap_ex;
	memset(over, 0, sizeof(Overlap_ex));
	over->operation = OP_SEND;
	over->wsabuf.buf = reinterpret_cast<CHAR*>(over->iocp_buff);
	over->wsabuf.len = packet[0];
	memcpy(over->iocp_buff, packet, packet[0]);

	int retval = WSASend(clients[id].sock, &over->wsabuf, 1, NULL, 0,
		&over->original_overlap, NULL);

	if (retval != 0)
	{
		int error_no = WSAGetLastError();
		error_display("SendPacket::WSASend()", error_no);
		while (true);
	}
}

void error_display(char* msg, int err_no)
{
	WCHAR *lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("%s", msg);
	wprintf(L"error%s\n", lpMsgBuf);
	LocalFree(lpMsgBuf);
}

bool Is_InRange(int a, int b)
{
	int dist = (clients[a].avatar.x - clients[b].avatar.x)
		*(clients[a].avatar.x - clients[b].avatar.x)
		+ (clients[a].avatar.y - clients[b].avatar.y)
		* (clients[a].avatar.y - clients[b].avatar.y);
	return dist <= VIEW_RADIUS * VIEW_RADIUS;
}

void Initialize()
{
	for (auto i = 0; i < MAX_USER; ++i)
	{
		clients[i].is_connected = false;
		clients[i].recv_overlap.operation = OP_RECV;
		clients[i].recv_overlap.wsabuf.buf =
			reinterpret_cast<CHAR*>(clients[i].recv_overlap.iocp_buff);
		clients[i].recv_overlap.wsabuf.len =
			sizeof(clients[i].recv_overlap.iocp_buff);
	}

	_wsetlocale(LC_ALL, L"korean"); // 프로그램이 실행되는 나라 및 지역에 대한 정보를 설정하는 함수

	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	// Iocp Handle 생성
	g_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
}
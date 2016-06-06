#include <WinSock2.h>
#include <Windows.h>

#include <vector>
#include <thread>
#include <iostream>
#include <unordered_set>
#include <mutex>
#include <queue>

#include "protocol.h"

#define NUM_THREADS 6

#define OP_RECV  1
#define OP_SEND  2
#define OP_MOVE  3

using namespace std;

struct Overlap_ex {
	WSAOVERLAPPED original_overlap;
	int operation;
	WSABUF wsabuf;
	unsigned char iocp_buffer[MAX_BUFF_SIZE];
};

struct Player {
	int x;
	int y;
	bool is_active;
	unsigned int last_move_time;
};

struct event_type {
	int obj_id;
	unsigned int wakeup_time;
	int event_id;
};

class mycomp
{
public:
	bool operator() (const event_type lhs, const event_type rhs) const
	{
		return (lhs.wakeup_time > rhs.wakeup_time);
	}
};

priority_queue<event_type, vector<event_type>, mycomp> timer_queue;
mutex timer_lock;

struct Client {
	SOCKET s;
	bool is_connected;
	unordered_set <int> view_list;
	mutex vl_lock;
	Player avatar;
	Overlap_ex recv_overlap;
	int	packet_size;
	int previous_size;
	unsigned char packet_buff[MAX_PACKET_SIZE];
};

Client clients[NUM_OF_NPC];

HANDLE g_hIocp;
bool	g_isshutdown = false;
#pragma comment (lib, "ws2_32.lib")

void error_display(char *msg, int err_no)
{
	WCHAR *lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("%s", msg);
	wprintf(L"에러%s\n", lpMsgBuf);
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

const bool Is_NPC(const int id)
{
	return id >= NPC_START;
}

const bool Is_Active(const int npc)
{
	return clients[npc].avatar.is_active;
}

void Initialize()
{
	for (auto i = 0; i < MAX_USER; ++i) {
		clients[i].is_connected = false;
		clients[i].recv_overlap.operation = OP_RECV;
		clients[i].recv_overlap.wsabuf.buf =
			reinterpret_cast<CHAR *>(clients[i].recv_overlap.iocp_buffer);
		clients[i].recv_overlap.wsabuf.len =
			sizeof(clients[i].recv_overlap.iocp_buffer);
	}

	for (auto i = NPC_START; i < NUM_OF_NPC; ++i)
	{
		clients[i].is_connected = true;
		clients[i].avatar.is_active = false;
		clients[i].avatar.x = rand() % BOARD_WIDTH;
		clients[i].avatar.y = rand() % BOARD_HEIGHT;
		clients[i].avatar.last_move_time = GetTickCount();
		//		timer_queue.push(event_type{ i, GetTickCount() + 1000, OP_MOVE });
	}

	_wsetlocale(LC_ALL, L"korean");

	WSADATA	wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	g_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
}

void SendPacket(int id, unsigned char *packet)
{
	Overlap_ex *over = new Overlap_ex;
	memset(over, 0, sizeof(Overlap_ex));
	over->operation = OP_SEND;
	over->wsabuf.buf = reinterpret_cast<CHAR *>(over->iocp_buffer);
	over->wsabuf.len = packet[0];
	memcpy(over->iocp_buffer, packet, packet[0]);

	int ret = WSASend(clients[id].s, &over->wsabuf, 1, NULL, 0,
		&over->original_overlap, NULL);
	if (0 != ret) {
		int error_no = WSAGetLastError();
		error_display("SendPacket:WSASend", error_no);
		while (true);
	}
}

void SendPutPlayerPacket(int client, int object)
{
	sc_packet_put_player packet;
	packet.id = object;
	packet.size = sizeof(packet);
	packet.type = SC_PUT_PLAYER;
	packet.x = clients[object].avatar.x;
	packet.y = clients[object].avatar.y;

	SendPacket(client, reinterpret_cast<unsigned char *>(&packet));
}

void SendRemovePlayerPacket(int client, int object)
{
	sc_packet_remove_player packet;
	packet.id = object;
	packet.size = sizeof(packet);
	packet.type = SC_REMOVE_PLAYER;

	SendPacket(client, reinterpret_cast<unsigned char *>(&packet));
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

	sc_packet_pos mov_packet;
	mov_packet.id = id;
	mov_packet.size = sizeof(mov_packet);
	mov_packet.type = SC_POS;
	mov_packet.x = x;
	mov_packet.y = y;

	SendPacket(id, reinterpret_cast<unsigned char *>(&mov_packet));

	unordered_set <int> new_list;
	for (auto i = 0; i < NUM_OF_NPC; ++i) {
		if (false == clients[i].is_connected) continue;
		if (i == id) continue;
		if (false == Is_InRange(i, id)) continue;
		new_list.insert(i);
	}

	for (auto i : new_list) {
		clients[id].vl_lock.lock();
		if (0 == clients[id].view_list.count(i)) { // 새로 뷰리스트에 들어오는 객체 처리
			clients[id].view_list.insert(i);
			clients[id].vl_lock.unlock();
			SendPutPlayerPacket(id, i);

			if (Is_NPC(i)) {
				if (!Is_Active(i)) {
					clients[i].avatar.is_active = true;
					timer_queue.push(event_type{ i, GetTickCount() + 1000, OP_MOVE });
				}
				continue;
			};
			clients[i].vl_lock.lock();
			if (0 == clients[i].view_list.count(id)) {
				clients[i].view_list.insert(id);
				clients[i].vl_lock.unlock();
				SendPutPlayerPacket(i, id);
			}
			else {
				clients[i].vl_lock.unlock();
				SendPacket(i, reinterpret_cast<unsigned char *>(&mov_packet));
			}
		}
		else { // 뷰리스트에 계속 유지되어 있는 객체 처리
			clients[id].vl_lock.unlock();
			if (Is_NPC(i)) continue;
			clients[i].vl_lock.lock();
			if (1 == clients[i].view_list.count(id)) {
				clients[i].vl_lock.unlock();
				SendPacket(i, reinterpret_cast<unsigned char *>(&mov_packet));
			}
			else {
				clients[i].view_list.insert(id);
				clients[i].vl_lock.unlock();
				SendPutPlayerPacket(i, id);
			}
		}
	}

	// 뷰리스트에서 나가는 객체 처리
	vector <int> remove_list;
	clients[id].vl_lock.lock();
	for (auto i : clients[id].view_list) {
		if (0 != new_list.count(i)) continue;
		remove_list.push_back(i);
	}
	for (auto i : remove_list) clients[id].view_list.erase(i);
	clients[id].vl_lock.unlock();

	for (auto i : remove_list)
		SendRemovePlayerPacket(id, i);


	for (auto i : remove_list) {
		if (Is_NPC(i)) continue;
		clients[i].vl_lock.lock();
		if (0 != clients[i].view_list.count(id)) {
			clients[i].view_list.erase(id);
			clients[i].vl_lock.unlock();
			SendRemovePlayerPacket(i, id);
		}
		else 	clients[i].vl_lock.unlock();
	}
}

void Do_Move(int i)
{
	if (false == clients[i].avatar.is_active) return;

	volatile int k;
	volatile int sum = 0;
	for (k = 0; k < 10000; k++) sum += k;

	int x = clients[i].avatar.x;
	int y = clients[i].avatar.y;

	unordered_set<int> view_list;
	for (auto pl = 0; pl < MAX_USER; pl++) {
		if (false == clients[pl].is_connected) continue;
		if (false == Is_InRange(i, pl)) continue;
		view_list.insert(pl);
	}
	switch (rand() % 4) {
	case 0: if (x < BOARD_WIDTH - 1) x++; break;
	case 1: if (x > 0) x--; break;
	case 2: if (y < BOARD_HEIGHT - 1) y++; break;
	case 3: if (y > 0) y--; break;
	}
	clients[i].avatar.x = x;
	clients[i].avatar.y = y;

	unordered_set <int> new_list;
	for (auto pl = 0; pl < MAX_USER; pl++) {
		if (false == clients[pl].is_connected) continue;
		if (false == Is_InRange(i, pl)) continue;
		new_list.insert(pl);
	}

	for (auto pl : view_list)
	{
		if (0 == new_list.count(pl)) {
			clients[pl].vl_lock.lock();
			clients[pl].view_list.erase(i);
			clients[pl].vl_lock.unlock();
			sc_packet_remove_player packet;
			packet.id = i;
			packet.size = sizeof(packet);
			packet.type = SC_REMOVE_PLAYER;
			SendPacket(pl, reinterpret_cast<unsigned char *>(&packet));
		}
		else {
			sc_packet_pos packet;
			packet.id = i;
			packet.size = sizeof(packet);
			packet.type = SC_POS;
			packet.x = x;
			packet.y = y;
			SendPacket(pl, reinterpret_cast<unsigned char *>(&packet));
		}
	}
	for (auto pl : new_list) {
		if (0 != view_list.count(pl)) continue;
		sc_packet_put_player packet;
		packet.id = i;
		packet.size = sizeof(packet);
		packet.type = SC_PUT_PLAYER;
		packet.x = x;
		packet.y = y;
		SendPacket(pl, reinterpret_cast<unsigned char *>(&packet));
	}
	int now = GetTickCount();
	if (NPC_START == i) cout << "NPC Move duaration: " <<
		now - clients[i].avatar.last_move_time << "\n";
	clients[i].avatar.last_move_time = now;

	timer_lock.lock();
	timer_queue.push(event_type{ i, GetTickCount() + 1000, OP_MOVE });
	timer_lock.unlock();

	if (true == new_list.empty()) clients[i].avatar.is_active = false;
}

void WorkerThreadStart()
{
	while (false == g_isshutdown)
	{
		DWORD iosize;
		DWORD key;
		Overlap_ex *my_overlap;

		BOOL result = GetQueuedCompletionStatus(g_hIocp,
			&iosize, &key, reinterpret_cast<LPOVERLAPPED *>(&my_overlap), INFINITE);
		if (FALSE == result) {
			// 에러 처리
		}
		if (0 == iosize) {
			closesocket(clients[key].s);
			sc_packet_remove_player discon;
			discon.id = key;
			discon.size = sizeof(discon);
			discon.type = SC_REMOVE_PLAYER;
			for (auto i = 0; i < MAX_USER; ++i) {
				if (false == clients[i].is_connected) continue;
				if (key == i) continue;
				SendPacket(i, reinterpret_cast<unsigned char*>(&discon));
			}
			clients[key].is_connected = false;
		}

		if (OP_RECV == my_overlap->operation) {
			unsigned char *buf_ptr = clients[key].recv_overlap.iocp_buffer;
			int remained = iosize;
			while (0 < remained) {
				if (0 == clients[key].packet_size)
					clients[key].packet_size = buf_ptr[0];
				int required = clients[key].packet_size
					- clients[key].previous_size;
				if (remained >= required) {
					memcpy(
						clients[key].packet_buff + clients[key].previous_size,
						buf_ptr, required);
					ProcessPacket(key, clients[key].packet_buff);
					buf_ptr += required;
					remained -= required;
					clients[key].packet_size = 0;
					clients[key].previous_size = 0;
				}
				else {
					memcpy(clients[key].packet_buff
						+ clients[key].previous_size,
						buf_ptr, remained);
					buf_ptr += remained;
					clients[key].previous_size += remained;
					remained = 0;
				}
			}
			DWORD flags = 0;
			WSARecv(clients[key].s,
				&clients[key].recv_overlap.wsabuf, 1, NULL, &flags,
				&clients[key].recv_overlap.original_overlap, NULL);
		}
		else if (OP_SEND == my_overlap->operation) {
			delete my_overlap;
		}
		else if (OP_MOVE == my_overlap->operation) {
			Do_Move(key);
			delete my_overlap;
		}
		else {
			cout << "Unknown IOCP event!\n";
			exit(-1);
		}
	}
}

void AcceptThreadStart()
{
	struct sockaddr_in listen_addr;

	SOCKET accept_socket = WSASocket(AF_INET, SOCK_STREAM,
		IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = htonl(ADDR_ANY);
	listen_addr.sin_port = htons(MY_SERVER_PORT);
	// listen_addr.sin_zero = 0;
	::bind(accept_socket,
		reinterpret_cast<sockaddr *>(&listen_addr), sizeof(listen_addr));
	listen(accept_socket, 10);
	// listen
	while (false == g_isshutdown)
	{
		struct sockaddr_in client_addr;
		int addr_size = sizeof(client_addr);
		SOCKET new_client = WSAAccept(accept_socket,
			reinterpret_cast<sockaddr *>(&client_addr), &addr_size,
			NULL, NULL);

		if (INVALID_SOCKET == new_client) {
			int error_no = WSAGetLastError();
			error_display("Accept::WSAAccept", error_no);
			while (true);
		}

		int new_id = -1;
		for (auto i = 0; i < MAX_USER; ++i)
			if (false == clients[i].is_connected) {
				new_id = i;
				break;
			}

		if (-1 == new_id) {
			cout << "Max Concurrent User excceded!\n";
			closesocket(new_client);
			continue;
		}

		clients[new_id].s = new_client;
		clients[new_id].avatar.x = 4;
		clients[new_id].avatar.y = 4;
		clients[new_id].packet_size = 0;
		clients[new_id].previous_size = 0;
		memset(&clients[new_id].recv_overlap.original_overlap, 0,
			sizeof(clients[new_id].recv_overlap.original_overlap));

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(new_client),
			g_hIocp, new_id, 0);

		clients[new_id].vl_lock.lock();
		clients[new_id].view_list.clear();
		clients[new_id].vl_lock.unlock();

		sc_packet_put_player enter_packet;
		enter_packet.id = new_id;
		enter_packet.size = sizeof(enter_packet);
		enter_packet.type = SC_PUT_PLAYER;
		enter_packet.x = clients[new_id].avatar.x;
		enter_packet.y = clients[new_id].avatar.y;

		SendPacket(new_id, reinterpret_cast<unsigned char *>(&enter_packet));

		for (auto i = 0; i < MAX_USER; ++i) {
			if (false == clients[i].is_connected) continue;
			if (i == new_id) continue;
			if (false == Is_InRange(i, new_id)) continue;
			clients[i].vl_lock.lock();
			clients[i].view_list.insert(new_id);
			clients[i].vl_lock.unlock();
			SendPacket(i, reinterpret_cast<unsigned char *>(&enter_packet));
		}

		for (auto i = 0; i < NUM_OF_NPC; ++i) {
			if (false == clients[i].is_connected) continue;
			if (i == new_id) continue;
			if (false == Is_InRange(i, new_id)) continue;

			if (Is_NPC(i) && (!Is_Active(i))) {
				clients[i].avatar.is_active = true;
				timer_queue.push(event_type{ i, GetTickCount() + 1000, OP_MOVE });
			}

			clients[new_id].vl_lock.lock();
			clients[new_id].view_list.insert(i);
			clients[new_id].vl_lock.unlock();
			enter_packet.id = i;
			enter_packet.x = clients[i].avatar.x;
			enter_packet.y = clients[i].avatar.y;
			SendPacket(new_id, reinterpret_cast<unsigned char *>(&enter_packet));
		}
		clients[new_id].is_connected = true;
		DWORD flags = 0;
		int ret = WSARecv(new_client, &clients[new_id].recv_overlap.wsabuf, 1, NULL,
			&flags, &clients[new_id].recv_overlap.original_overlap, NULL);
		if (0 != ret)
		{
			int error_no = WSAGetLastError();
			if (WSA_IO_PENDING != error_no)
				error_display("Accept:WSARecv", error_no);
		}
	}
}

void heart_beat(int npc)
{
	if ((clients[npc].avatar.last_move_time + 1000) <= GetTickCount())
		Do_Move(npc);
}

void AI_Thread_Start()
{
	while (true) {
		for (int i = NPC_START; i < NUM_OF_NPC; ++i)
			heart_beat(i);
		Sleep(1);
	}
}

void Timer_Thread_Start()
{
	while (true) {
		Sleep(1);
		timer_lock.lock();
		while (false == timer_queue.empty()) {
			if (timer_queue.top().wakeup_time > GetTickCount()) break;
			event_type ev = timer_queue.top();
			timer_queue.pop();
			timer_lock.unlock();
			Overlap_ex *over = new Overlap_ex;
			over->operation = ev.event_id;
			PostQueuedCompletionStatus(g_hIocp, 1,
				ev.obj_id,
				&(over->original_overlap));
			timer_lock.lock();
		}
		timer_lock.unlock();
	}
}

void Cleanup()
{
	WSACleanup();
}

int main()
{
	vector <thread *> worker_threads;

	Initialize();

	for (auto i = 0; i < NUM_THREADS; ++i)
		worker_threads.push_back(new thread{ WorkerThreadStart });

	thread accept_thread{ AcceptThreadStart };

	// thread ai_thread{ AI_Thread_Start };

	thread timer_thread{ Timer_Thread_Start };

	while (false == g_isshutdown)
	{
		Sleep(1000);
	}

	for (auto th : worker_threads) {
		th->join();
		delete th;
	}
	accept_thread.join();
	Cleanup();
}
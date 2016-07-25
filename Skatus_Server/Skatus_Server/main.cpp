#include "stdafx.h"
#include "server.h"
#include "Threads.h"

// ���� ���� �߰� ��, stdafx.h���� extern ���� �߰��� ��
HANDLE			g_hIocp;				// IOCP �ڵ�
Client			g_clients[MAX_USER]; // Ŭ���̾�Ʈ�� ������ ������ ���� ����ü
BOOL			g_isshutdown = FALSE;

int main(int argc, char argv[])
{
	// ���� �ʱ�ȭ
	MyServer::Initialize();

	// �ش� PC�� CPU ���� Ȯ��
	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);

	// ���Ʈ ������ ����
	std::thread accept_threads(MyThreads::Accept_Thread);

	// ��Ŀ ������ ����
	std::vector <std::thread*> worker_threads;
	for (auto i = 0; i < systemInfo.dwNumberOfProcessors; ++i)
		worker_threads.push_back(new std::thread{ MyThreads::Worker_Thread });

	while (1) {
		if (g_isshutdown)
		{
			// ������ ����
			for (auto& thread : worker_threads) thread->join();
			accept_threads.join();

			// ������ ����
			for (auto& data : worker_threads) delete data;

			WSACleanup();
		}
		else
			Sleep(1000);
	}
}
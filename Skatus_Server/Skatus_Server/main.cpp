#include "stdafx.h"
#include "server.h"
#include "Threads.h"

// 전역 변수 추가 시, stdafx.h에도 extern 선언 추가할 것
HANDLE			g_hIocp;				// IOCP 핸들
Client			g_clients[MAX_USER]; // 클라이언트의 정보를 가지고 오는 구조체
BOOL			g_isshutdown = FALSE;

int main(int argc, char argv[])
{
	// 서버 초기화
	MyServer::Initialize();

	// 해당 PC의 CPU 개수 확인
	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);

	// 어셉트 스레드 생성
	std::thread accept_threads(MyThreads::Accept_Thread);

	// 워커 스레드 생성
	std::vector <std::thread*> worker_threads;
	for (auto i = 0; i < systemInfo.dwNumberOfProcessors; ++i)
		worker_threads.push_back(new std::thread{ MyThreads::Worker_Thread });

	while (1) {
		if (g_isshutdown)
		{
			// 쓰레드 종료
			for (auto& thread : worker_threads) thread->join();
			accept_threads.join();

			// 쓰레드 제거
			for (auto& data : worker_threads) delete data;

			WSACleanup();
		}
		else
			Sleep(1000);
	}
}
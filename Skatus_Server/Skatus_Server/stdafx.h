#pragma once
#pragma warning (disable : 4996)

#include <WinSock2.h>
#include <Windows.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <assert.h>
#include <d3dx11.h>
#include <xnamath.h>
#include <unordered_map>

#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dx11.lib")
#pragma comment (lib, "d3dx10.lib")

#include "protocol.h"
#include "ServerRogicMgr.h"
#include "Utility.h"

extern HANDLE	g_hIocp;
extern Client	g_clients[MAX_USER];
extern BOOL		g_isshutdown;
extern ServerRogicMgr	g_RogicMgr;

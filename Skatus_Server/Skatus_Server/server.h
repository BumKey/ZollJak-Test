#pragma once
#include "stdafx.h"
#include "ServerRogicMgr.h"

class ServerRogicMgr;
namespace MyServer {
	void Initialize();
	void Process_Packet(char* buf, ServerRogicMgr& rogicMgr);
	void Send_Packet(DWORD id, char *packet);
}
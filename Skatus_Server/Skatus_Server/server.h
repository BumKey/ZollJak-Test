#pragma once
#include "stdafx.h"

namespace MyServer {
	void Initialize();
	unsigned char* Pharse_Packet(DWORD id, unsigned char buf[]);
	void Send_Packet(DWORD id, unsigned char *packet);
}
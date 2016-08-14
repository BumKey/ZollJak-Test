#pragma once
#include "stdafx.h"

namespace MyServer {
	void Initialize();
	void Pharse_Packet(DWORD id, unsigned char buf[]);
	void Send_Packet(DWORD id, unsigned char *packet);
}
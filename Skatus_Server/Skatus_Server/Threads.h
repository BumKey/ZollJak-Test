#pragma once
#include "stdafx.h"
#include "server.h"
#include "ServerRogicMgr.h"

namespace MyThreads {

	void Accept_Thread();
	void Worker_Thread();
	void Rogic_Thread();
}

#pragma once
#include "Monster.h"

class Cyclop : public Monster
{
public:
	Cyclop(SkinnedMesh* mesh, const InstanceDesc& info);
	~Cyclop();
};


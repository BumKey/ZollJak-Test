#pragma once
#include "Monster.h"

class Cyclop : public Monster
{
public:
	Cyclop(SkinnedMesh* mesh, const SO_InitDesc& info);
	~Cyclop();
};


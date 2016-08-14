#pragma once
#include "SkinnedObject.h"

class Warrior : public SkinnedObject
{
public:
	Warrior(SkinnedMesh* mesh, const SO_InitDesc& info);
	~Warrior();
};


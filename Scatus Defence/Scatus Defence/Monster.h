#pragma once
#include "SkinnedObject.h"
class Monster : public SkinnedObject
{
public:
	Monster(SkinnedMesh* mesh, const InstanceDesc& info);
	virtual ~Monster();
};


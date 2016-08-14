#pragma once
#include "BasicObject.h"

class Temple : public BasicObject
{
public:
	Temple(BasicMesh* mesh, const InstanceDesc& info, Label label);
	~Temple();
};


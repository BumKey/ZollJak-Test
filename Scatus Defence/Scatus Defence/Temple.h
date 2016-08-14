#pragma once
#include "BasicObject.h"

class Temple : public BasicObject
{
public:
	Temple(BasicMesh* mesh, const BO_InitDesc& info, Label label);
	~Temple();
};


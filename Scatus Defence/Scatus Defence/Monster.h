#pragma once
#include "SkinnedObject.h"
class Monster : public SkinnedObject
{
public:
	virtual int Get_Obj_type() { return 0; };
	virtual Properties * Get_Properties() { return NULL; }
	virtual int Get_States() { return 0; }
	virtual int SetObj_State(int new_state) { return 0; } // 오브젝트 상태설정
	Monster(SkinnedMesh* mesh, const InstanceDesc& info);
	virtual ~Monster();
};


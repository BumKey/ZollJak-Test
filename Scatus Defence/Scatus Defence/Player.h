#pragma once
#include "SkinnedObject.h"

class Player : public SkinnedObject
{
public:
	Player(SkinnedMesh* Mesh, const InstanceDesc& info);
	~Player();

public:
	virtual int Get_Obj_type() { return 0; };
	virtual bool SetClip(std::string clipName);
	virtual void Animate(float dt);
	virtual void Release(ResourceMgr& rMgr);
	virtual Properties * Get_Properties() { return NULL; }
	virtual int Get_States() { return 0; }
	virtual int SetObj_State(int new_state) { return 0; } // 오브젝트 상태설정
};


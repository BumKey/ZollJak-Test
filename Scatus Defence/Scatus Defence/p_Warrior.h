#pragma once
#include"Player.h"
class p_Warrior : public Player
{
public:
	Warriorproperties warrior_p;
	virtual int Get_Obj_type() { return warrior_p.obj_type; };
	virtual int Get_States() { return warrior_p.state; }
	virtual Properties * Get_Properties() { return &warrior_p; }
	virtual int SetObj_State(int new_state) { warrior_p.state = new_state; return new_state; }
	p_Warrior(SkinnedMesh* Mesh, const InstanceDesc& info): Player(Mesh, info) {}
	~p_Warrior();
};


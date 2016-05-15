#pragma once
#include "Monster.h"
#include <unordered_map>

enum GoblinType {
	Red, Green, Blue
};

enum GoblinAnim {
	attack1, attack2, damage, dead, walk,
	drop_down, run, sit_up, stand, look_around
};

class Goblin : public Monster
{
public:
	Goblin(SkinnedMesh* mesh, const InstanceDesc& info, GoblinType type);
	~Goblin();
	
public:
	std::string GetAnimName(GoblinAnim eAnim);

	virtual void DrawToScene(ID3D11DeviceContext * dc, const Camera & cam, XMFLOAT4X4 shadowTransform, FLOAT tHeight);
	virtual void Release(ResourceMgr& rMgr);

	virtual bool SetClip(std::string clipName);
	virtual void Animate(float dt);

	virtual Properties * Get_Properties() { return &goblin_properties; }
	virtual int Get_States() { return goblin_properties.state; }
	virtual int SetObj_State(int new_state) { goblin_properties.state = new_state; return new_state; }
	virtual int Get_Obj_type() { return goblin_properties.obj_type; };
private:
	GoblinProperties goblin_properties;
	
	GoblinType mType;
	std::unordered_map<GoblinAnim, std::string> mGoblinAnimNames;
};


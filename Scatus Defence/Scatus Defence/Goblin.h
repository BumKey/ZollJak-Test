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

	virtual void DrawToScene(ID3D11DeviceContext* dc, const Camera& cam, const XMFLOAT4X4& shadowTransform, const FLOAT& tHeight);
	virtual void Release(ResourceMgr& rMgr);

	virtual void Animate(float dt);

private:
	void SetClip();

	GoblinType mType;
	std::unordered_map<GoblinAnim, std::string> mGoblinAnimNames;
};


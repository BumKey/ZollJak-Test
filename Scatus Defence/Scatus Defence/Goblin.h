#pragma once
#include "Monster.h"
#include <unordered_map>


class Goblin : public Monster
{
public:
	enum Type {
		Red, Green, Blue
	};

	enum Anim {
		attack1, attack2, damage, dead, walk,
		drop_down, run, sit_up, stand, look_around
	};

	Goblin(SkinnedMesh* mesh, const InstanceDesc& info, Type type);
	~Goblin();

public:
	std::string GetAnimName(Anim eAnim);

	virtual void DrawToScene(ID3D11DeviceContext* dc, const Camera& cam, const XMFLOAT4X4& shadowTransform, const FLOAT& tHeight);

	virtual void Animate(float dt);

private:
	void SetClip();

	Type mGoblinType;
};


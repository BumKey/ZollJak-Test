#pragma once
#include "Monster.h"
#include <unordered_map>


class Goblin : public Monster
{
public:
	enum Type {
		Red, Green, Blue
	};

	Goblin(SkinnedMesh* mesh, const InstanceDesc& info, Type type);
	~Goblin();

public:
	virtual void DrawToScene(ID3D11DeviceContext* dc, const Camera& cam, const XMFLOAT4X4& shadowTransform, const FLOAT& tHeight);

private:
	Type mGoblinType;
};


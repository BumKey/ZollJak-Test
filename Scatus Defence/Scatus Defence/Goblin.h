#pragma once
#include "Monster.h"
#include <unordered_map>


class Goblin : public Monster
{
public:
	enum Type {
		Red = 0, Green, Blue
	};

	Goblin(SkinnedMesh* mesh, const SO_InitDesc& info, Type type);
	~Goblin();

public:
	virtual void DrawToScene(ID3D11DeviceContext* dc, const XMFLOAT4X4& shadowTransform);

private:
	Type mGoblinType;
};


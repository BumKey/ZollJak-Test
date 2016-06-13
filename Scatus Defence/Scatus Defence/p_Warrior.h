#pragma once
#include"Player.h"
class p_Warrior : public Player
{
public:

	p_Warrior(SkinnedMesh* Mesh, const InstanceDesc& info): Player(Mesh, info) {}
	~p_Warrior();
};


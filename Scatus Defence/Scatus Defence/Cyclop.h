#pragma once
#include "Monster.h"



class Cyclop : public Monster
{
public:
	Cyclop(SkinnedMesh* mesh, const InstanceDesc& info);
	~Cyclop();

	enum CyclopAnim {
		attack1, attack2, hit, dead, walk,
		run, stunned, stunned_hit, idle
	};

	virtual void Animate(float dt);
private:
	void SetClip();
};


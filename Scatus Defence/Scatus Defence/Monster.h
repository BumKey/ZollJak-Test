#pragma once
#include "SkinnedObject.h"

class Monster : public SkinnedObject
{
public:
	Monster(SkinnedMesh* mesh, const InstanceDesc& info);
	virtual ~Monster();

	virtual void AttackToTarget(float dt);
	virtual void MoveToTarget(float dt); // For Monster

	bool			HasTarget() { return mHasTarget; }

	void			SetTarget(GameObject* target);
	void			SetNoTarget();

private:
	bool mHasTarget;

};


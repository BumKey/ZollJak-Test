#pragma once
#include "SkinnedObject.h"

class Monster : public SkinnedObject
{
public:
	Monster(SkinnedMesh* mesh, const InstanceDesc& info);
	virtual ~Monster();

public:
	virtual void Update(float dt);
	virtual void MovingCollision(const XMFLOAT3& crushedObjectPos , float dt);

	void		SetAI_State(AI_State::States state);

protected:
	virtual void AttackToTarget(float dt);
	virtual void MoveToTarget(float dt);

private:
	float AngleToTarget(XMVECTOR vTarget);

private:
	AI_State::States mAI_States;

};


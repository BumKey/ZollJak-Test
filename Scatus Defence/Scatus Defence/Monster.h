#pragma once
#include "SkinnedObject.h"

class Monster : public SkinnedObject
{
public:
	Monster();
	Monster(SkinnedMesh* mesh, const SO_InitDesc& info);
	virtual ~Monster();

public:
	virtual void Update(float dt);
	virtual void MovingCollision(const XMFLOAT3& crushedObjectPos , float dt);

	void		SetAI_State(AI_State::States state);

protected:
	virtual void AttackToTarget(float dt);
	virtual void MoveToTargetObject(float dt);

private:
	AI_State::States mAI_States;

};


#pragma once
#include "SkinnedObject.h"
#include "GameTimer.h"

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

	virtual void Attack(SkinnedObject* target);
	virtual void AttackToTarget(float dt);
	virtual void MoveToTarget(float dt);
	
private:
	virtual void FrustumCulling();

	AI_State::States mAI_States;
	GameTimer mTimer;


};


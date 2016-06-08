#pragma once
#include "SkinnedObject.h"

class Player : public SkinnedObject
{
public:
	Player(SkinnedMesh* Mesh, const InstanceDesc& info);
	~Player();

public:
	void Update(float dt);

	virtual void Attack();
	virtual void Release(ResourceMgr& rMgr);

private:
	virtual void Move(float dt);

private:
	bool m_bAttackAnim;

};


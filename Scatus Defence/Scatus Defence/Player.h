#pragma once
#include "SkinnedObject.h"
#include "Singletone.h"
#include "Monster.h"
#include "protocol.h"
#include "PacketMgr.h"

class Player : public SkinnedObject, public Singletone<Player>
{
private:
	Player();
	~Player();

	friend class Singletone<Player>;
public:
	virtual void Init(SkinnedMesh* mesh, const InstanceDesc& info);

	void Update(float dt);
	void CollisionMoving(const XMFLOAT3& dPos, float dt);

private:
	virtual void ProccessKeyInput(float dt);
	void Move(float walk, float strafe);
};


#pragma once
#include "SkinnedObject.h"
#include "Singletone.h"
#include "Monster.h"
#include "protocol.h"
#include"PacketMgr.h"

class Player : public SkinnedObject, public Singletone<Player>
{
private:
	Player();
	~Player();

	friend class Singletone<Player>;
public:
	void Update(float dt);
	virtual void Release(ResourceMgr& rMgr);

private:
	virtual void Move(float dt);
};


#pragma once
#include "SkinnedObject.h"

class Player : public SkinnedObject
{
public:
	Player(SkinnedMesh* Mesh, const InstanceDesc& info);
	~Player();

public:
	virtual bool SetClip(std::string clipName);
	virtual void Animate(float dt);
	virtual void Release(ResourceMgr& rMgr);
};


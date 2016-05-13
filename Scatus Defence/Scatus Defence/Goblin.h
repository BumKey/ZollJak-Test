#pragma once
#include "SkinnedObject.h"

enum GoblinType {
	Red = 0, Green, Blue
};

class Goblin : public SkinnedObject
{
public:
	Goblin(SkinnedMesh* mesh, const InstanceDesc& info, GoblinType type);
	~Goblin();
	
public:
	virtual void DrawToScene(ID3D11DeviceContext * dc, const Camera & cam, XMFLOAT4X4 shadowTransform, FLOAT tHeight);
	virtual void Release(ResourceMgr& rMgr);

	virtual bool SetClip(std::string clipName);
	virtual void Animate(float dt);
	

private:
	GoblinType mType;
};


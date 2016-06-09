#pragma once
#include "GameObject.h"
#include "SkinnedMesh.h"

class SkinnedObject : public GameObject
{
public:
	SkinnedObject(SkinnedMesh* mesh, const InstanceDesc& info);
	virtual~SkinnedObject();

public:
	virtual void Walk(float d);
	virtual void Strafe(float d);
	virtual void MoveTo(Vector2D direction, float dt); // For Monster
	virtual void MoveTo(Vector2D targetPos, XMFLOAT3 dir, float dt);
	virtual void RotateY(float angle);
	virtual void Attack(float dt);
	virtual void Update(float dt);

	virtual void DrawToScene(ID3D11DeviceContext* dc, const Camera& cam, const XMFLOAT4X4& shadowTransform, const FLOAT& tHeight);
	virtual void DrawToShadowMap(ID3D11DeviceContext* dc, const Camera& cam, const XMMATRIX& lightViewProj, const FLOAT& tHeight);
	virtual void DrawToSsaoNormalDepthMap(ID3D11DeviceContext* dc, const Camera& cam, const FLOAT& tHeight);

	virtual void Release(ResourceMgr& rMgr);

	virtual void Animate(float dt) = 0;

	bool  AnimEnd(std::string clipName);
	void  SetMovingSpeed(FLOAT speed) { mProperty.movespeed = speed; }

protected:
	enum Anims {
		attack1, attack2, hit, dead, walk,
		run, stunned, stunned_hit, idle,
		drop_down, sit_up, look_around
	};

	SkinnedMesh* mMesh;

	FLOAT mTimePos;

	std::string mCurrClipName;
	std::unordered_map<UINT, std::string> mAnimNames;
	std::vector<XMFLOAT4X4> mFinalTransforms;
};


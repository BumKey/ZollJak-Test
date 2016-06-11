#pragma once
#include "GameObject.h"
#include "SkinnedMesh.h"

enum Anims {
	attack1, attack2, hit, dead, walk,
	run, stunned, stunned_hit, idle,
	drop_down, sit_up, look_around
};

class SkinnedObject : public GameObject
{
public:
	SkinnedObject(SkinnedMesh* mesh, const InstanceDesc& info);
	virtual~SkinnedObject();

public:
	virtual void Walk(float d);
	virtual void Strafe(float d);
	virtual void RotateY(float angle);
	virtual void Update(float dt);
	virtual void Animate(float dt);


	virtual void DrawToScene(ID3D11DeviceContext* dc, const Camera& cam, const XMFLOAT4X4& shadowTransform, const FLOAT& tHeight);
	virtual void DrawToShadowMap(ID3D11DeviceContext* dc, const Camera& cam, const XMMATRIX& lightViewProj, const FLOAT& tHeight);
	virtual void DrawToSsaoNormalDepthMap(ID3D11DeviceContext* dc, const Camera& cam, const FLOAT& tHeight);

	virtual void Release(ResourceMgr* rMgr);

	bool  CurrAnimEnd();

	virtual std::string	GetAnimName(Anims& eAnim); const
	void  SetMovingSpeed(FLOAT speed) { mProperty.movespeed = speed; }

private:
	void SetClip();

protected:
	SkinnedMesh* mMesh;

	FLOAT mTimePos;
	FLOAT mAngle;

	std::string mCurrClipName;
	std::unordered_map<Anims, std::string> mAnimNames;
	std::vector<XMFLOAT4X4> mFinalTransforms;
};


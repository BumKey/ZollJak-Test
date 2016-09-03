#pragma once
#include "GameObject.h"
#include "ResourceMgr.h"
#include "SkinnedMesh.h"

enum Anims {
	attack1, attack2, hit, dead, walk,
	run, stunned, stunned_hit, idle,
	drop_down, sit_up, look_around
};

class SkinnedObject : public GameObject
{
public:
	SkinnedObject();
	SkinnedObject(SkinnedMesh* mesh, const SO_InitDesc& info);
	virtual~SkinnedObject();

public:
	virtual void Init(SkinnedMesh* mesh, const SO_InitDesc& info);

	virtual void Update(float dt);
	virtual void Animate(float dt);

	virtual void DrawToScene(ID3D11DeviceContext* dc, const XMFLOAT4X4& shadowTransform);
	virtual void DrawToShadowMap(ID3D11DeviceContext* dc, const XMMATRIX& lightViewProj);
	virtual void DrawToSsaoNormalDepthMap(ID3D11DeviceContext* dc);

	virtual void MoveToTarget(float dt);

	bool  CurrAnimEnd();

	virtual std::string	GetAnimName(Anims& eAnim); const
	void  SetMovingSpeed(FLOAT speed) { mProperty.movespeed = speed; }

private:
	void SetClip();

protected:
	XNA::OrientedBox mOOBB;

	FLOAT mTimePos;;

	std::string mCurrClipName;
	std::unordered_map<Anims, std::string> mAnimNames;
	std::vector<XMFLOAT4X4> mFinalTransforms;
};


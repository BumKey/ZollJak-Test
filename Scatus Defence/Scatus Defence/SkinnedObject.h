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

	virtual void Walk(float dt);
	virtual void Strafe(float dt);

	virtual void Update(float dt);
	virtual void Animate(float dt);

	virtual void Attack(SkinnedObject* target);
	virtual void Die();

	virtual void DrawToScene(ID3D11DeviceContext* dc, const XMFLOAT4X4& shadowTransform);
	virtual void DrawToShadowMap(ID3D11DeviceContext* dc, const XMMATRIX& lightViewProj);
	virtual void DrawToSsaoNormalDepthMap(ID3D11DeviceContext* dc);

	virtual void MoveToTarget(float dt);

	bool  CurrAnimEnd();

	void					SetMovingSpeed(FLOAT speed) { mProperty.movespeed = speed; }

	ActionState::States		GetActionState() const { return mActionState; }
	virtual std::string		GetAnimName(Anims& eAnim); const

	bool					IsAttack() { return mActionState == ActionState::Attack ? true : false; }
	bool					IsDead() { return mActionState == ActionState::Die ? true : false; }
	bool					OneHit() { return m_bForOneHit; }

	void ChangeActionState(ActionState::States state);
	bool IsActionStateChangeAble();

private:
	void SetClip();

protected:
	ActionState::States mActionState;
	bool m_bForOneHit;

	XNA::OrientedBox mOOBB;

	FLOAT mTimePos;;

	std::string mCurrClipName;
	std::unordered_map<Anims, std::string> mAnimNames;
	std::vector<XMFLOAT4X4> mFinalTransforms;
};


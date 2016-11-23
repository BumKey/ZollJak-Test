#pragma once
#include "GameObject.h"
#include "ResourceMgr.h"
#include "SkinnedMesh.h"
//#include "Sound_Manager.h"

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
	virtual void Damage(float damage);
	virtual void Die();

	virtual void DrawToScene(ID3D11DeviceContext* dc, const XMFLOAT4X4& shadowTransform);
	virtual void DrawToShadowMap(ID3D11DeviceContext* dc, const XMMATRIX& lightViewProj);
	virtual void DrawToSsaoNormalDepthMap(ID3D11DeviceContext* dc);

	virtual void MoveToTarget(float dt);

	void SlowDown() { m_bSlowDown = true; }
	void SlowDownOff() { m_bSlowDown = false; }
	bool CurrAnimEnd();

	ActionState::States		GetActionState() const { return mActionState; }
	virtual std::string		GetAnimName(Anims& eAnim); const

	void					SetMovingSpeed(FLOAT speed) { mProperty.movespeed = speed; }
	void					SetNextMove(XMFLOAT3 pos) { pos.y = -0.1f; mTargetPos = pos; }
	void					SetTargetPos(const XMFLOAT3& pos) { mTargetPos = pos; }
	void					SetTarget(SkinnedObject* target);
	void					SetNoTarget() { mTarget = 0; mHasTarget = false; }
	void					SetAttackState();

	bool					IsAttack() { return mActionState == ActionState::Attack ? true : false; }
	bool					IsDead() { return mActionState == ActionState::Die ? true : false; }

	void ChangeActionState(ActionState::States state);
	bool IsActionStateChangeAble();

private:
	void SetClip();

protected:
	ActionState::States mActionState;
	bool m_bForOneHit;
	bool m_bSlowDown;
	bool mHasTarget;

	FLOAT mTimePos;;
	XMFLOAT3 mTargetPos;
	SkinnedObject* mTarget;

	std::string mCurrClipName;
	std::unordered_map<Anims, std::string> mAnimNames;
	std::vector<XMFLOAT4X4> mFinalTransforms;
};


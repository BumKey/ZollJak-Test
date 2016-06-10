#pragma once
#include "Camera.h"
#include "RenderStates.h"
#include "ResourceMgr.h"
#include "Effects.h"
#include "GameMesh.h"
#include "Properties.h"
#include <list>
#include "Utilities.h"

struct InstanceDesc
{
	XMFLOAT3 Pos;
	XMFLOAT3 Rot;
	FLOAT Scale;

	InstanceDesc() {
		Pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		Rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		Scale = 1.0f;
	}
};

class GameObject
{
public:
	GameObject(GameMesh* mesh, const InstanceDesc& info);
	virtual ~GameObject();

public:
	virtual void Walk(float dt) = 0;
	virtual void Strafe(float dt) = 0;
	virtual void RotateY(float angle) = 0;
	virtual void Update(float dt) = 0;

	void Attack(GameObject* target);

	virtual void DrawToScene(ID3D11DeviceContext* dc, const Camera& cam, const XMFLOAT4X4& shadowTransform, const FLOAT& tHeight) = 0;
	virtual void DrawToShadowMap(ID3D11DeviceContext* dc, const Camera& cam, const XMMATRIX& lightViewProj, const FLOAT& tHeight) = 0;
	virtual void DrawToSsaoNormalDepthMap(ID3D11DeviceContext* dc, const Camera& cam, const FLOAT& tHeight) = 0;

	virtual void Release(ResourceMgr* rMgr) = 0;
	virtual void ChangeActionState(ActionState::States aState);

	GameMesh*				GetMesh() const { return mMesh; }
	XMFLOAT4X4				GetWorld() const { return mWorld; }
	XMFLOAT3				GetPos() const { return mPosition; }
	XMFLOAT3				GetLook() const { return mCurrLook; }
	XMFLOAT3				GetRight() const { return mRight; }
	XMFLOAT3				GetUp() const { return mUp; }
	UINT					GetID() const { return mID; }
	UINT					GetObjectGeneratedCount() const { return GeneratedCount; }
	Properties				GetProperty() const { return mProperty; }
	GameObject*				GetTarget() const { return mTarget; }
	ObjectType::Types		GetObjectType() const { return mObjectType; }
	ActionState::States		GetActionState() const { return mActionState; }
	CollisionState::States  GetCollisionState() const { return mCollisionState; }
	XNA::OrientedBox		GetOOBB() const { return mOOBB; }

	void					SetHP(int hp) { mProperty.hp_now = hp; }
	void					SetAttackState() { mActionState = ActionState::Attack; }

	bool					IsAttack() { return mActionState == ActionState::Attack ? true : false; }
	bool					IsDead() { return mActionState == ActionState::Die ? true : false; }
	bool					HasTarget() { return mHasTarget; }

	void					SetTarget(GameObject* target);
	void					SetNoTarget() { mTarget = nullptr; mHasTarget = false; }
	void					SetNoneCollision() { mCollisionState = CollisionState::None; }

	void  PrintLocation(); //객체 위치값출력 반환

	// Behaves
	virtual void Die();

private:
	UINT mID;
	static UINT GeneratedCount;
	bool mHasTarget;

protected:
	XMFLOAT4X4 mWorld;
	GameMesh* mMesh;

	XNA::OrientedBox mOOBB;

	FLOAT	 mScaling;
	XMFLOAT3 mRotation;
	XMFLOAT3 mPosition;

	XMFLOAT3 mRight;
	XMFLOAT3 mUp;
	XMFLOAT3 mCurrLook;
	XMFLOAT3 mPrevLook;
	XMFLOAT3 mDirection;

	Properties mProperty;
	ActionState::States mActionState;
	CollisionState::States mCollisionState;
	ObjectType::Types mObjectType;
	GameObject* mTarget;
};


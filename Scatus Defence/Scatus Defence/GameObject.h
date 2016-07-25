#pragma once
#include "Camera.h"
#include "RenderStates.h"
#include "ResourceMgr.h"
#include "Terrain.h"
#include "Effects.h"
#include "GameMesh.h"
#include "Properties.h"
#include "protocol.h"
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
	GameObject();
	GameObject(GameMesh* mesh, const InstanceDesc& info);
	virtual ~GameObject();

public:
	virtual void Walk(float dt);
	virtual void Strafe(float dt);
	virtual void RotateY(float angle);
	virtual void Update(float dt) = 0;

	void Attack(GameObject* target);

	virtual void DrawToScene(ID3D11DeviceContext* dc, const XMFLOAT4X4& shadowTransform) = 0;
	virtual void DrawToShadowMap(ID3D11DeviceContext* dc, const XMMATRIX& lightViewProj) = 0;
	virtual void DrawToSsaoNormalDepthMap(ID3D11DeviceContext* dc) = 0;

	virtual void Release();
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
	XNA::AxisAlignedBox		GetAABB() const { return mAABB; }
	XNA::Sphere				GetBS() const { return mBS; }

	void					SetHP(int hp) { mProperty.hp_now = hp; }
	void					SetAttackState() { mActionState = ActionState::Attack; m_bForOneHit = true; }

	bool					IsAttack() { return mActionState == ActionState::Attack ? true : false; }
	bool					IsDead() { return mActionState == ActionState::Die ? true : false; }
	bool					OneHit() { return m_bForOneHit; }
	bool					HasTarget() { return mHasTarget; }

	void					SetTarget(GameObject* target);
	void					SetNoTarget() { mTarget = nullptr; mHasTarget = false; }
	void					SetNoneCollision() { mCollisionState = CollisionState::None; }

	void  PrintLocation(); //객체 위치값출력 반환

	// Behaves
	virtual void Die();

protected:
	void InitBoundingObject();
	void UpdateBoundingObject();

protected:
	XMFLOAT4X4	mWorld;
	GameMesh*	mMesh;

	FLOAT		mExtentY;
	XNA::Sphere mBS;
	XNA::AxisAlignedBox mAABB;

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

private:
	UINT mID;
	static UINT GeneratedCount;

	bool m_bForOneHit;
	bool mHasTarget;
};


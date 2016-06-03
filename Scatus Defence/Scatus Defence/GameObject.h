#pragma once
#include "Camera.h"
#include "RenderStates.h"
#include "ResourceMgr.h"
#include "Effects.h"
#include "GameMesh.h"
#include "Properties.h"
#include <list>
#include "2d/Vector2D.h"

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
	GameObject(GameMesh* mesh);
	virtual ~GameObject();

public:
	virtual void Walk(float dt) = 0;
	virtual void Strafe(float dt) = 0;
	virtual void MoveTo(Vector2D direction, float dt) = 0; //xz평면 안에서의 이동
	virtual void RotateY(float angle) = 0;
	virtual void Update(float dt) = 0;

	virtual void DrawToScene(ID3D11DeviceContext* dc, const Camera& cam, const XMFLOAT4X4& shadowTransform, const FLOAT& tHeight) = 0;
	virtual void DrawToShadowMap(ID3D11DeviceContext* dc, const Camera& cam, const XMMATRIX& lightViewProj, const FLOAT& tHeight) = 0;
	virtual void DrawToSsaoNormalDepthMap(ID3D11DeviceContext* dc, const Camera& cam, const FLOAT& tHeight) = 0;

	virtual void Release(ResourceMgr& rMgr) = 0;

	GameMesh*		GetMesh() const { return mMesh; }
	XMFLOAT4X4		GetWorld() const { return mWorld; }
	XMFLOAT3		GetPos() const { return mPosition; }
	XMFLOAT3		GetLook() const { return mCurrLook; }
	XMFLOAT3		GetRight() const { return mRight; }
	XMFLOAT3		GetUp() const { return mUp; }
	UINT			GetID() const { return mID; }
	UINT			GetObjectGeneratedCount() const { return GeneratedCount; }
	Properties		GetProperty() const { return mProperty; }
	state_type		GetState() const{ return mProperty.state; }
	Vector2D		GetPos2D() const { return Vector2D(mPosition.x, mPosition.z); }
	GameObject*		GetTarget() const { return mTarget; }
	Object_type		GetType() const { return mObjectType; }

	void			SetState(state_type state) { mProperty.state = state; }
	void			SetHP(int hp) { mProperty.hp_now = hp; }
	void			SetTarget(GameObject* target) { if (target) mTarget = target; }

	void  PrintLocation(); //객체 위치값출력 반환
private:
	UINT mID;
	static UINT GeneratedCount;

protected:
	XMFLOAT4X4 mWorld;
	GameMesh* mMesh;

	FLOAT	 mScaling;
	XMFLOAT3 mRotation;
	XMFLOAT3 mPosition;

	XMFLOAT3 mRight;
	XMFLOAT3 mUp;
	XMFLOAT3 mCurrLook;
	XMFLOAT3 mPrevLook;
	XMFLOAT3 mDirection;

	Properties mProperty;
	Object_type mObjectType;
	GameObject* mTarget;
};


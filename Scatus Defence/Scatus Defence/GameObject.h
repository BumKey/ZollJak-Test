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
	FLOAT Yaw;
	FLOAT Scale;
};



class GameObject
{
public:
	GameObject(GameMesh* mesh);
	virtual ~GameObject();

public:
	virtual void Walk(float d) = 0;
	virtual void Strafe(float d) = 0;
	virtual void RotateY(float angle) = 0;
	virtual void Update() = 0;

	virtual void DrawToScene(ID3D11DeviceContext* dc, const Camera& cam, XMFLOAT4X4 shadowTransform, FLOAT tHeight) = 0;
	virtual void DrawToShadowMap(ID3D11DeviceContext* dc, const Camera& cam, const XMMATRIX& lightViewProj, FLOAT tHeight) = 0;
	virtual void DrawToSsaoNormalDepthMap(ID3D11DeviceContext* dc, const Camera& cam, FLOAT tHeight) = 0;

	virtual void Release(ResourceMgr& rMgr) = 0;

	virtual int Get_Obj_type() { return 0; };
	virtual int SetObj_State(int new_state) { return 0; } // ������Ʈ ���¼���
	virtual Properties * Get_Properties() { return NULL; } // ������Ʈ �Ӽ���ȯ
	virtual int Get_States() { return 0; } // ������Ʈ ���¹�ȯ
	void SettingTarget(std::list<GameObject*> Oppenent); // ������Ʈ�� ���� ������ ����Ʈ
	void Attack(GameObject * Target); // ����
	int Get_Object_type() { return m_Object_type; }
	Vector2D Heading()const { return m_vHeading; }
	void      SetHeading(Vector2D new_heading);

	GameMesh*		GetMesh() const { return mMesh; }
	XMFLOAT4X4		GetWorld() const { return mWorld; }
	XMFLOAT3		GetPos() const { return mPosition; }
	XMFLOAT3		GetLook() const { return mCurrLook; }
	XMFLOAT3		GetRight() const { return mRight; }
	XMFLOAT3		GetUp() const { return mUp; }
	UINT			GetID() const { return mID; }
	UINT			GetObjectGeneratedCount() const { return GeneratedCount; }
	Vector2D		GetPos2D() const {return Vector2D(mPosition.x, mPosition.y); }
	void Move2D(Vector2D direction, float dt); //xz��� �ȿ����� �̵�
	void  printlocation(); //��ü ��ġ����� ��ȯ

	GameObject * current_target_obj;
	std::list<GameObject*> Oppenents;
private:
	UINT mID;
	static UINT GeneratedCount;
	int m_Object_type; // ������Ʈ�� �������� ������ ��Ÿ���� ���� 

protected:
	XMFLOAT4X4 mWorld;
	GameMesh* mMesh;
	Vector2D    m_vHeading;

	FLOAT	 mScaling;
	XMFLOAT3 mRotation;
	XMFLOAT3 mPosition;

	XMFLOAT3 mRight;
	XMFLOAT3 mUp;
	XMFLOAT3 mCurrLook;
	XMFLOAT3 mPrevLook;



};


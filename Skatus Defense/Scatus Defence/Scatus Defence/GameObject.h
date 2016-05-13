#pragma once

#include "BasicModel.h"
#include "d3dUtil.h"
#include <Camera.h>
#include "RenderStates.h"
#include "Effects.h"
#include "game/MovingEntity.h"
#include "Properties.h"
#include "TargetingSystem.h"
#include <list>
class TargetingSystem;
enum Object_type
{
	type_p_warrior, // ĳ���� ����
	type_p_archer, // ĳ���� ��ó
	type_p_builder, // ĳ���� ���డ
	type_goblin,// ��- ���
	type_object,// ��Ÿ ������Ʈ(������)
};



// ���� ���, �޽� ������, �׸��� �Լ��� ����
// ���� ��ü ���º�ȭ ����

// Ŭ���� ���� : ��ƼƼ

enum Model_Effect {
	Base = 0, Alpha
};
//template <typename Properties>
class GameObject : public MovingEntity
{
public:
	GameObject();
	GameObject(BasicModel* model, XMFLOAT4X4 world, Model_Effect me, int obj_type);
	GameObject(BasicModel * model, XMFLOAT4X4 world, Model_Effect me, int obj_type, Vector2D location);
	~GameObject();
	GameObject * current_target_obj;
	std::list<GameObject*> Oppenents;
	
	float BoundingRadius;
private:
	XMFLOAT4X4 mWorld;
	BasicModel* mModel;

	Model_Effect mME;
	int m_Object_type;
public:
	void Draw(ID3D11DeviceContext* dc, const Camera& cam, XMFLOAT4X4 shadowTransform);
	void Animate();
	void Move(XMVECTOR direction, float dt);
	void Move2D(Vector2D direction, float dt);
	void DrawObjectToShadowMap(ID3D11DeviceContext* dc, const Camera& cam, const XMMATRIX& lightViewProj);
	void DrawObjectToSsaoNormalDepthMap(ID3D11DeviceContext* dc, const Camera& cam);
	void  printlocation(); //��ü ��ġ����� ��ȯ

	Model_Effect GetType();
	BasicModel* GetModel();
	int Get_Object_type() { return m_Object_type; }
	XMFLOAT4X4 GetWorld();
	virtual int SetObj_State(int new_state) { return 0; }
	virtual Properties * Get_Properties() { return NULL; }
	virtual int Get_States() { return 0; }
	void SettingTarget(std::list<GameObject*> Oppenent);
	void Attack(GameObject * Target);



};


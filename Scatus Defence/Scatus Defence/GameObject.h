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
	type_p_warrior, // 캐릭터 전사
	type_p_archer, // 캐릭터 아처
	type_p_builder, // 캐릭터 건축가
	type_goblin,// 적- 고블린
	type_object,// 기타 오브젝트(지형등)
};



// 월드 행렬, 메쉬 포인터, 그리기 함수를 가진
// 게임 객체 상태변화 관리

// 클래스 유형 : 엔티티

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
	void  printlocation(); //객체 위치값출력 반환

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


#pragma once
#include"GameObject.h"
#include "Properties.h"

class Enemy : public GameObject
{
	public:
		virtual Properties * Get_Properties() { return NULL; }
		virtual int Get_States() { return 0; }
	Enemy(BasicModel* model, XMFLOAT4X4 world, Model_Effect me, int obj_type, Vector2D location) :GameObject(model, world, me, obj_type, location)
	{
			
		m_vPosition = location;
	}
	~Enemy();
};

class Goblin : public Enemy
{
public: GoblinProperties goblin_properties;
	   virtual Properties * Get_Properties() { return &goblin_properties; }
	   virtual int Get_States() { return goblin_properties.state; }
	 virtual int SetObj_State(int new_state) { goblin_properties.state = new_state; return new_state; }

	Goblin(BasicModel* model, XMFLOAT4X4 world, Model_Effect me, int obj_type, Vector2D location) :Enemy(model, world, me, obj_type, location)
	{
		printf("고블린이 생성되었습니다.\n");
		m_dMaxSpeed = goblin_properties.movespeed;
		m_vPosition = location;


	}
	~Goblin()
	{
		printf("고블린이 삭제되었습니다.\n");
	}
//	virtual Properties* Get_Properties() { return &goblin_properties; }
};

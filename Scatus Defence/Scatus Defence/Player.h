#pragma once
#include "GameObject.h"
#include "Properties.h"

class Player : public GameObject
{
public:

	virtual Properties * Get_Properties() { return NULL; }
	Player(BasicModel* model, XMFLOAT4X4 world, Model_Effect me, int obj_type ,Vector2D location) :GameObject(model, world, me, obj_type, location)
	{
	}
	~Player();
	virtual int Get_States() { return 0; }
};

class Warrior : public Player
{
	public :
		Warriorproperties warrior_p;
		virtual int Get_States() { return warrior_p.state; }
		virtual Properties * Get_Properties() { return &warrior_p;}
		virtual int SetObj_State(int new_state) { warrior_p.state = new_state; return new_state; }
		Warrior(BasicModel* model, XMFLOAT4X4 world, Model_Effect me, int obj_type, Vector2D location) :Player(model, world, me, obj_type, location)
		{
			
		};
		~Warrior();

};




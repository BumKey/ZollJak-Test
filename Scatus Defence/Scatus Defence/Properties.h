#pragma once

namespace AttackType {
	enum Types
	{
		type_shortdistance,
		type_longdistance
	};
}

namespace ActionState {
	enum States
	{
		Idle,
		Battle,
		Walk,
		Run,
		Die,
		Build,
		Attack,
		Damage
	};
}

namespace AI_State {
	enum States
	{
		None,
		MovingToTarget,
		AttackToTarget
	};
}

namespace CollisionState {
	enum States
	{
		None,
		MovingCollision,
		AttackCollision
	};
}

namespace ObjectType {
	enum Types
	{
		None = 0,

		Player,
		Warrior, // 캐릭터 전사
		Archer, // 캐릭터 아처
		Builder, // 캐릭터 건축가
		
		Monster,
		Goblin, // 적- 고블린
		Cyclop,
		
		Obstacle,
		Tree,
		Base,
		Stairs,
		Pillar1,
		Pillar2,
		Pillar3,
		Pillar4,
		Rock,
		Temple
	};
}

class Properties
{
public:
	char * name;
	int id;
	int pw;
	int level;
	int gold;
	int exp_now;
	int hp_now;
	int hp_max;
	int exp_max;
	int respawn_loc;
	int guardpoint;
	int restoring_hp;
	int attack_type;
	int Upgrade_weapon;
	int Upgrade_guard;
	int army_list;
	int obj_type;
	float attackspeed;
	float movespeed;
	float attackrange;
	float attakpoint;
	Properties();
	~Properties();
	void printPropeties();
	void Levelup();
	void Upgrade();
};
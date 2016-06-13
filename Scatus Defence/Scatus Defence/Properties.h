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
		type_p_warrior, // 캐릭터 전사
		type_p_archer, // 캐릭터 아처
		type_p_builder, // 캐릭터 건축가
		Player,
		Goblin, // 적- 고블린
		Cyclop,
		Obstacle,// 기타 오브젝트(지형등)
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
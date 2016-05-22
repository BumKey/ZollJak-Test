#pragma once


enum attack_type
{
	type_shortdistance,
	type_longdistance
};

enum state_type
{
	type_null,
	type_battle,
	type_walk,
	type_run,
	type_die,
	type_build,
	type_attack,
	type_attacked,
	type_idle
};

enum Object_type
{
	type_p_warrior, // 캐릭터 전사
	type_p_archer, // 캐릭터 아처
	type_p_builder, // 캐릭터 건축가
	goblin, // 적- 고블린
	cyclop,
	type_object,// 기타 오브젝트(지형등)
};

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
	state_type state;
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

class Warriorproperties : public Properties
{
public:
	Warriorproperties();
	~Warriorproperties();

};

class GoblinProperties : public Properties
{
public:
	GoblinProperties();

	~GoblinProperties();

};
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
	type_die,
	type_build,
	type_run,
	type_attack,
	type_attacked,
	type_idle
};
class Properties
{
public:
	char * name;
	int id;
	int pw;
	int gold;
	int exp_now;
	int hp_now;
	int hp_max;
	int exp_max;
	int movespeed;
	int respawn_loc;
	int attakpoint;
	int guardpoint;
	int restoring_hp;
	int attack_type;
	int Upgrade_weapon;
	int Upgrade_guard;
	int state;
	int army_list;
	float attackspeed;
	int level;
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
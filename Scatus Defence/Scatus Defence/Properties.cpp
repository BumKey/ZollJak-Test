#include "Properties.h"

Properties::Properties()
{
	name = nullptr;
	id = 0;
	pw = 0;
	gold = 0;
	exp_now = 0;
	hp_max = 0;
	hp_now = hp_max;
	exp_max = 300;
	movespeed = 0;
	respawn_loc = 1;
	attakpoint = 0;
	guardpoint = 0;
	restoring_hp = 10;
	attack_type = 0;
	Upgrade_weapon = 0;
	Upgrade_guard = 0;
	army_list = 0;
	attackrange = 0;
}


Properties::~Properties()
{
}

void Levelup()
{


}
void Upgrade()
{

}

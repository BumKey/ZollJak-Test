#pragma once
#include<vector>
#include "2d/Vector2D.h"
#include <iostream>
#include <fstream>
#include <stdio.h>

using namespace std;

enum map_entity {
	map_null,
	map_nothing,
	map_building,
	map_wall,
	map_temple,
	map_enemy,
	map_untouchable,//¸Ê ¿Ü°û
};
struct Wall2D
{
	Vector2D m_vPosition;

};

class Map
{
public:
	vector<Wall2D> m_Walls;
	
	Vector2D temple_loc;
	Vector2D enemy_loc;
	Map();
	~Map();
	void Map::print();
	void Save(char* FileName);
	void Load(char* FileName);
};


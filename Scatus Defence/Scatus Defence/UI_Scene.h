#pragma once
#include <list>
#include <vector>
enum type_Scene {

	Scene_Title,
	Scene_Title_Setting,
	Scene_Ingame,
	Scene_Wave

};
class UI_Scene
{

private:
public:
	 std::vector<int> UI_Image_list;
	std::vector<int> UI_Text_List;
	
	wchar_t* Scene_name;
	UI_Scene();
	UI_Scene(wchar_t* name) { Scene_name = name; }
	~UI_Scene();
};


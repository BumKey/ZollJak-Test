#pragma once
#include <vector>

class GameObject;
class TargetingSystem
{
public:
	GameObject * m_pOwner;

	
	
	void SettingTarget(std::vector<GameObject*> Oppenent);
	TargetingSystem();
	~TargetingSystem();
};


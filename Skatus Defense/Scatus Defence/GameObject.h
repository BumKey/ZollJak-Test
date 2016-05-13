#pragma once
#include "game/MovingEntity.h"
#include "Vector2D.h"
class GameObject : public MovingEntity
{
	
public:

	virtual void Update() = 0;
	virtual void Render() = 0;
	GameObject();
	~GameObject();
};


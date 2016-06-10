#pragma once
#include "ObjectMgr.h"
#include "Utilities.h"
#include "2DMap.h"

class CollisionMgr
{
public:
	CollisionMgr();
	~CollisionMgr();

public:
	void Init(ObjectMgr* objectMgr);
	void Update(float dt);

	/// <summary>
	/// Notice : ���� �� �޼���� �÷��̾� ���ݸ� ����.
	/// ������ ���� �浹������ ���� �ൿ������ ���鿡�� RogicMgr�� �ִ�. 
	/// ���� �浹������ ��ȭ�� ����ٸ� �� �� �� �����ؾ��Ѵٴ� ���ŷο��� �ִ�.
	void AttackCollision();

private:
	void MovingCollision(float dt);

	bool LowDetectWithMonsters(GameObject* sourceObj);
	void HighDetectWithMonsters(GameObject* sourceObj, std::vector<UINT>& outIndices);

private:
	ObjectMgr* mObjectMgr;
	std::vector<Monster*> mMonsters;
	Player* mPlayer;
};


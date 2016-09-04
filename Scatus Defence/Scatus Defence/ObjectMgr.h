#pragma once
#include <vector>
#include <map>

#include "Singletone.h"
#include "ResourceMgr.h"
#include "PacketMgr.h"
#include "BasicObject.h"
#include "Goblin.h"
#include "Cyclop.h"
#include "Warrior.h"

#define Object_Mgr ObjectMgr::GetInstance()

// ������Ʈ ������ ���� Ŭ����
// Notice: ������ �и����� ���Ͽ� ������Ʈ �׸����
// SceneMgr���� ���.
class ObjectMgr : public Singletone<ObjectMgr>
{
private:
	ObjectMgr();
	~ObjectMgr();

	friend class Singletone<ObjectMgr>;
public:
	void AddMainPlayer(SkinnedObject* obj, const UINT& id);
	void AddOtherPlayer(SO_InitDesc desc, const UINT& id);
	void AddMonster(const ObjectType::Types& type, const SO_InitDesc& desc, const UINT& id);
	void AddObstacle(const ObjectType::Types& type, const BO_InitDesc& desc);

	void RemovePlayer(const UINT& id);

	const std::vector<GameObject*>&		GetAllObjects() { return mAllObjects; }
	UINT								GetCurrPlayerNum() const { return mCurrPlayerNum; }
	void UpdatePlayer(const UINT& id, const ObjectInfo& info);
	void UpdateMonster(const UINT& id, const ObjectInfo& info);
	void Update(float dt);

private:
	UINT mCurrPlayerNum;

	std::vector<GameObject*>						mAllObjects;					
	std::vector<BasicObject*>						mBasicObjects;
	std::vector<Monster*>							mMonsters;
	SkinnedObject*									mPlayers[MAX_USER];
};


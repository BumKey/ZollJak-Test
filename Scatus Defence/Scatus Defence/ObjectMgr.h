#pragma once
#include "GameObject.h"
#include "BasicObject.h"
#include "SkinnedObject.h"
#include "ResourceMgr.h"
#include "Player.h"
#include "Goblin.h"
#include "Cyclop.h"
#include "ResourceMgr.h"
#include "Singletone.h"
#include <vector>
#include <map>

#define Object_Mgr ObjectMgr::GetInstance()

// 오브젝트 관리를 위한 클래스
// Notice: 역할의 분명함을 위하여 오브젝트 그리기는
// SceneMgr에서 담당.
class ObjectMgr : public Singletone<ObjectMgr>
{
private:
	ObjectMgr();
	~ObjectMgr();

	friend class Singletone<ObjectMgr>;
public:
	void AddPlayer(SkinnedObject* player) { mSkinnedObjects[mCurrPlayerNum++] = player; }
	void AddMonster(const ObjectType::Types& type, const SO_InitDesc& desc, const UINT& id);
	void AddObstacle(const ObjectType::Types& type, const BO_InitDesc& desc);

	const std::vector<GameObject*>&		GetAllObjects() { return mAllObjects; }

	void Update(const UINT& id, const ObjectInfo& info);
	void Update(float dt);

private:
	UINT mCurrPlayerNum;

	std::vector<GameObject*>						mAllObjects;					
	std::vector<BasicObject*>						mBasicObjects;
	std::unordered_map<UINT, SkinnedObject*>		mSkinnedObjects;
};


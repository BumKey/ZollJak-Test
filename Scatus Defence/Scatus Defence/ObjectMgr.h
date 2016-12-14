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
#include "Temple.h"

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
	void AddMainPlayer(SkinnedObject* obj, const UINT& id);
	void AddOtherPlayer(SO_InitDesc desc, const UINT& id);
	void AddMonster(const ObjectType::Types& type, const SO_InitDesc& desc, const UINT& id);
	void AddObstacle(const ObjectType::Types& type, const BO_InitDesc& desc);

	void RemovePlayer(const UINT& id);
	void ReleaseMonsters();
	void KillAllMonsteres();

	const std::vector<GameObject*>&		GetAllObjects() { return mAllObjects; }
	SkinnedObject**						GetPlayers() { return mPlayers; }
	std::unordered_map<UINT, Monster*>	GetMonsters() { return mMonsters; }
	UINT								GetCurrPlayerNum() const { return mCurrPlayerNum; }
	const XMFLOAT3						GetTemplePos() { return mTemplePos; }

	void UpdatePlayer(const UINT& id, const PlayerInfos& info);
	void UpdateMonster(const UINT& id, const MonInfos& info);
	void Update(float dt);

private:
	UINT mCurrPlayerNum;

	XMFLOAT3 mTemplePos;

	std::vector<GameObject*>						mAllObjects;					
	std::vector<BasicObject*>						mBasicObjects;
	std::unordered_map<UINT, Monster*>				mMonsters;
	SkinnedObject*									mPlayers[MAX_USER];
};


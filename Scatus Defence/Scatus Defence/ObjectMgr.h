#pragma once
#include "GameObject.h"
#include "BasicObject.h"
#include "SkinnedObject.h"
#include "ResourceMgr.h"
#include "Player.h"
#include <list>
#include <vector>
#include <map>

// ������Ʈ ������ ���� Ŭ����
// Notice: ������ �и����� ���Ͽ� ������Ʈ �׸����
// SceneMgr���� ���.
class ObjectMgr
{
public:
	ObjectMgr();
	~ObjectMgr();

	void AddObstacle(BasicObject* basicObject);
	void AddStructure(BasicObject* basicObject);
	void AddProjectile(BasicObject* basicObject);
	void AddMonster(SkinnedObject* skinnedObject);

	const std::vector<GameObject*>& GetAllObjects() { return mAllObjects; }
	void SetPlayer(Player* player) { mPlayer = player; }

	void Update();
	void Update(float dt);

	void ReleaseAll(ResourceMgr& resourceMgr);

private:
	UINT mStage;
	UINT mMaxMonsters;
	UINT mMaxStructures;
	UINT mTotalObjectNum;

	Player* mPlayer;
	std::vector<GameObject*>	mAllObjects;					// �ݺ� ��ȸ�� ���� ���� ���ͷ�
	std::vector<BasicObject*>	mObstacles;						// ������ �� ����, ������ �̷������ �ʴ� ������������ �����̳�
	std::list<BasicObject*>		mStructures, mProjectiles;		// ����, ���� ���ɼ��� �ִ� BasicObject(�ǹ�, �߻�ü)
	std::list<SkinnedObject*>	mMonsters;						// ����, ���� ���ɼ��� �ִ� SkinnedObject 
};


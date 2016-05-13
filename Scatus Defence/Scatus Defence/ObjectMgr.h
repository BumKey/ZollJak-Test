#pragma once
#include "GameObject.h"
#include "BasicObject.h"
#include "SkinnedObject.h"
#include "Player.h"
#include <list>
#include <vector>
#include <map>

// 오브젝트 관리를 위한 클래스
// Notice: 역할의 분명함을 위하여 오브젝트 그리기는
// SceneMgr에서 담당.
class ObjectMgr
{
public:
	ObjectMgr();
	~ObjectMgr();

	void AddObstacle(BasicObject* basicObject);
	void AddStructure(BasicObject* basicObject);
	void AddProjectile(BasicObject* basicObject);
	void AddMonster(SkinnedObject* skinnedObject);

	std::vector<GameObject*>& GetAllObjects() { return mAllObjects; } const
	void SetPlayer(Player* player) { mPlayer = player; }

	void Update();
	void Update(float dt);
private:
	UINT mStage;
	UINT mMaxMonsters;
	UINT mMaxStructures;
	UINT mTotalObjectNum;

	Player* mPlayer;
	std::vector<GameObject*>	mAllObjects;					// 반복 순회가 가장 빠른 벡터로
	std::vector<BasicObject*>	mObstacles;						// 생성된 후 삽입, 삭제가 이루어지지 않는 지형지물들의 컨테이너
	std::list<BasicObject*>		mStructures, mProjectiles;		// 삽입, 삭제 가능성이 있는 BasicObject(건물, 발사체)
	std::list<SkinnedObject*>	mMonsters;						// 삽입, 삭제 가능성이 있는 SkinnedObject 
};


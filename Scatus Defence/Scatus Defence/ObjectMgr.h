#pragma once
#include "GameObject.h"
#include "BasicObject.h"
#include "SkinnedObject.h"
#include "ResourceMgr.h"
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
	//Add
	bool AddObstacle(BasicObject* basicObject);//bool 형으로 해서 나중에 객체 생성한계초과여부 확인
	bool AddStructure(BasicObject* basicObject);
	bool AddProjectile(BasicObject* basicObject);
	bool AddMonster(GameObject* skinnedObject); // 충돌검사 길찾기를 위한 몬스터리스트, 다형성 구현을 위해skinnedObject*에서  GameObject*로
	bool AddOurTeam(GameObject* skinnedObject); // 충돌검사, 길찾기를 위한 아군 리스트
	//Get
	const std::list<GameObject*>& GetAllObjects() { return mAllObjects; }// 리스트로변경
	const std::list<GameObject*>& GetAllMonsters() { return mMonsters;}// 리스트로변경
	const std::list<GameObject*>& GetAllOurTeam() { return mOurTeam; }// 리스트로변경
	
	void SetPlayer(Player* player) { mPlayer = player; }

	void Update();
	void Update(float dt);

	void ReleaseAll(ResourceMgr& resourceMgr);
	void ReleaseAllMonsers(ResourceMgr& resourceMgr);
	void ReleaseAllDeads(ResourceMgr& resourceMgr);

private:
	UINT mStage;
	UINT mMaxMonsters;
	UINT mMaxStructures;
	UINT mTotalObjectNum;

	Player* mPlayer;
	std::list<GameObject*>	mAllObjects;					// 반복 순회가 가장 빠른 벡터로
	std::vector<BasicObject*>	mObstacles;						// 생성된 후 삽입, 삭제가 이루어지지 않는 지형지물들의 컨테이너
	std::list<BasicObject*>		mStructures, mProjectiles;		// 삽입, 삭제 가능성이 있는 BasicObject(건물, 발사체), 낮은 가능성, 상황을 봐서 건물은 mOurs에 병합될 수도 있음
	std::list<GameObject*>	mMonsters;						// 삽입, 삭제 가능성이 있는 SkinnedObject 
	std::list<GameObject*>	mOurTeam;				// 주로 아군 전사 NPC등을 넣을 리스트 // 현재는 단순히 플레이어 등을 넣어두는 리스트로 사용하겠음


};


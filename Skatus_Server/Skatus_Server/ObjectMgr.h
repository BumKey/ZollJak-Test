#pragma once
#include "protocol.h"
#include <vector>

class ObjectMgr
{
public:
	ObjectMgr();
	~ObjectMgr();

	void AddObject(ObjectType::Types oType);

	const std::vector<ForClientInfo>&		GetAllObjects();
	void ReleaseAll();
	void ReleaseAllMonsters();

private:
	void CreateMap();

private:
	UINT mStage;
	UINT mMaxMonsters;
	UINT mMaxStructures;
	UINT mTotalObjectNum;

	std::vector<ForClientInfo>	mAllObjects;					// 반복 순회가 가장 빠른 벡터로
	std::vector<ForClientInfo>	mObstacles;						// 생성된 후 삽입, 삭제가 이루어지지 않는 지형지물들의 컨테이너
	std::vector<ForClientInfo>	mStructures, mProjectiles;		// 삽입, 삭제 가능성이 있는 BasicObject(건물, 발사체), 낮은 가능성, 상황을 봐서 건물은 mOurs에 병합될 수도 있음
	std::vector<ForClientInfo>	mMonsters;						// 삽입, 삭제 가능성이 있는 SkinnedObject 
	std::vector<ForClientInfo>	mOurTeam;				// 주로 아군 전사 NPC등을 넣을 리스트 // 현재는 단순히 플레이어 등을 넣어두는 리스트로 사용하겠음
	std::vector<ForClientInfo>	mOppenents;
};
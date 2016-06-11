#pragma once
#include "GameObject.h"
#include "BasicObject.h"
#include "SkinnedObject.h"
#include "ResourceMgr.h"
#include "Player.h"
#include "Monster.h"
#include "ResourceMgr.h"
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

public:
	void Init(ResourceMgr* resourceMgr);


	bool AddProjectile(BasicObject* basicObject);
	bool AddMonster(Monster* monster); // �浹�˻� ��ã�⸦ ���� ���͸���Ʈ, ������ ������ ����skinnedObject*����  GameObject*��
	bool AddOurTeam(SkinnedObject* skinnedObject); // �浹�˻�, ��ã�⸦ ���� �Ʊ� ����Ʈ


	const std::vector<GameObject*>&		GetAllObjects() { return mAllObjects; }
	std::vector<Monster*>&				GetMonsters() { return mMonsters; }
	std::vector<GameObject*>&			GetOurTeam() { return mOurTeam; }
	std::vector<GameObject*>&			GetOppenents() { return mOurTeam; }

	Player* GetPlayer() { return mPlayer; }

	void Update();
	void Update(float dt);

	void ReleaseAll();
	void ReleaseAllMonsters();

private:
	void CreateMap();
	bool AddObstacle(BasicObject* basicObject);//bool ������ �ؼ� ���߿� ��ü �����Ѱ��ʰ����� Ȯ��
	bool AddStructure(BasicObject* basicObject);

private:
	UINT mStage;
	UINT mMaxMonsters;
	UINT mMaxStructures;
	UINT mTotalObjectNum;

	Player* mPlayer;
	std::vector<GameObject*>	mAllObjects;					// �ݺ� ��ȸ�� ���� ���� ���ͷ�
	std::vector<BasicObject*>	mObstacles;						// ������ �� ����, ������ �̷������ �ʴ� ������������ �����̳�
	std::vector<BasicObject*>		mStructures, mProjectiles;		// ����, ���� ���ɼ��� �ִ� BasicObject(�ǹ�, �߻�ü), ���� ���ɼ�, ��Ȳ�� ���� �ǹ��� mOurs�� ���յ� ���� ����
	std::vector<Monster*>			mMonsters;						// ����, ���� ���ɼ��� �ִ� SkinnedObject 
	std::vector<GameObject*>		mOurTeam;				// �ַ� �Ʊ� ���� NPC���� ���� ����Ʈ // ����� �ܼ��� �÷��̾� ���� �־�δ� ����Ʈ�� ����ϰ���
	std::vector<GameObject*>		mOppenents;

	ResourceMgr* mResourceMgr;
};


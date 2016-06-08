#pragma once
#include "GameObject.h"
#include "BasicObject.h"
#include "SkinnedObject.h"
#include "ResourceMgr.h"
#include "Player.h"
#include "Monster.h"
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

public:
	//Add
	bool AddObstacle(BasicObject* basicObject);//bool ������ �ؼ� ���߿� ��ü �����Ѱ��ʰ����� Ȯ��
	bool AddStructure(BasicObject* basicObject);
	bool AddProjectile(BasicObject* basicObject);
	bool AddMonster(Monster* monster); // �浹�˻� ��ã�⸦ ���� ���͸���Ʈ, ������ ������ ����skinnedObject*����  GameObject*��
	bool AddOurTeam(SkinnedObject* skinnedObject); // �浹�˻�, ��ã�⸦ ���� �Ʊ� ����Ʈ

												   //Get
	const std::vector<GameObject*>& GetAllObjects() { return mAllObjects; }
	std::list<Monster*>&			GetMonsters() { return mMonsters; }
	std::list<GameObject*>&			GetOurTeam() { return mOurTeam; }
	std::list<GameObject*>&			GetOppenents() { return mOurTeam; }

	Player* GetPlayer() { return mPlayer; }
	void SetPlayer(Player* player) { mPlayer = player; }

	void Update();
	void Update(float dt);

	void ReleaseAll(ResourceMgr& resourceMgr);
	void ReleaseAllMonsters(ResourceMgr& resourceMgr);

private:
	UINT mStage;
	UINT mMaxMonsters;
	UINT mMaxStructures;
	UINT mTotalObjectNum;

	Player* mPlayer;
	std::vector<GameObject*>	mAllObjects;					// �ݺ� ��ȸ�� ���� ���� ���ͷ�
	std::vector<BasicObject*>	mObstacles;						// ������ �� ����, ������ �̷������ �ʴ� ������������ �����̳�
	std::list<BasicObject*>		mStructures, mProjectiles;		// ����, ���� ���ɼ��� �ִ� BasicObject(�ǹ�, �߻�ü), ���� ���ɼ�, ��Ȳ�� ���� �ǹ��� mOurs�� ���յ� ���� ����
	std::list<Monster*>			mMonsters;						// ����, ���� ���ɼ��� �ִ� SkinnedObject 
	std::list<GameObject*>		mOurTeam;				// �ַ� �Ʊ� ���� NPC���� ���� ����Ʈ // ����� �ܼ��� �÷��̾� ���� �־�δ� ����Ʈ�� ����ϰ���
	std::list<GameObject*>		mOppenents;

};


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
	//Add
	bool AddObstacle(BasicObject* basicObject);//bool ������ �ؼ� ���߿� ��ü �����Ѱ��ʰ����� Ȯ��
	bool AddStructure(BasicObject* basicObject);
	bool AddProjectile(BasicObject* basicObject);
	bool AddMonster(GameObject* skinnedObject); // �浹�˻� ��ã�⸦ ���� ���͸���Ʈ, ������ ������ ����skinnedObject*����  GameObject*��
	bool AddOurTeam(GameObject* skinnedObject); // �浹�˻�, ��ã�⸦ ���� �Ʊ� ����Ʈ
	//Get
	const std::list<GameObject*>& GetAllObjects() { return mAllObjects; }// ����Ʈ�κ���
	const std::list<GameObject*>& GetAllMonsters() { return mMonsters;}// ����Ʈ�κ���
	const std::list<GameObject*>& GetAllOurTeam() { return mOurTeam; }// ����Ʈ�κ���
	
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
	std::list<GameObject*>	mAllObjects;					// �ݺ� ��ȸ�� ���� ���� ���ͷ�
	std::vector<BasicObject*>	mObstacles;						// ������ �� ����, ������ �̷������ �ʴ� ������������ �����̳�
	std::list<BasicObject*>		mStructures, mProjectiles;		// ����, ���� ���ɼ��� �ִ� BasicObject(�ǹ�, �߻�ü), ���� ���ɼ�, ��Ȳ�� ���� �ǹ��� mOurs�� ���յ� ���� ����
	std::list<GameObject*>	mMonsters;						// ����, ���� ���ɼ��� �ִ� SkinnedObject 
	std::list<GameObject*>	mOurTeam;				// �ַ� �Ʊ� ���� NPC���� ���� ����Ʈ // ����� �ܼ��� �÷��̾� ���� �־�δ� ����Ʈ�� ����ϰ���


};


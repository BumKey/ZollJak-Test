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

	std::vector<ForClientInfo>	mAllObjects;					// �ݺ� ��ȸ�� ���� ���� ���ͷ�
	std::vector<ForClientInfo>	mObstacles;						// ������ �� ����, ������ �̷������ �ʴ� ������������ �����̳�
	std::vector<ForClientInfo>	mStructures, mProjectiles;		// ����, ���� ���ɼ��� �ִ� BasicObject(�ǹ�, �߻�ü), ���� ���ɼ�, ��Ȳ�� ���� �ǹ��� mOurs�� ���յ� ���� ����
	std::vector<ForClientInfo>	mMonsters;						// ����, ���� ���ɼ��� �ִ� SkinnedObject 
	std::vector<ForClientInfo>	mOurTeam;				// �ַ� �Ʊ� ���� NPC���� ���� ����Ʈ // ����� �ܼ��� �÷��̾� ���� �־�δ� ����Ʈ�� ����ϰ���
	std::vector<ForClientInfo>	mOppenents;
};
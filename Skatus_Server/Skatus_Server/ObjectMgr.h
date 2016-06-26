#pragma once
#include "Protocol.h"
#include "MathHelper.h"
#include <vector>
#include <unordered_map>

struct InstanceDesc
{
	XMFLOAT3 Pos;
	XMFLOAT3 Rot;
	FLOAT Scale;

	InstanceDesc() {
		Pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		Rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		Scale = 1.0f;
	}
};

// ������Ʈ ������ ���� Ŭ����
// Notice: ������ �и����� ���Ͽ� ������Ʈ �׸����
// SceneMgr���� ���.
class ObjectMgr
{
public:
	ObjectMgr();
	~ObjectMgr();

	void AddObject(ObjectType::Types oType, const InstanceDesc& info);

	const std::unordered_map<ObjectType::Types, InstanceDesc>&		GetObjects() { return mObjects; }
private:
	void CreateMap();

private:
	UINT mStage;
	UINT mMaxMonsters;
	UINT mMaxStructures;
	UINT mTotalObjectNum;

	InstanceDesc mPlayerInfo;
	std::unordered_map<ObjectType::Types, InstanceDesc> mObjects;
};


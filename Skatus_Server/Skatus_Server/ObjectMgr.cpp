//#include "ObjectMgr.h"
//
//// 임의의 수치임
//ObjectMgr::ObjectMgr() : mStage(1), mTotalObjectNum(0)
//{
//	mMaxMonsters = mStage * 200;
//	mMaxStructures = mStage * 5;
//
//	mPlayerInfo.Pos = XMFLOAT3(170.0f, 0.05f, -280.0f);
//	mPlayerInfo.Rot.y = 0.0f;
//	mPlayerInfo.Scale = 0.05f;
//
//	CreateMap();
//}
//
//ObjectMgr::~ObjectMgr()
//{
//}
//
//void ObjectMgr::AddObject(ObjectType::Types oType, const InstanceDesc& info)
//{
//	mObjects[oType] = info;
//}
//
//void ObjectMgr::CreateMap()
//{
//	InstanceDesc info;
//
//	info.Pos = XMFLOAT3(195.0f, 0.05f, -300.0f);
//	info.Rot.y = 0.0f;
//	info.Scale = 0.3f;
//
//	AddObject(ObjectType::Temple, info);
//
//	for (UINT i = 0; i < 10; ++i)
//	{
//		info.Pos = XMFLOAT3(mPlayerInfo.Pos.x + 50.0f - rand() % 100,
//			-0.1f, mPlayerInfo.Pos.z + 50.0f - rand() % 100);
//		info.Scale = MathHelper::RandF()*2.0f + 0.5f;
//		info.Rot.y = MathHelper::RandF()*MathHelper::Pi * 2;
//
//		AddObject(ObjectType::Tree, info);
//	}
//
//	for (UINT i = 0; i < 20; ++i)
//	{
//		info.Pos = XMFLOAT3(mPlayerInfo.Pos.x + 50.0f - rand() % 100,
//			-0.1f, mPlayerInfo.Pos.z + 50.0f - rand() % 100);
//		info.Scale = MathHelper::RandF()*2.0f + 0.5f;
//		info.Rot.y = MathHelper::RandF()*MathHelper::Pi * 2.0f;
//
//		AddObject(ObjectType::Rock, info);
//	}
//}
//
//

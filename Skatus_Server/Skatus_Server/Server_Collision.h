#pragma once


#include "ObjectMgr.h"


class Server_Collision
{
public:
	
	Server_Collision();
	~Server_Collision();
	void Col_Obstacle(BYTE C_ID);
	bool Col_AABB_Sphere(XNA::AxisAlignedBox& a , XNA::Sphere& b);
	void SetObjmgr(ObjectMgr &objmgr) { mObjectMgr = objmgr; }
	ObjectMgr GetObjmgr() {return mObjectMgr;}
private:
	ObjectMgr mObjectMgr;

};


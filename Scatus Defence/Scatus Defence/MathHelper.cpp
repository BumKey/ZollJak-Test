//***************************************************************************************
// MathHelper.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "MathHelper.h"
#include <float.h>
#include <cmath>

const float MathHelper::Infinity = FLT_MAX;
const float MathHelper::Pi = 3.1415926535f;

float MathHelper::AngleFromXY(float x, float y)
{
	float theta = 0.0f;

	// Quadrant I or IV
	if (x >= 0.0f)
	{
		// If x = 0, then atanf(y/x) = +pi/2 if y > 0
		//                atanf(y/x) = -pi/2 if y < 0
		theta = atanf(y / x); // in [-pi/2, +pi/2]

		if (theta < 0.0f)
			theta += 2.0f*Pi; // in [0, 2*pi).
	}

	// Quadrant II or III
	else
		theta = atanf(y / x) + Pi; // in [0, 2*pi).

	return theta;
}

float MathHelper::AngleToTarget(XMVECTOR vTarget, XMFLOAT3 currLook)
{
	// 방향으로 회전
	XMFLOAT3 fTargetDir;
	XMStoreFloat3(&fTargetDir, vTarget);

	float dot = -fTargetDir.x*currLook.x - fTargetDir.z*currLook.z;
	float det = -fTargetDir.x*currLook.z + fTargetDir.z*currLook.x;
	float angle = atan2(det, dot);

	return angle;
}

FLOAT MathHelper::DistanceVector(XMFLOAT3 l, XMFLOAT3 r)
{
	FLOAT a = l.x - r.x;
	FLOAT b = l.y - r.y;
	FLOAT c = l.z - r.z;
	return sqrtf(a*a + b*b + c*c);
}

bool MathHelper::Float3Equal(XMFLOAT3 l, XMFLOAT3 r)
{
	if (abs(l.x - r.x) <= 0.03f &&
		abs(l.z - r.z) <= 0.03f)
		return true;
	else
		return false;
}

XMVECTOR MathHelper::TargetVector2D(XMFLOAT3 target, XMFLOAT3 origin)
{
	XMFLOAT3 fTarget(target.x - origin.x, 0, target.z - origin.z);
	XMVECTOR vTarget = XMLoadFloat3(&fTarget);

	return XMVector3Normalize(vTarget);
}

XMVECTOR MathHelper::RandUnitVec3()
{
	XMVECTOR One = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	XMVECTOR Zero = XMVectorZero();

	// Keep trying until we get a point on/in the hemisphere.
	while (true)
	{
		// Generate random point in the cube [-1,1]^3.
		XMVECTOR v = XMVectorSet(MathHelper::RandF(-1.0f, 1.0f), MathHelper::RandF(-1.0f, 1.0f), MathHelper::RandF(-1.0f, 1.0f), 0.0f);

		// Ignore points outside the unit sphere in order to get an even distribution 
		// over the unit sphere.  Otherwise points will clump more on the sphere near 
		// the corners of the cube.

		if (XMVector3Greater(XMVector3LengthSq(v), One))
			continue;

		return XMVector3Normalize(v);
	}
}

XMVECTOR MathHelper::RandHemisphereUnitVec3(XMVECTOR n)
{
	XMVECTOR One = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	XMVECTOR Zero = XMVectorZero();

	// Keep trying until we get a point on/in the hemisphere.
	while (true)
	{
		// Generate random point in the cube [-1,1]^3.
		XMVECTOR v = XMVectorSet(MathHelper::RandF(-1.0f, 1.0f), MathHelper::RandF(-1.0f, 1.0f), MathHelper::RandF(-1.0f, 1.0f), 0.0f);

		// Ignore points outside the unit sphere in order to get an even distribution 
		// over the unit sphere.  Otherwise points will clump more on the sphere near 
		// the corners of the cube.

		if (XMVector3Greater(XMVector3LengthSq(v), One))
			continue;

		// Ignore points in the bottom hemisphere.
		if (XMVector3Less(XMVector3Dot(n, v), Zero))
			continue;

		return XMVector3Normalize(v);
	}
}
#include "CollisionMgr.h"



CollisionMgr::CollisionMgr()
{
}


CollisionMgr::~CollisionMgr()
{
}

bool CollisionMgr::FrustumAABBCulling(GameObject* obj)
{
	XNA::AxisAlignedBox aabb = obj->GetAABB();
	XMFLOAT3   pos = obj->GetPos();
	XMFLOAT4X4 world = obj->GetWorld();

	XMVECTOR detView = XMMatrixDeterminant(Camera::GetInstance()->View());
	XMMATRIX invView = XMMatrixInverse(&detView, Camera::GetInstance()->View());

	XMMATRIX W = XMLoadFloat4x4(&world);
	XMMATRIX T = XMMatrixTranslation(0.0f, Terrain::GetInstance()->GetHeight(pos), 0.0f);
	W = XMMatrixMultiply(W, T);

	XMMATRIX invW = XMMatrixInverse(&XMMatrixDeterminant(W), W);

	XMMATRIX toLocal = XMMatrixMultiply(invView, invW);

	// Decompose the matrix into its individual parts.
	XMVECTOR scale;
	XMVECTOR rotQuat;
	XMVECTOR translation;
	XMMatrixDecompose(&scale, &rotQuat, &translation, toLocal);

	// Transform the camera frustum from view space to the object's local space.
	XNA::Frustum localspaceFrustum;
	XNA::TransformFrustum(&localspaceFrustum, &Camera::GetInstance()->GetCamFrustum(), XMVectorGetX(scale), rotQuat, translation);

	return XNA::IntersectAxisAlignedBoxFrustum(&aabb, &localspaceFrustum);
}

bool CollisionMgr::MonsterFrustumCulling(GameObject* obj)
{
	XMFLOAT3 pos		= obj->GetPos();
	FLOAT scale			= obj->GetScale();
	XMFLOAT4X4 world	= obj->GetWorld();
	XNA::AxisAlignedBox aabb = obj->GetAABB();
	const Camera& cam	= *Camera::GetInstance();

	XMVECTOR detView = XMMatrixDeterminant(cam.View());
	XMMATRIX invView = XMMatrixInverse(&detView, cam.View());

	// 현재 모델은 어떻게 누워있는지 통일되어 있지 않다.
	float max = -MathHelper::Infinity;
	max = MathHelper::Max(max, aabb.Extents.x);
	max = MathHelper::Max(max, aabb.Extents.y);
	max = MathHelper::Max(max, aabb.Extents.z);

	if (max != aabb.Extents.y)
	{
		aabb.Extents.x = aabb.Extents.y;
		aabb.Extents.y = max;
		aabb.Extents.z = aabb.Extents.x;
	}

	XMMATRIX W = XMLoadFloat4x4(&world);
	XMMATRIX T = XMMatrixTranslation(0.0f, Terrain::GetInstance()->GetHeight(pos) + max*scale, 0.0f);
	W = XMMatrixMultiply(W, T);

	XMVECTOR v = XMLoadFloat3(&aabb.Center);
	v = XMVector3TransformCoord(v, W);

	// Decompose the matrix into its individual parts.
	XMVECTOR s;
	XMVECTOR rotQuat;
	XMVECTOR translation;
	XMMatrixDecompose(&s, &rotQuat, &translation, W);

	XNA::OrientedBox ob;
	XMStoreFloat3(&ob.Center, v);
	ob.Extents.x = aabb.Extents.x * scale * 1.2f;
	ob.Extents.y = max * scale  * 1.2f;
	ob.Extents.z = aabb.Extents.z * scale * 1.2f;
	XMStoreFloat4(&ob.Orientation, rotQuat);

	XMMatrixDecompose(&s, &rotQuat, &translation, invView);

	// Transform the camera frustum from view space to the object's local space.
	XNA::Frustum worldspaceFrustum;
	XNA::TransformFrustum(&worldspaceFrustum, &cam.GetCamFrustum(), XMVectorGetX(s), rotQuat, translation);

	// Perform the box/frustum intersection test in local space.
	return XNA::IntersectOrientedBoxFrustum(&ob, &worldspaceFrustum);
}

bool CollisionMgr::CollisionCheckOOBB(GameObject * obj1, GameObject * obj2)
{
	const Camera& cam = *Camera::GetInstance();

	XMFLOAT4X4 world1 = obj1->GetWorld();
	XMFLOAT4X4 world2 = obj2->GetWorld();
	XNA::AxisAlignedBox aabb1 = obj1->GetAABB();
	XNA::AxisAlignedBox aabb2 = obj2->GetAABB();

	XMVECTOR detView = XMMatrixDeterminant(cam.View());
	XMMATRIX invView = XMMatrixInverse(&detView, cam.View());

	XMMATRIX W1 = XMLoadFloat4x4(&world1);
	XMMATRIX W2 = XMLoadFloat4x4(&world2);
	XMMATRIX T1 = XMMatrixTranslation(0.0f, Terrain::GetInstance()->GetHeight(obj1->GetPos()), 0.0f);
	XMMATRIX T2 = XMMatrixTranslation(0.0f, Terrain::GetInstance()->GetHeight(obj2->GetPos()), 0.0f);
	W1 = XMMatrixMultiply(W1, T1);
	W2 = XMMatrixMultiply(W2, T2);

	XMVECTOR v1 = XMLoadFloat3(&aabb1.Center);
	XMVECTOR v2 = XMLoadFloat3(&aabb2.Center);

	v1 = XMVector3TransformCoord(v1, W1);
	v2 = XMVector3TransformCoord(v2, W2);

	// Decompose the matrix into its individual parts.
	XMVECTOR s1, s2;
	XMVECTOR r1, r2;
	XMVECTOR t1, t2;
	XMMatrixDecompose(&s1, &r1, &t1, W1);
	XMMatrixDecompose(&s2, &r2, &t2, W2);

	XNA::OrientedBox ob1, ob2;
	XMStoreFloat3(&ob1.Center, v1);
	XMStoreFloat3(&ob2.Center, v2);

	ob1.Extents.x = aabb1.Extents.x * obj1->GetScale();
	ob1.Extents.y = aabb1.Extents.y * obj1->GetScale();
	ob1.Extents.z = aabb1.Extents.z * obj1->GetScale();
	ob2.Extents.x = aabb2.Extents.x * obj2->GetScale();
	ob2.Extents.y = aabb2.Extents.y * obj2->GetScale();
	ob2.Extents.z = aabb2.Extents.z * obj2->GetScale();

	XMStoreFloat4(&ob1.Orientation, r1);
	XMStoreFloat4(&ob2.Orientation, r2);

	return XNA::IntersectOrientedBoxOrientedBox(&ob1, &ob2);
}

void CollisionMgr::CollisionMoving(XMFLOAT3 & outPos, const XMFLOAT3 & dPos, FLOAT moveSpeed, FLOAT dt)
{
	XMFLOAT3 target;
	XMStoreFloat3(&target, MathHelper::TargetVector2D(dPos, outPos));

	outPos = outPos - target*dt*moveSpeed;
}


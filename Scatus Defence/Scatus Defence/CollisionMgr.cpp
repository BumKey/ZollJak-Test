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

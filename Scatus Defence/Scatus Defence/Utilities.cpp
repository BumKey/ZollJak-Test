#include "Utilities.h"

XMFLOAT4 operator+(XMFLOAT4 l, XMFLOAT4 r) {
	if (l.w != 0.0f && (l.w != r.w))
		int z = 0;
	return XMFLOAT4(l.x + r.x, l.y + r.y, l.z + r.z, r.w);
}

XMFLOAT3 operator-(XMFLOAT3 l, XMFLOAT3 r) { return XMFLOAT3(l.x - r.x, l.y - r.y, l.z - r.z); }
XMFLOAT3 operator+(XMFLOAT3 l, XMFLOAT3 r) { return XMFLOAT3(l.x + r.x, l.y + r.y, l.z + r.z); }
XMFLOAT3 operator/(XMFLOAT3 l, XMFLOAT3 r) { return XMFLOAT3(l.x / r.x, l.y / r.y, l.z / r.z); }
XMFLOAT3 operator*(XMFLOAT3 l, FLOAT r) { return XMFLOAT3(l.x * r, l.y * r, l.z * r); }
XMFLOAT3 operator/(XMFLOAT3 l, FLOAT r) { return XMFLOAT3(l.x / r, l.y / r, l.z / r); }
bool operator==(XMFLOAT3 l, XMFLOAT3 r) {
	if (abs(l.x - r.x) <= 0.001f &&
		abs(l.y - r.y) <= 0.001f &&
		abs(l.z - r.z) <= 0.001f)
		return true;
	else
		return false;
}

XMFLOAT2 operator+(XMFLOAT2 l, XMFLOAT2 r) { return XMFLOAT2(l.x + r.x, l.y + r.y); }
XMFLOAT2 operator-(XMFLOAT2 l, XMFLOAT2 r) { return XMFLOAT2(l.x - r.x, l.y - r.y); }
bool operator==(XMFLOAT2 l, XMFLOAT2 r) {
	if (abs(l.x - r.x) <= 0.001f &&
		abs(l.y - r.y) <= 0.001f)
		return true;
	else
		return false;
}
bool operator!=(XMFLOAT2 l, XMFLOAT2 r) {
	if (abs(l.x - r.x) <= 0.001f &&
		abs(l.y - r.y) <= 0.001f)
		return false;
	else
		return true;
}

XMFLOAT4 Float4Normalize(const XMFLOAT4& in)
{
	XMFLOAT3 in4to3(in.x, in.y, in.z);
	XMFLOAT3 out;

	XMVECTOR vIn = XMLoadFloat3(&in4to3);
	XMStoreFloat3(&out, XMVector4Normalize(vIn));
	return XMFLOAT4(out.x, out.y, out.z, in.w);
}

XMFLOAT3 Float3Normalize(const XMFLOAT3& in)
{
	XMFLOAT3 out;

	XMVECTOR vIn = XMLoadFloat3(&in);
	XMStoreFloat3(&out, XMVector3Normalize(vIn));
	return out;
}

XMFLOAT2 Float2Normalize(const XMFLOAT2& in)
{
	XMFLOAT2 out;

	XMVECTOR vIn = XMLoadFloat2(&in);
	XMStoreFloat2(&out, XMVector3Normalize(vIn));
	return out;
}
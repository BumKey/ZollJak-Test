#ifndef MeshData_H
#define MeshData_H

#include "d3dUtil.h"

class MeshData
{
public:
	struct Subset
	{
		Subset() : Id(-1), VertexStart(0), VertexCount(0),
			FaceStart(0), FaceCount(0) { }

		UINT Id;
		UINT VertexStart;
		UINT VertexCount;
		UINT FaceStart;
		UINT FaceCount;
	};

public:
    MeshData();
	~MeshData();

	template <typename VertexType>
	void SetVertices(ID3D11Device* device, const VertexType* vertices, UINT count);

	void SetIndices(ID3D11Device* device, const USHORT* indices, UINT count);

	void SetSubsetTable(std::vector<Subset>& subsetTable);

	void Draw(ID3D11DeviceContext* dc, UINT subsetId);

private:
	MeshData(const MeshData& rhs);
	MeshData& operator=(const MeshData& rhs);

private:
	ID3D11Buffer* mVB;
	ID3D11Buffer* mIB;

	DXGI_FORMAT mIndexBufferFormat; // Always 16-bit
	UINT mVertexStride;

	std::vector<Subset> mSubsetTable;
};

template <typename VertexType>
void MeshData::SetVertices(ID3D11Device* device, const VertexType* vertices, UINT count)
{
	ReleaseCOM(mVB);

	mVertexStride = sizeof(VertexType);

	D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(VertexType) * count;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = vertices;

    HR(device->CreateBuffer(&vbd, &vinitData, &mVB));
}

#endif // MeshData_H
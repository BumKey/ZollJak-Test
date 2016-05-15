#pragma once
#include "BasicMesh.h"
#include "SkinnedMesh.h"
#include "TextureMgr.h"

// �ϳ��� �����ϰ� ���� ������ �����ؾ��ϴ� ���ҽ�����
// �����ϰ� ������ �� �ֵ��� �Ѵ�.
class ResourceMgr
{
public:
	ResourceMgr();
	~ResourceMgr();

	void Init(ID3D11Device* dc);

	// Obstacles
	BasicMesh* TreeMesh;
	BasicMesh* BaseMesh;
	BasicMesh* StairsMesh;
	BasicMesh* Pillar1Mesh;
	BasicMesh* Pillar2Mesh;
	BasicMesh* Pillar3Mesh;
	BasicMesh* Pillar4Mesh;
	BasicMesh* RockMesh;
	BasicMesh* Temple;

	SkinnedMesh* GetGoblinMesh();
	static ID3D11ShaderResourceView* GoblinDiffuseMapSRV[2];
	void ReleaseGoblinMesh();

private:
	ID3D11Device* mDevice;
	TextureMgr mTexMgr;

	UINT mGoblinReferences;

	// ���� Ƚ���� �ǹ��ִ� �޽���
	SkinnedMesh* mGoblinMesh;
};


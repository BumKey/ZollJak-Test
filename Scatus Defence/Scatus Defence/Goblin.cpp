#include "Goblin.h"

Goblin::Goblin(SkinnedMesh* mesh, const InstanceDesc& info, GoblinType type)
	: SkinnedObject(mesh, info), mType(type)
{
	mTimePos = 0.0f;
	mMovingSpeed = 5.5f;
	mCurrClipName = "stand";
}

Goblin::~Goblin()
{
}

void Goblin::DrawToScene(ID3D11DeviceContext * dc, const Camera & cam, XMFLOAT4X4 shadowTransform, FLOAT tHeight)
{
	XMMATRIX view = cam.View();
	XMMATRIX proj = cam.Proj();
	XMMATRIX viewProj = cam.ViewProj();

	ID3DX11EffectTechnique* tech = Effects::NormalMapFX->Light3TexTech;
	ID3DX11EffectTechnique* alphaClippedTech = Effects::NormalMapFX->Light3TexAlphaClipTech;

	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	XMMATRIX world;
	XMMATRIX worldInvTranspose;
	XMMATRIX worldViewProj;

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	XMMATRIX toTexSpace(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX st = XMLoadFloat4x4(&shadowTransform);

	if (GetAsyncKeyState('1') & 0x8000)
		dc->RSSetState(RenderStates::WireframeRS);

	ID3DX11EffectTechnique* activeSkinnedTech = Effects::NormalMapFX->Light3TexSkinnedTech;
	dc->IASetInputLayout(InputLayouts::PosNormalTexTanSkinned);

	// Draw the animated characters.
	D3DX11_TECHNIQUE_DESC techDesc;
	activeSkinnedTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		world = XMLoadFloat4x4(&mWorld);
		world = XMMatrixMultiply(world, XMMatrixTranslation(0.0f, tHeight, 0.0f));
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world*view*proj;

		Effects::NormalMapFX->SetWorld(world);
		Effects::NormalMapFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::NormalMapFX->SetWorldViewProj(worldViewProj);
		Effects::NormalMapFX->SetWorldViewProjTex(worldViewProj*toTexSpace);
		Effects::NormalMapFX->SetShadowTransform(world*st);
		Effects::NormalMapFX->SetTexTransform(XMMatrixScaling(1.0f, 1.0f, 1.0f));
		Effects::NormalMapFX->SetBoneTransforms(
			&mFinalTransforms[0],
			mFinalTransforms.size());

		Effects::NormalMapFX->SetMaterial(mMesh->Mat[0]);
		//Effects::NormalMapFX->SetNormalMap(mMesh->NormalMapSRV[subset]);

		switch (mType)
		{
		case GoblinType::Red:
			Effects::NormalMapFX->SetDiffuseMap(ResourceMgr::GoblinDiffuseMapSRV[0]);
			break;
		case GoblinType::Green:
			Effects::NormalMapFX->SetDiffuseMap(mMesh->DiffuseMapSRV[0]);
			break;
		case GoblinType::Blue:
			Effects::NormalMapFX->SetDiffuseMap(ResourceMgr::GoblinDiffuseMapSRV[1]);
			break;
		}
	
		activeSkinnedTech->GetPassByIndex(p)->Apply(0, dc);
		mMesh->MeshData.Draw(dc, 0);	
	}
}

void Goblin::Release(ResourceMgr & rMgr)
{
	rMgr.ReleaseGoblinMesh();
}

bool Goblin::SetClip(std::string clipName)
{
	if (clipName == "attack01")
		mTimePos = 0.0f;

	// 공격 애니메이션 아직 안 끝났으면 바꾸지X
	if (mCurrClipName == "attack01" && mTimePos < mMesh->SkinnedData.GetClipEndTime(mCurrClipName))
		return false;
	else
		mCurrClipName = clipName;
}

void Goblin::Animate(float dt)
{
	mTimePos += dt;

	mMesh->SkinnedData.GetFinalTransforms(mCurrClipName, mTimePos, mFinalTransforms);

	// Loop animation
	if (mTimePos > mMesh->SkinnedData.GetClipEndTime(mCurrClipName))
	{
		mTimePos = 0.0f;
		if (mCurrClipName == "attack01")		// attack01은 루프 안쓰는 애니메이션
			mCurrClipName = "stand";
	}
}

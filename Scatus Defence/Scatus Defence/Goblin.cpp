#include "Goblin.h"

Goblin::Goblin(SkinnedMesh* mesh, const InstanceDesc& info, Type type)
	: Monster(mesh, info), mGoblinType(type)
{
	mTimePos = 0.0f;
	mProperty.movespeed = MathHelper::RandF(0.07f, 0.2f);
	mProperty.attackspeed = MathHelper::RandF(0.6f, 1.1f);
	mProperty.attakpoint = 0.5f;

	mObjectType = Object_type::goblin;

	mAnimNames[Anims::attack1] = "attack01";
	mAnimNames[Anims::attack2] = "attack02";
	mAnimNames[Anims::hit] = "damage";
	mAnimNames[Anims::dead] = "dead";
	mAnimNames[Anims::drop_down] = "drop_down";
	mAnimNames[Anims::run] = "run";
	mAnimNames[Anims::sit_up] = "sit_up";
	mAnimNames[Anims::idle] = "stand";
	mAnimNames[Anims::walk] = "walk";
	mAnimNames[Anims::look_around] = "stand2";

	SetState(type_idle);
}

Goblin::~Goblin()
{
}

std::string Goblin::GetAnimName(const Anims& eAnim)
{
	return mAnimNames[eAnim];
}

void Goblin::DrawToScene(ID3D11DeviceContext* dc, const Camera& cam, const XMFLOAT4X4& shadowTransform, const FLOAT& tHeight)
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

		switch (mGoblinType)
		{
		case Type::Red:
			Effects::NormalMapFX->SetDiffuseMap(ResourceMgr::GoblinDiffuseMapSRV[0]);
			break;
		case Type::Green:
			Effects::NormalMapFX->SetDiffuseMap(mMesh->DiffuseMapSRV[0]);
			break;
		case Type::Blue:
			Effects::NormalMapFX->SetDiffuseMap(ResourceMgr::GoblinDiffuseMapSRV[1]);
			break;
		}
	
		activeSkinnedTech->GetPassByIndex(p)->Apply(0, dc);
		mMesh->MeshData.Draw(dc, 0);	
	}
}
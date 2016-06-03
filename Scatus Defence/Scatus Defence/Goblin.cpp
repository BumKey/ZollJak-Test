#include "Goblin.h"

Goblin::Goblin(SkinnedMesh* mesh, const InstanceDesc& info, Type type)
	: Monster(mesh, info), mGoblinType(type)
{
	mTimePos = 0.0f;
	mProperty.movespeed = MathHelper::RandF(0.07f, 0.2f);
	mProperty.attackspeed = MathHelper::RandF(0.6f, 1.1f);
	mProperty.attakpoint = 0.5f;

	mObjectType = Object_type::goblin;

	mAnimNames[Anim::attack1] = "attack01";
	mAnimNames[Anim::attack2] = "attack02";
	mAnimNames[Anim::damage] = "damage";
	mAnimNames[Anim::dead] = "dead";
	mAnimNames[Anim::drop_down] = "drop_down";
	mAnimNames[Anim::run] = "run";
	mAnimNames[Anim::sit_up] = "sit_up";
	mAnimNames[Anim::stand] = "stand";
	mAnimNames[Anim::walk] = "walk";
	mAnimNames[Anim::look_around] = "stand2";

	SetState(type_idle);
}

Goblin::~Goblin()
{
}

std::string Goblin::GetAnimName(Anim eAnim)
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


void Goblin::SetClip()
{
	if (mProperty.state == state_type::type_idle)
		mCurrClipName = mAnimNames[Anim::stand];

	if (mProperty.state == state_type::type_attack &&
		mCurrClipName != mAnimNames[Anim::attack1] &&
		mCurrClipName != mAnimNames[Anim::attack2]) {
		if(rand()%2)
			mCurrClipName = mAnimNames[Anim::attack1];
		else
			mCurrClipName = mAnimNames[Anim::attack2];
	}

	//if (mProperty.state == state_type::type_battle)
	//	mCurrClipName = mAnimNames[Anim::attack2];

	if (mProperty.state == state_type::type_run)
		mCurrClipName = mAnimNames[Anim::run];

	if (mProperty.state == state_type::type_walk)
		mCurrClipName = mAnimNames[Anim::walk];

	if (mProperty.state == state_type::type_die)
		mCurrClipName = mAnimNames[Anim::dead];

	//// 공격 애니메이션 아직 안 끝났으면 바꾸지X
	//if ((mCurrClipName == mAnimNames[Anim::attack1] ||
	//	mCurrClipName == mAnimNames[Anim::attack2]) &&
	//	mTimePos < mMesh->SkinnedData.GetClipEndTime(mCurrClipName))
	//	return false;
	//else
	//	mCurrClipName = clipName;
}

void Goblin::Animate(float dt)
{
	SetClip();

	if (mProperty.state == state_type::type_attack)
		mTimePos += dt*mProperty.attackspeed;
	else 
		mTimePos += dt*mProperty.movespeed*10.0f;

	mMesh->SkinnedData.GetFinalTransforms(mCurrClipName, mTimePos, mFinalTransforms);

	// Loop animation
	if (mTimePos > mMesh->SkinnedData.GetClipEndTime(mCurrClipName))
	{
		mTimePos = 0.0f;
		if (mCurrClipName == mAnimNames[Anim::attack1] ||
			mCurrClipName == mAnimNames[Anim::attack2])		// attack01은 루프 안쓰는 애니메이션
			mProperty.state = state_type::type_idle;
	}
}

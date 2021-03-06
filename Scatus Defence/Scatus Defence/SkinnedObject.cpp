#include "SkinnedObject.h"
#include "SceneMgr.h"

SkinnedObject::SkinnedObject() : GameObject(), mTimePos(0.0f)
{
}

SkinnedObject::SkinnedObject(SkinnedMesh* mesh, const SO_InitDesc& info) : GameObject(mesh, info),
mTimePos(0.0f)
{
	mMesh = mesh;
	mFinalTransforms.resize(mesh->SkinnedData.BoneCount());
}

SkinnedObject::~SkinnedObject()
{
}

void SkinnedObject::Init(SkinnedMesh * mesh, const SO_InitDesc & info)
{
	mMesh = mesh;

	XMMATRIX S = XMMatrixScaling(info.Scale, info.Scale, info.Scale);
	XMMATRIX R = XMMatrixRotationRollPitchYaw(info.Rot.x, info.Rot.y, info.Rot.z);
	XMMATRIX T = XMMatrixTranslation(info.Pos.x, info.Pos.y, info.Pos.z);

	XMStoreFloat4x4(&mWorld, S*R*T);

	mScaling = info.Scale;
	mRotation = info.Rot;
	mPosition = info.Pos;

	R = XMMatrixRotationY(info.Rot.y);
	XMStoreFloat3(&mRight, XMVector3TransformNormal(XMLoadFloat3(&mRight), R));
	XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
	XMStoreFloat3(&mCurrLook, XMVector3TransformNormal(XMLoadFloat3(&mCurrLook), R));

	InitBoundingObject();

	mFinalTransforms.resize(mesh->SkinnedData.BoneCount());
}

void SkinnedObject::Update(float dt)
{
	XMVECTOR vR = XMLoadFloat3(&mRight);
	XMVECTOR vU = XMLoadFloat3(&mUp);
	XMVECTOR vCL = XMLoadFloat3(&mCurrLook);
	XMVECTOR vOL = XMLoadFloat3(&mPrevLook);
	XMVECTOR vP = XMLoadFloat3(&mPosition);

	vCL = XMVector3Normalize(vCL);
	vOL = XMVector3Normalize(vOL);
	vU = XMVector3Normalize(XMVector3Cross(vCL, vR));
	vR = XMVector3Cross(vU, vCL);

	XMStoreFloat3(&mRight, vR);
	XMStoreFloat3(&mUp, vU);
	XMStoreFloat3(&mCurrLook, vCL);
	XMStoreFloat3(&mPrevLook, vOL);

	if (mRotation.y >= MathHelper::Pi*2.0f)
		mRotation.y = 0.0f;

	XMMATRIX S = XMMatrixScaling(mScaling, mScaling, mScaling);
	XMMATRIX R = XMMatrixRotationRollPitchYaw(mRotation.x, mRotation.y, mRotation.z);
	XMVECTOR Q = XMQuaternionRotationMatrix(R);
	XMMATRIX T = XMMatrixTranslation(mPosition.x, mPosition.y, mPosition.z);

	XMStoreFloat4x4(&mWorld, S*R*T);

	UpdateBoundingObject();
}

void SkinnedObject::Animate(float dt)
{
	assert(mAnimNames.size() > 0);

	SetClip();

	if (mActionState == ActionState::Attack)
		mTimePos += dt*mProperty.attackspeed;
	else
		mTimePos += dt*mProperty.movespeed/5.0f;

	static_cast<SkinnedMesh*>(mMesh)->SkinnedData.GetFinalTransforms(mCurrClipName, mTimePos, mFinalTransforms);

	// Loop animation
	if (CurrAnimEnd())
	{
		if(mActionState != ActionState::Die)
			mTimePos = 0.0f;

		if (mCurrClipName == mAnimNames[Anims::attack1] ||
			mCurrClipName == mAnimNames[Anims::attack2] ||
			mCurrClipName == mAnimNames[Anims::hit]) // attack01은 루프 안쓰는 애니메이션
			ChangeActionState(ActionState::Idle);
	}
}

void SkinnedObject::DrawToScene(ID3D11DeviceContext * dc, const XMFLOAT4X4& shadowTransform)
{
	const Camera& cam = *Camera::GetInstance();

	XMMATRIX view = cam.View();
	XMMATRIX proj = cam.Proj();
	XMMATRIX viewProj = cam.ViewProj();

	ID3DX11EffectTechnique* tech = Effects::NormalMapFX->Light3TexTech;
	ID3DX11EffectTechnique* alphaClippedTech = Effects::NormalMapFX->Light3TexAlphaClipTech;

	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	if (GetAsyncKeyState('1') & 0x8000)
		dc->RSSetState(RenderStates::WireframeRS);

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

	float tHeight = Terrain::GetInstance()->GetHeight(mPosition);

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

		for (UINT subset = 0; subset < mMesh->SubsetCount; ++subset)
		{
			Effects::NormalMapFX->SetMaterial(mMesh->Mat[subset]);
			Effects::NormalMapFX->SetDiffuseMap(mMesh->DiffuseMapSRV[subset]);
			Effects::NormalMapFX->SetNormalMap(mMesh->NormalMapSRV[subset]);

			activeSkinnedTech->GetPassByIndex(p)->Apply(0, dc);
			mMesh->MeshData.Draw(dc, subset);
		}
	}
}

void SkinnedObject::DrawToSsaoNormalDepthMap(ID3D11DeviceContext * dc)
{
	const Camera& cam = *Camera::GetInstance();

	XMMATRIX view = cam.View();
	XMMATRIX proj = cam.Proj();
	XMMATRIX viewProj = XMMatrixMultiply(view, proj);

	ID3DX11EffectTechnique* animatedTech = Effects::SsaoNormalDepthFX->NormalDepthSkinnedTech;

	XMMATRIX world;
	XMMATRIX worldInvTranspose;
	XMMATRIX worldView;
	XMMATRIX worldInvTransposeView;
	XMMATRIX worldViewProj;

	dc->IASetInputLayout(InputLayouts::PosNormalTexTanSkinned);

	if (GetAsyncKeyState('1') & 0x8000)
		dc->RSSetState(RenderStates::WireframeRS);

	float tHeight = Terrain::GetInstance()->GetHeight(mPosition);

	D3DX11_TECHNIQUE_DESC techDesc;
	animatedTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		world = XMLoadFloat4x4(&mWorld);
		world = XMMatrixMultiply(world, XMMatrixTranslation(0.0f, tHeight, 0.0f));
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldView = world*view;
		worldInvTransposeView = worldInvTranspose*view;
		worldViewProj = world*view*proj;

		Effects::SsaoNormalDepthFX->SetWorldView(worldView);
		Effects::SsaoNormalDepthFX->SetWorldInvTransposeView(worldInvTransposeView);
		Effects::SsaoNormalDepthFX->SetWorldViewProj(worldViewProj);
		Effects::SsaoNormalDepthFX->SetTexTransform(XMMatrixIdentity());
		Effects::SsaoNormalDepthFX->SetBoneTransforms(
			&mFinalTransforms[0],
			mFinalTransforms.size());

		animatedTech->GetPassByIndex(p)->Apply(0, dc);

		for (UINT subset = 0; subset < mMesh->SubsetCount; ++subset)
		{
			mMesh->MeshData.Draw(dc, subset);
		}
	}
}

void SkinnedObject::SetClip()
{
	if (mActionState == ActionState::Idle)
		mCurrClipName = mAnimNames[Anims::idle];

	if (mActionState == ActionState::Attack &&
		mCurrClipName != mAnimNames[Anims::attack1] &&
		mCurrClipName != mAnimNames[Anims::attack2]) {
		if (rand() % 2)
			mCurrClipName = mAnimNames[Anims::attack1];
		else
			mCurrClipName = mAnimNames[Anims::attack2];
	}

	if (mActionState == ActionState::Run)
		mCurrClipName = mAnimNames[Anims::run];

	if (mActionState == ActionState::Walk)
		mCurrClipName = mAnimNames[Anims::walk];

	if (mActionState == ActionState::Die)
		mCurrClipName = mAnimNames[Anims::dead];

	if (mActionState == ActionState::Damage)
		mCurrClipName = mAnimNames[Anims::hit];
}

bool SkinnedObject::CurrAnimEnd() 
{
	if (mTimePos > static_cast<SkinnedMesh*>(mMesh)->SkinnedData.GetClipEndTime(mCurrClipName))
		return true;

	return false;
}

std::string SkinnedObject::GetAnimName(Anims & eAnim)
{
	return mAnimNames[eAnim]; 
}


void SkinnedObject::DrawToShadowMap(ID3D11DeviceContext * dc, const XMMATRIX & lightViewProj)
{
	const Camera& cam = *Camera::GetInstance();

	Effects::BuildShadowMapFX->SetEyePosW(cam.GetPosition());
	Effects::BuildShadowMapFX->SetViewProj(lightViewProj);

	// These properties could be set per object if needed.
	Effects::BuildShadowMapFX->SetHeightScale(0.07f);
	Effects::BuildShadowMapFX->SetMaxTessDistance(1.0f);
	Effects::BuildShadowMapFX->SetMinTessDistance(25.0f);
	Effects::BuildShadowMapFX->SetMinTessFactor(1.0f);
	Effects::BuildShadowMapFX->SetMaxTessFactor(5.0f);

	ID3DX11EffectTechnique* animatedSmapTech = Effects::BuildShadowMapFX->BuildShadowMapSkinnedTech;

	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	if (GetAsyncKeyState('1') & 0x8000)
		dc->RSSetState(RenderStates::WireframeRS);

	XMMATRIX world;
	XMMATRIX worldInvTranspose;
	XMMATRIX worldViewProj;

	dc->IASetInputLayout(InputLayouts::PosNormalTexTanSkinned);

	float tHeight = Terrain::GetInstance()->GetHeight(mPosition);

	D3DX11_TECHNIQUE_DESC techDesc;
	animatedSmapTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		world = XMLoadFloat4x4(&mWorld);
		world = XMMatrixMultiply(world, XMMatrixTranslation(0.0f, tHeight, 0.0f));
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world*lightViewProj;

		Effects::BuildShadowMapFX->SetWorld(world);
		Effects::BuildShadowMapFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BuildShadowMapFX->SetWorldViewProj(worldViewProj);
		Effects::BuildShadowMapFX->SetTexTransform(XMMatrixIdentity());
		Effects::BuildShadowMapFX->SetBoneTransforms(
			&mFinalTransforms[0],
			mFinalTransforms.size());


		animatedSmapTech->GetPassByIndex(p)->Apply(0, dc);

		for (UINT subset = 0; subset < mMesh->SubsetCount; ++subset)
		{
			mMesh->MeshData.Draw(dc, subset);
		}
	}
}

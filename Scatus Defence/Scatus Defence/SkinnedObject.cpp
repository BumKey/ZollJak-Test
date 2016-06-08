#include "SkinnedObject.h"

SkinnedObject::SkinnedObject(SkinnedMesh* mesh, const InstanceDesc& info) : GameObject(mesh, info), mMesh(mesh)
{
	mTimePos = 0.0f;
	mFinalTransforms.resize(mMesh->SkinnedData.BoneCount());
}

SkinnedObject::~SkinnedObject()
{
}

void SkinnedObject::Walk(float d)
{
	mDirection = mCurrLook;

	// mPosition += d*mLook
	XMVECTOR s = XMVectorReplicate(d*mProperty.movespeed);
	XMVECTOR l = XMLoadFloat3(&mCurrLook);
	XMVECTOR p = XMLoadFloat3(&mPosition);
	XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, l, p));

	mActionState = ActionState::Run;
}

void SkinnedObject::Strafe(float d)
{
	mDirection = mRight;

	// mPosition += d*mRight
	XMVECTOR s = XMVectorReplicate(d*mProperty.movespeed);
	XMVECTOR r = XMLoadFloat3(&mRight);
	XMVECTOR p = XMLoadFloat3(&mPosition);
	XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, r, p));

	mActionState = ActionState::Run;
}

void SkinnedObject::RotateY(float angle)
{
	mRotation.y += angle;
	mPrevLook = mCurrLook;
	// Rotate the basis vectors about the world y-axis.
	XMMATRIX R = XMMatrixRotationY(angle);

	XMStoreFloat3(&mRight, XMVector3TransformNormal(XMLoadFloat3(&mRight), R));
	XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
	XMStoreFloat3(&mCurrLook, XMVector3TransformNormal(XMLoadFloat3(&mCurrLook), R));
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

	XMFLOAT3 center = mMesh->GetAABB().Center;
	XMFLOAT3 extent = mMesh->GetAABB().Extents * mScaling;

	XMVECTOR vCenter = XMLoadFloat3(&center);

	vCenter = XMVector3TransformCoord(vCenter, S*R*T);
	XMStoreFloat3(&center, vCenter);
	mOOBB.Center = center;
	mOOBB.Extents = extent;
	XMStoreFloat4(&mOOBB.Orientation, Q);
}

void SkinnedObject::Animate(float dt)
{
	assert(mAnimNames.size() > 0);

	SetClip();

	if (mActionState == ActionState::Attack)
		mTimePos += dt*mProperty.attackspeed;
	else
		mTimePos += dt*mProperty.movespeed/5.0f;

	mMesh->SkinnedData.GetFinalTransforms(mCurrClipName, mTimePos, mFinalTransforms);

	// Loop animation
	if (mTimePos > mMesh->SkinnedData.GetClipEndTime(mCurrClipName))
	{
		mTimePos = 0.0f;
		if (mCurrClipName == mAnimNames[Anims::attack1] ||
			mCurrClipName == mAnimNames[Anims::attack2])		// attack01은 루프 안쓰는 애니메이션
			mActionState = ActionState::Idle;
	}
}

void SkinnedObject::MovingCollision(XMFLOAT3 intersectedObject, float dt)
{
	mCollisionState = CollisionState::MovingCollision;
	XMVECTOR vTarget = MathHelper::TargetVector2D(intersectedObject, mPosition);

	XMVECTOR s = XMVectorReplicate(dt*mProperty.movespeed);
	XMVECTOR p = XMLoadFloat3(&mPosition);

	// 부딪힌 오브젝트부터 멀어지는 방향
	XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, -vTarget, p));

	// 차별을 두기 위해 일단 런으로
	mActionState = ActionState::Run;
}

void SkinnedObject::DrawToScene(ID3D11DeviceContext * dc, const Camera & cam, const XMFLOAT4X4& shadowTransform, const FLOAT& tHeight)
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

void SkinnedObject::DrawToSsaoNormalDepthMap(ID3D11DeviceContext * dc, const Camera & cam, const FLOAT& tHeight)
{
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

void SkinnedObject::Release(ResourceMgr & rMgr)
{
	rMgr.ReleaseMesh(mObjectType);
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
}

bool SkinnedObject::CurrAnimEnd() 
{
	if (abs(mTimePos - mMesh->SkinnedData.GetClipEndTime(mCurrClipName)) <= 0.1f)
		return true;

	return false;
}

std::string SkinnedObject::GetAnimName(Anims & eAnim)
{
	return mAnimNames[eAnim]; 
}


void SkinnedObject::DrawToShadowMap(ID3D11DeviceContext * dc, const Camera & cam, const XMMATRIX & lightViewProj, const FLOAT& tHeight)
{
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

	XMMATRIX world;
	XMMATRIX worldInvTranspose;
	XMMATRIX worldViewProj;

	dc->IASetInputLayout(InputLayouts::PosNormalTexTanSkinned);

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

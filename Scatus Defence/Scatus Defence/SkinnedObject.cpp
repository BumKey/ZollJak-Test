#include "SkinnedObject.h"

SkinnedObject::SkinnedObject(SkinnedMesh* mesh, const InstanceDesc& info) : GameObject(mesh), mMesh(mesh)
{
	XMMATRIX S = XMMatrixScaling(info.Scale, info.Scale, info.Scale);
	XMMATRIX R = XMMatrixRotationRollPitchYaw(0.0f, info.Yaw, 0.0f);
	XMMATRIX T = XMMatrixTranslation(info.Pos.x, info.Pos.y, info.Pos.z);

	XMStoreFloat4x4(&mWorld, S*R*T);

	mScaling = info.Scale;
	mRotation = XMFLOAT3(0.0f, info.Yaw, 0.0f);
	mPosition = info.Pos;

	R = XMMatrixRotationY(info.Yaw);
	XMStoreFloat3(&mRight, XMVector3TransformNormal(XMLoadFloat3(&mRight), R));
	XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
	XMStoreFloat3(&mCurrLook, XMVector3TransformNormal(XMLoadFloat3(&mCurrLook), R));

	mFinalTransforms.resize(mMesh->SkinnedData.BoneCount());
}

SkinnedObject::~SkinnedObject()
{
}

void SkinnedObject::Walk(float d)
{
	// mPosition += d*mLook
	XMVECTOR s = XMVectorReplicate(d*mMovingSpeed);
	XMVECTOR l = XMLoadFloat3(&mCurrLook);
	XMVECTOR p = XMLoadFloat3(&mPosition);
	XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, l, p));

	mProperty.state = state_type::type_run;
}

void SkinnedObject::Strafe(float d)
{
	// mPosition += d*mRight
	XMVECTOR s = XMVectorReplicate(d*mMovingSpeed);
	XMVECTOR r = XMLoadFloat3(&mRight);
	XMVECTOR p = XMLoadFloat3(&mPosition);
	XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, r, p));

	mProperty.state = state_type::type_run;
}

void SkinnedObject::MoveTo(Vector2D targetPos, float dt)
{
	XMFLOAT3 fTargetPos;
	fTargetPos.x = targetPos.x - mPosition.x;
	fTargetPos.y = 0;
	fTargetPos.z = targetPos.y - mPosition.z;

	XMVECTOR vPlayer = XMLoadFloat3(&mPosition);
	XMVECTOR vTarget = XMLoadFloat3(&fTargetPos);
	XMVector3Normalize(vPlayer);
	XMVector3Normalize(vTarget);

	XMVECTOR s = XMVectorReplicate(dt*mMovingSpeed);
	XMVECTOR p = XMLoadFloat3(&mPosition);

	// 방향으로 이동
	XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, vTarget, p));

	// 방향으로 회전
	XMVECTOR vAngle = XMVector3AngleBetweenVectors(vPlayer, vTarget);
	mRotation.y = XMVectorGetY(vAngle);

	mProperty.state = state_type::type_walk;
}



void SkinnedObject::RotateY(float angle)
{
	mRotation.y += angle;

	// Rotate the basis vectors about the world y-axis.
	XMMATRIX R = XMMatrixRotationY(angle);

	XMStoreFloat3(&mRight, XMVector3TransformNormal(XMLoadFloat3(&mRight), R));
	XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
	XMStoreFloat3(&mCurrLook, XMVector3TransformNormal(XMLoadFloat3(&mCurrLook), R));
}

void SkinnedObject::Attack(GameObject* target)
{
	if (target != NULL)
	{
		SetTarget(target);
		mProperty.state = type_attack;

		if (target->GetState() != type_die)
		{
			int target_hp = target->GetProperty().hp_now;
			int armor = target->GetProperty().guardpoint;
			int damage = target->GetProperty().attakpoint;

			target->SetHP(target_hp + (damage*(1 - (armor*0.06)) / (1 + 0.06*armor)));

			printf("공격을 성공했습니다. 상대의 체력 : %d \n", target->GetProperty().hp_now);

			if (target->GetProperty().hp_now <= -500)
			{
				printf("사망자 이름 : %s\n", target->GetProperty().name);
			}
			if (target->GetProperty().hp_now <= 0)
			{
				target->SetState(type_die);
				printf("타겟 사망");
				this->SetState(type_idle);
			}
		}
	}
	else
	{
		printf("타겟이 지정되지 않았습니다.");

	}
}

void SkinnedObject::Update()
{
	XMVECTOR vR = XMLoadFloat3(&mRight);
	XMVECTOR vU = XMLoadFloat3(&mUp);
	XMVECTOR vCL = XMLoadFloat3(&mCurrLook);
	XMVECTOR vOL = XMLoadFloat3(&mOriginLook);
	XMVECTOR vP = XMLoadFloat3(&mPosition);

	vCL = XMVector3Normalize(vCL);
	vOL = XMVector3Normalize(vOL);
	vU = XMVector3Normalize(XMVector3Cross(vCL, vR));
	vR = XMVector3Cross(vU, vCL);

	XMStoreFloat3(&mRight, vR);
	XMStoreFloat3(&mUp, vU);
	XMStoreFloat3(&mCurrLook, vCL);
	XMStoreFloat3(&mOriginLook, vOL);
	
	//XMVECTOR Q0 = XMLoadFloat3(&XMFLOAT3(0.0f, 0.0f, -1.0f));
	//XMVECTOR Q1 = XMLoadFloat3(&mCurrLook);
	//XMVECTOR Q = XMQuaternionSlerp(Q0, Q1, 0.5f);

	/*XMVECTOR vAngle = XMVector3AngleBetweenVectors(vOL, vCL);
	XMVECTOR vDot = XMVector3Dot(vOL, vCL);
	float fAngle = XMVectorGetZ(vAngle);
	float fdAngle = XMConvertToDegrees(fAngle);
	if (XMVectorGetX(vDot) < 0)
		fAngle = MathHelper::Pi - fAngle;*/

	if (mRotation.y >= MathHelper::Pi*2.0f)
		mRotation.y = 0.0f;

	XMMATRIX S = XMMatrixScaling(mScaling, mScaling, mScaling);
	//XMMATRIX R = XMMatrixRotationQuaternion(Q);
	XMMATRIX R = XMMatrixRotationY(mRotation.y);
	XMMATRIX T = XMMatrixTranslation(mPosition.x, mPosition.y, mPosition.z);

	XMStoreFloat4x4(&mWorld, S*R*T);
}

void SkinnedObject::DrawToScene(ID3D11DeviceContext * dc, const Camera & cam, XMFLOAT4X4 shadowTransform, FLOAT tHeight)
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

void SkinnedObject::DrawToSsaoNormalDepthMap(ID3D11DeviceContext * dc, const Camera & cam, FLOAT tHeight)
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

bool SkinnedObject::AnimEnd(std::string clipName)
{
	if (abs(mTimePos - mMesh->SkinnedData.GetClipEndTime(clipName)) <= 0.1f)
		return true;

	return false;
}

void SkinnedObject::DrawToShadowMap(ID3D11DeviceContext * dc, const Camera & cam, const XMMATRIX & lightViewProj, FLOAT tHeight)
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

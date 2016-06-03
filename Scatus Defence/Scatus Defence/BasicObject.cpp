#include "BasicObject.h"

BasicObject::BasicObject(BasicMesh * mesh, const InstanceDesc& info, Label label) : GameObject(mesh), mLabel(label)
{
	XMMATRIX S = XMMatrixScaling(info.Scale, info.Scale, info.Scale);
	XMMATRIX R = XMMatrixRotationRollPitchYaw(info.Rot.x, info.Rot.y, info.Rot.z);
	XMMATRIX T = XMMatrixTranslation(info.Pos.x, info.Pos.y, info.Pos.z);

	XMStoreFloat4x4(&mWorld, S*R*T);

	mScaling = info.Scale;
	mRotation = XMFLOAT3(0.0f, info.Rot.y, 0.0f);
	mPosition = info.Pos;

	R = XMMatrixRotationY(info.Rot.y);
	XMStoreFloat3(&mRight, XMVector3TransformNormal(XMLoadFloat3(&mRight), R));
	XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
	XMStoreFloat3(&mCurrLook, XMVector3TransformNormal(XMLoadFloat3(&mCurrLook), R));
}

// 후에 스마트포인터 사용해서 메모리 효율적 관리하도록
BasicObject::~BasicObject()
{
}

void BasicObject::Walk(float d)
{
}

void BasicObject::Strafe(float d)
{
}

void BasicObject::MoveTo(Vector2D direction, float dt)
{
}

void BasicObject::RotateY(float angle)
{
}

void BasicObject::Update(float dt)
{
}

void BasicObject::DrawToScene(ID3D11DeviceContext* dc, const Camera& cam, const XMFLOAT4X4& shadowTransform, const FLOAT& tHeight)
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

	dc->IASetInputLayout(InputLayouts::PosNormalTexTan);

	if (GetAsyncKeyState('1') & 0x8000)
		dc->RSSetState(RenderStates::WireframeRS);

	world = XMLoadFloat4x4(&mWorld);
	world = XMMatrixMultiply(world, XMMatrixTranslation(0.0f, tHeight, 0.0f));
	worldInvTranspose = MathHelper::InverseTranspose(world);
	worldViewProj = world*view*proj;


	D3DX11_TECHNIQUE_DESC techDesc;
	switch (mLabel)
	{
	case Label::Basic:
		// Draw static opaque objects.
		tech->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			Effects::NormalMapFX->SetWorld(world);
			Effects::NormalMapFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::NormalMapFX->SetWorldViewProj(worldViewProj);
			Effects::NormalMapFX->SetWorldViewProjTex(worldViewProj*toTexSpace);
			Effects::NormalMapFX->SetShadowTransform(world*st);
			Effects::NormalMapFX->SetTexTransform(XMMatrixScaling(1.0f, 1.0f, 1.0f));

			for (UINT subset = 0; subset < mMesh->SubsetCount; ++subset)
			{
				Effects::NormalMapFX->SetMaterial(mMesh->Mat[subset]);
				Effects::NormalMapFX->SetDiffuseMap(mMesh->DiffuseMapSRV[subset]);
				Effects::NormalMapFX->SetNormalMap(mMesh->NormalMapSRV[subset]);

				tech->GetPassByIndex(p)->Apply(0, dc);
				mMesh->MeshData.Draw(dc, subset);
			}
		}
		break;

	case Label::AlphaBasic:
		// The alpha tested triangles are leaves, so render them double sided.
		dc->RSSetState(RenderStates::NoCullRS);
		alphaClippedTech->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{

			Effects::NormalMapFX->SetWorld(world);
			Effects::NormalMapFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::NormalMapFX->SetWorldViewProj(worldViewProj);
			Effects::NormalMapFX->SetWorldViewProjTex(worldViewProj*toTexSpace);
			Effects::NormalMapFX->SetShadowTransform(world*st);
			Effects::NormalMapFX->SetTexTransform(XMMatrixScaling(1.0f, 1.0f, 1.0f));

			for (UINT subset = 0; subset < mMesh->SubsetCount; ++subset)
			{
				Effects::NormalMapFX->SetMaterial(mMesh->Mat[subset]);
				Effects::NormalMapFX->SetDiffuseMap(mMesh->DiffuseMapSRV[subset]);
				Effects::NormalMapFX->SetNormalMap(mMesh->NormalMapSRV[subset]);

				alphaClippedTech->GetPassByIndex(p)->Apply(0, dc);
				mMesh->MeshData.Draw(dc, subset);
			}
		}
		break;
	}
}

void BasicObject::DrawToShadowMap(ID3D11DeviceContext* dc, const Camera& cam, const XMMATRIX& lightViewProj, const FLOAT& tHeight)
{
	Effects::BuildShadowMapFX->SetEyePosW(cam.GetPosition());
	Effects::BuildShadowMapFX->SetViewProj(lightViewProj);

	ID3DX11EffectTechnique* tech = Effects::BuildShadowMapFX->BuildShadowMapTech;
	ID3DX11EffectTechnique* alphaClippedTech = Effects::BuildShadowMapFX->BuildShadowMapAlphaClipTech;

	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	XMMATRIX world;
	XMMATRIX worldInvTranspose;
	XMMATRIX worldViewProj;

	dc->IASetInputLayout(InputLayouts::PosNormalTexTan);

	if (GetAsyncKeyState('1') & 0x8000)
		dc->RSSetState(RenderStates::WireframeRS);

	D3DX11_TECHNIQUE_DESC techDesc;

	world = XMLoadFloat4x4(&mWorld);
	world = XMMatrixMultiply(world, XMMatrixTranslation(0.0f, tHeight, 0.0f));
	worldInvTranspose = MathHelper::InverseTranspose(world);
	worldViewProj = world*lightViewProj;

	if (mLabel == Label::Basic) {
		tech->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			Effects::BuildShadowMapFX->SetWorld(world);
			Effects::BuildShadowMapFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::BuildShadowMapFX->SetWorldViewProj(worldViewProj);
			Effects::BuildShadowMapFX->SetTexTransform(XMMatrixScaling(1.0f, 1.0f, 1.0f));

			tech->GetPassByIndex(p)->Apply(0, dc);

			for (UINT subset = 0; subset < mMesh->SubsetCount; ++subset)
			{
				mMesh->MeshData.Draw(dc, subset);
			}
		}
	}

	if (mLabel == Label::AlphaBasic) {
		alphaClippedTech->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			Effects::BuildShadowMapFX->SetWorld(world);
			Effects::BuildShadowMapFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::BuildShadowMapFX->SetWorldViewProj(worldViewProj);
			Effects::BuildShadowMapFX->SetTexTransform(XMMatrixScaling(1.0f, 1.0f, 1.0f));

			for (UINT subset = 0; subset < mMesh->SubsetCount; ++subset)
			{
				Effects::BuildShadowMapFX->SetDiffuseMap(mMesh->DiffuseMapSRV[subset]);
				alphaClippedTech->GetPassByIndex(p)->Apply(0, dc);
				mMesh->MeshData.Draw(dc, subset);
			}
		}
	}
}

void BasicObject::DrawToSsaoNormalDepthMap(ID3D11DeviceContext* dc, const Camera& cam, const FLOAT& tHeight)
{
	XMMATRIX view = cam.View();
	XMMATRIX proj = cam.Proj();
	XMMATRIX viewProj = XMMatrixMultiply(view, proj);

	ID3DX11EffectTechnique* tech = Effects::SsaoNormalDepthFX->NormalDepthTech;
	ID3DX11EffectTechnique* alphaClippedTech = Effects::SsaoNormalDepthFX->NormalDepthAlphaClipTech;

	XMMATRIX world;
	XMMATRIX worldInvTranspose;
	XMMATRIX worldView;
	XMMATRIX worldInvTransposeView;
	XMMATRIX worldViewProj;

	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dc->IASetInputLayout(InputLayouts::PosNormalTexTan);

	if (GetAsyncKeyState('1') & 0x8000)
		dc->RSSetState(RenderStates::WireframeRS);

	world = XMLoadFloat4x4(&mWorld);
	world = XMMatrixMultiply(world, XMMatrixTranslation(0.0f, tHeight, 0.0f));
	worldInvTranspose = MathHelper::InverseTranspose(world);
	worldView = world*view;
	worldInvTransposeView = worldInvTranspose*view;
	worldViewProj = world*view*proj;

	D3DX11_TECHNIQUE_DESC techDesc;
	if (mLabel == Label::Basic) {
		tech->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			Effects::SsaoNormalDepthFX->SetWorldView(worldView);
			Effects::SsaoNormalDepthFX->SetWorldInvTransposeView(worldInvTransposeView);
			Effects::SsaoNormalDepthFX->SetWorldViewProj(worldViewProj);
			Effects::SsaoNormalDepthFX->SetTexTransform(XMMatrixScaling(1.0f, 1.0f, 1.0f));

			tech->GetPassByIndex(p)->Apply(0, dc);
			for (UINT subset = 0; subset < mMesh->SubsetCount; ++subset)
			{
				mMesh->MeshData.Draw(dc, subset);
			}
		}
	}

	// The alpha tested triangles are leaves, so render them double sided.
	if (mLabel == Label::AlphaBasic) {
		dc->RSSetState(RenderStates::NoCullRS);
		alphaClippedTech->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			Effects::SsaoNormalDepthFX->SetWorldView(worldView);
			Effects::SsaoNormalDepthFX->SetWorldInvTransposeView(worldInvTransposeView);
			Effects::SsaoNormalDepthFX->SetWorldViewProj(worldViewProj);
			Effects::SsaoNormalDepthFX->SetTexTransform(XMMatrixScaling(1.0f, 1.0f, 1.0f));

			for (UINT subset = 0; subset < mMesh->SubsetCount; ++subset)
			{
				Effects::SsaoNormalDepthFX->SetDiffuseMap(mMesh->DiffuseMapSRV[subset]);
				alphaClippedTech->GetPassByIndex(p)->Apply(0, dc);
				mMesh->MeshData.Draw(dc, subset);
			}
		}
	}
	dc->RSSetState(0);
}

void BasicObject::Release(ResourceMgr& rMgr)
{
	mMesh = nullptr;
}

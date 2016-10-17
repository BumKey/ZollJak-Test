#include "Temple.h"

Temple::Temple()
{

}

Temple::~Temple()
{
}

void Temple::Init(BasicMesh * mesh, const BO_InitDesc & info)
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
}

void Temple::DrawToScene(ID3D11DeviceContext * dc, const XMFLOAT4X4 & shadowTransform)
{
	const Camera& cam = *Camera::GetInstance();

	XMMATRIX view = cam.View();
	XMMATRIX proj = cam.Proj();
	XMMATRIX viewProj = cam.ViewProj();

	ID3DX11EffectTechnique* tech = Effects::NormalMapFX->TempleTech;

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

	float tHeight = Terrain::GetInstance()->GetHeight(mPosition);

	world = XMLoadFloat4x4(&mWorld);
	world = XMMatrixMultiply(world, XMMatrixTranslation(0.0f, tHeight, 0.0f));
	worldInvTranspose = MathHelper::InverseTranspose(world);
	worldViewProj = world*view*proj;

	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);

	// Draw static opaque objects.
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
}

void Temple::DrawToShadowMap(ID3D11DeviceContext * dc, const XMMATRIX & lightViewProj)
{
	const Camera& cam = *Camera::GetInstance();

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

	float tHeight = Terrain::GetInstance()->GetHeight(mPosition);

	world = XMLoadFloat4x4(&mWorld);
	world = XMMatrixMultiply(world, XMMatrixTranslation(0.0f, tHeight, 0.0f));
	worldInvTranspose = MathHelper::InverseTranspose(world);
	worldViewProj = world*lightViewProj;

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

void Temple::DrawToSsaoNormalDepthMap(ID3D11DeviceContext * dc)
{
	const Camera& cam = *Camera::GetInstance();

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

	float tHeight = Terrain::GetInstance()->GetHeight(mPosition);

	world = XMLoadFloat4x4(&mWorld);
	world = XMMatrixMultiply(world, XMMatrixTranslation(0.0f, tHeight, 0.0f));
	worldInvTranspose = MathHelper::InverseTranspose(world);
	worldView = world*view;
	worldInvTransposeView = worldInvTranspose*view;
	worldViewProj = world*view*proj;

	D3DX11_TECHNIQUE_DESC techDesc;

	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		Effects::SsaoNormalDepthFX->SetWorldView(worldView);
		Effects::SsaoNormalDepthFX->SetWorldInvTransposeView(worldInvTransposeView);
		Effects::SsaoNormalDepthFX->SetWorldViewProj(worldViewProj);
		Effects::SsaoNormalDepthFX->SetTexTransform(XMMatrixScaling(1.0f, 2.0f, 1.0f));

		tech->GetPassByIndex(p)->Apply(0, dc);
		for (UINT subset = 0; subset < mMesh->SubsetCount; ++subset)
		{
			mMesh->MeshData.Draw(dc, subset);
		}
	}
}

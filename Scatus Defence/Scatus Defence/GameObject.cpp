#include "GameObject.h"

GameObject::GameObject()
{
}


GameObject::GameObject(BasicModel * model, XMFLOAT4X4 world, Model_Effect me) : mModel(model),
mWorld(world), mME(me)
{
}


GameObject::~GameObject()
{
	SafeDelete(mModel);
}

void GameObject::Draw(ID3D11DeviceContext* dc, const Camera& cam, XMFLOAT4X4 shadowTransform)
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

	UINT stride = sizeof(Vertex::PosNormalTexTan);
	UINT offset = 0;

	dc->IASetInputLayout(InputLayouts::PosNormalTexTan);

	if (GetAsyncKeyState('1') & 0x8000)
		dc->RSSetState(RenderStates::WireframeRS);

	world = XMLoadFloat4x4(&mWorld);
	worldInvTranspose = MathHelper::InverseTranspose(world);
	worldViewProj = world*view*proj;

	//
	// Draw opaque objects.
	// 
	D3DX11_TECHNIQUE_DESC techDesc;
	if (mME == Model_Effect::Base) {
		tech->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			Effects::NormalMapFX->SetWorld(world);
			Effects::NormalMapFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::NormalMapFX->SetWorldViewProj(worldViewProj);
			Effects::NormalMapFX->SetWorldViewProjTex(worldViewProj*toTexSpace);
			Effects::NormalMapFX->SetShadowTransform(world*st);
			Effects::NormalMapFX->SetTexTransform(XMMatrixScaling(1.0f, 1.0f, 1.0f));

			for (UINT subset = 0; subset < mModel->SubsetCount; ++subset)
			{
				Effects::NormalMapFX->SetMaterial(mModel->Mat[subset]);
				Effects::NormalMapFX->SetDiffuseMap(mModel->DiffuseMapSRV[subset]);
				Effects::NormalMapFX->SetNormalMap(mModel->NormalMapSRV[subset]);

				tech->GetPassByIndex(p)->Apply(0, dc);
				mModel->ModelMesh.Draw(dc, subset);
			}
		}
	}
	// The alpha tested triangles are leaves, so render them double sided.
	if (mME == Model_Effect::Alpha) {
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

			for (UINT subset = 0; subset < mModel->SubsetCount; ++subset)
			{
				Effects::NormalMapFX->SetMaterial(mModel->Mat[subset]);
				Effects::NormalMapFX->SetDiffuseMap(mModel->DiffuseMapSRV[subset]);
				Effects::NormalMapFX->SetNormalMap(mModel->NormalMapSRV[subset]);

				alphaClippedTech->GetPassByIndex(p)->Apply(0, dc);
				mModel->ModelMesh.Draw(dc, subset);
			}
		}
	}
}

void GameObject::DrawObjectToShadowMap(ID3D11DeviceContext* dc, const Camera& cam, const XMMATRIX& lightViewProj)
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
	worldInvTranspose = MathHelper::InverseTranspose(world);
	worldViewProj = world*lightViewProj;

	if (mME == Model_Effect::Base) {
		tech->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			Effects::BuildShadowMapFX->SetWorld(world);
			Effects::BuildShadowMapFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::BuildShadowMapFX->SetWorldViewProj(worldViewProj);
			Effects::BuildShadowMapFX->SetTexTransform(XMMatrixScaling(1.0f, 1.0f, 1.0f));

			tech->GetPassByIndex(p)->Apply(0, dc);

			for (UINT subset = 0; subset < mModel->SubsetCount; ++subset)
			{
				mModel->ModelMesh.Draw(dc, subset);
			}
		}
	}

	if (mME == Model_Effect::Alpha) {
		alphaClippedTech->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			Effects::BuildShadowMapFX->SetWorld(world);
			Effects::BuildShadowMapFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::BuildShadowMapFX->SetWorldViewProj(worldViewProj);
			Effects::BuildShadowMapFX->SetTexTransform(XMMatrixScaling(1.0f, 1.0f, 1.0f));

			for (UINT subset = 0; subset < mModel->SubsetCount; ++subset)
			{
				Effects::BuildShadowMapFX->SetDiffuseMap(mModel->DiffuseMapSRV[subset]);
				alphaClippedTech->GetPassByIndex(p)->Apply(0, dc);
				mModel->ModelMesh.Draw(dc, subset);
			}
		}
	}
}

void GameObject::DrawObjectToSsaoNormalDepthMap(ID3D11DeviceContext * dc, const Camera & cam)
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
	worldInvTranspose = MathHelper::InverseTranspose(world);
	worldView = world*view;
	worldInvTransposeView = worldInvTranspose*view;
	worldViewProj = world*view*proj;

	D3DX11_TECHNIQUE_DESC techDesc;
	if (mME == Model_Effect::Base) {
		tech->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			Effects::SsaoNormalDepthFX->SetWorldView(worldView);
			Effects::SsaoNormalDepthFX->SetWorldInvTransposeView(worldInvTransposeView);
			Effects::SsaoNormalDepthFX->SetWorldViewProj(worldViewProj);
			Effects::SsaoNormalDepthFX->SetTexTransform(XMMatrixScaling(1.0f, 1.0f, 1.0f));

			tech->GetPassByIndex(p)->Apply(0, dc);
			for (UINT subset = 0; subset < mModel->SubsetCount; ++subset)
			{
				mModel->ModelMesh.Draw(dc, subset);
			}
		}
	}

	// The alpha tested triangles are leaves, so render them double sided.
	if (mME == Model_Effect::Alpha) {
		dc->RSSetState(RenderStates::NoCullRS);
		alphaClippedTech->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			Effects::SsaoNormalDepthFX->SetWorldView(worldView);
			Effects::SsaoNormalDepthFX->SetWorldInvTransposeView(worldInvTransposeView);
			Effects::SsaoNormalDepthFX->SetWorldViewProj(worldViewProj);
			Effects::SsaoNormalDepthFX->SetTexTransform(XMMatrixScaling(1.0f, 1.0f, 1.0f));

			for (UINT subset = 0; subset < mModel->SubsetCount; ++subset)
			{
				Effects::SsaoNormalDepthFX->SetDiffuseMap(mModel->DiffuseMapSRV[subset]);
				alphaClippedTech->GetPassByIndex(p)->Apply(0, dc);
				mModel->ModelMesh.Draw(dc, subset);
			}
		}
	}
	dc->RSSetState(0);
}

Model_Effect GameObject::GetType()	{	return mME;		}
BasicModel * GameObject::GetModel() {	return mModel;	}
XMFLOAT4X4 GameObject::GetWorld()	{	return mWorld;	}

void GameObject::Animate()
{
}

void GameObject::Move(XMVECTOR direction, float dt)
{
}

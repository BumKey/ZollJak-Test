#include "SceneMgr.h"

SceneMgr::SceneMgr() : mLightRotationAngle(0.0f), mScreenQuadVB(0), mScreenQuadIB(0), mSky(0), mSmap(0), mSsao(0)
{
	mDirLights[0].Ambient = XMFLOAT4(0.6f, 0.6f, 0.45f, 1.0f);
	mDirLights[0].Diffuse = XMFLOAT4(0.6f, 0.6f, 0.45f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	mDirLights[0].Direction = XMFLOAT3(0.58f, -0.63735f, 0.57735f);

	mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[1].Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[1].Direction = XMFLOAT3(0.707f, -0.707f, 0.0f);

	mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Diffuse = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	mDirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Direction = XMFLOAT3(0.0f, 0.0, -1.0f);

	mOriginalLightDir[0] = mDirLights[0].Direction;
	mOriginalLightDir[1] = mDirLights[1].Direction;
	mOriginalLightDir[2] = mDirLights[2].Direction;
}

SceneMgr::~SceneMgr()
{
	ReleaseCOM(mScreenQuadVB);
	ReleaseCOM(mScreenQuadIB);

	SafeDelete(mSky);
	SafeDelete(mSmap);
	SafeDelete(mSsao);
}

void SceneMgr::Init(ID3D11Device* device, ID3D11DeviceContext * dc,
	ID3D11DepthStencilView* dsv, ID3D11RenderTargetView* rtv,
	const Camera& cam, UINT width, UINT height)
{
	md3dImmediateContext = dc;
	mDepthStencilView = dsv;
	mRenderTargetView = rtv;

	mScreenViewport.TopLeftX = 0;
	mScreenViewport.TopLeftY = 0;
	mScreenViewport.Width = static_cast<float>(width);
	mScreenViewport.Height = static_cast<float>(height);
	mScreenViewport.MinDepth = 0.0f;
	mScreenViewport.MaxDepth = 1.0f;

	mSky = new Sky(device, L"Textures/grasscube1024.dds", 4000.0f);
	mSmap = new ShadowMap(device, SMapSize, SMapSize);
	mSsao = new Ssao(device, md3dImmediateContext, width, height, cam.GetFovY(), cam.GetFarZ());
	
	Terrain::InitInfo tii;
	tii.HeightMapFilename = L"Textures/terrain3.raw";
	tii.LayerMapFilename0 = L"Textures/grass2.dds";
	tii.LayerMapFilename1 = L"Textures/darkdirt.dds";
	tii.LayerMapFilename2 = L"Textures/red_dirt.dds";
	tii.LayerMapFilename3 = L"Textures/lightdirt.dds";
	tii.LayerMapFilename4 = L"Textures/dirt.dds";
	tii.BlendMapFilename = L"Textures/blend.dds";
	tii.HeightScale = 70.0f;
	tii.HeightmapWidth = 513;
	tii.HeightmapHeight = 513;
	tii.CellSpacing = 2.0f;
	mTerrain.Init(device, md3dImmediateContext, tii);
	
	BuildScreenQuadGeometryBuffers(device);
}

void SceneMgr::DrawScene(const std::vector<GameObject*>& allObjects, const Camera& cam)
{
	CreateShadowMap(allObjects, cam);
	CreateSsaoMap(allObjects, cam);

	//
	// Restore the back and depth buffer and viewport to the OM stage.
	//
	ID3D11RenderTargetView* renderTargets[1] = { mRenderTargetView };
	md3dImmediateContext->OMSetRenderTargets(1, renderTargets, mDepthStencilView);
	md3dImmediateContext->RSSetViewports(1, &mScreenViewport);

	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));

	// We already laid down scene depth to the depth buffer in the Normal/Depth map pass,
	// so we can set the depth comparison test to “EQUALS.? This prevents any overdraw
	// in this rendering pass, as only the nearest visible pixels will pass this depth
	// comparison test.

	md3dImmediateContext->OMSetDepthStencilState(RenderStates::EqualsDSS, 0);

	// Set per frame constants.
	Effects::NormalMapFX->SetDirLights(mDirLights);
	Effects::NormalMapFX->SetEyePosW(cam.GetPosition());
	Effects::NormalMapFX->SetCubeMap(mSky->CubeMapSRV());
	Effects::NormalMapFX->SetShadowMap(mSmap->DepthMapSRV());
	Effects::NormalMapFX->SetSsaoMap(mSsao->AmbientSRV());
	Effects::NormalMapFX->SetFogColor(Colors::LightSteelBlue);
	Effects::NormalMapFX->SetFogStart(30.0f);
	Effects::NormalMapFX->SetFogRange(300.0f);

	// Draw Objects
	for (auto i : allObjects) {
		XMFLOAT3 pos = i->GetPos();
		i->DrawToScene(md3dImmediateContext, cam, mShadowTransform, mTerrain.GetHeight(pos));
	}

	md3dImmediateContext->RSSetState(0);
	md3dImmediateContext->OMSetDepthStencilState(0, 0);

	mTerrain.DrawToScene(md3dImmediateContext, cam, mShadowTransform, mSmap->DepthMapSRV(), mDirLights);
	md3dImmediateContext->RSSetState(0);

	mSky->Draw(md3dImmediateContext, cam);

	//DrawScreenQuad();

	// restore default states, as the SkyFX changes them in the effect file.
	md3dImmediateContext->RSSetState(0);
	md3dImmediateContext->OMSetDepthStencilState(0, 0);

	// Unbind shadow map and AmbientMap as a shader input because we are going to render
	// to it next frame.  These textures can be at any slot, so clear all slots.
	ID3D11ShaderResourceView* nullSRV[16] = { 0 };
	md3dImmediateContext->PSSetShaderResources(0, 16, nullSRV);
}

void SceneMgr::Update(float dt)
{
	//
	// Animate the lights (and hence shadows).
	//

	BuildShadowTransform();
}

void SceneMgr::CreateSsaoMap(const std::vector<GameObject*>& allObjects, const Camera& cam)
{
	//
	// Render the view space normals and depths.  This render target has the
	// same dimensions as the back buffer, so we can use the screen viewport.
	// This render pass is needed to compute the ambient occlusion.
	// Notice that we use the main depth/stencil buffer in this pass.  
	//
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	md3dImmediateContext->RSSetViewports(1, &mScreenViewport);
	mSsao->SetNormalDepthRenderTarget(mDepthStencilView);

	for (auto i : allObjects) {
		XMFLOAT3 pos = i->GetPos();
		i->DrawToSsaoNormalDepthMap(md3dImmediateContext, cam, mTerrain.GetHeight(pos));
	}

	//
	// Now compute the ambient occlusion.
	// 

	mSsao->ComputeSsao(cam);
	mSsao->BlurAmbientMap(2);
}

void SceneMgr::CreateShadowMap(const std::vector<GameObject*>& allObjects, const Camera& cam)
{
	mSmap->BindDsvAndSetNullRenderTarget(md3dImmediateContext);

	XMMATRIX view = XMLoadFloat4x4(&mLightView);
	XMMATRIX proj = XMLoadFloat4x4(&mLightProj);
	XMMATRIX viewProj = XMMatrixMultiply(view, proj);

	for (auto i : allObjects) {
		XMFLOAT3 pos = i->GetPos();
		i->DrawToShadowMap(md3dImmediateContext, cam, viewProj, mTerrain.GetHeight(pos));
	}

	md3dImmediateContext->RSSetState(0);
}

void SceneMgr::DrawScreenQuad()
{
	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;

	md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mScreenQuadVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mScreenQuadIB, DXGI_FORMAT_R32_UINT, 0);

	// Scale and shift quad to lower-right corner.
	XMMATRIX world(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, -0.5f, 0.0f, 1.0f);

	ID3DX11EffectTechnique* tech = Effects::DebugTexFX->ViewRedTech;
	D3DX11_TECHNIQUE_DESC techDesc;

	tech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		Effects::DebugTexFX->SetWorldViewProj(world);
		Effects::DebugTexFX->SetTexture(mSmap->DepthMapSRV());

		tech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(6, 0, 0);
	}
}

void SceneMgr::BuildShadowTransform()
{
	// Only the first "main" light casts a shadow.
	XMVECTOR lightDir = XMLoadFloat3(&mDirLights[0].Direction);
	XMVECTOR lightPos = -2.0f*mSceneBounds.Radius*lightDir;
	XMVECTOR targetPos = XMLoadFloat3(&mSceneBounds.Center);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(lightPos, targetPos, up);

	// Transform bounding sphere to light space.
	XMFLOAT3 sphereCenterLS;
	XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, V));

	// Ortho frustum in light space encloses scene.
	float l = sphereCenterLS.x - mSceneBounds.Radius;
	float b = sphereCenterLS.y - mSceneBounds.Radius;
	float n = sphereCenterLS.z - mSceneBounds.Radius;
	float r = sphereCenterLS.x + mSceneBounds.Radius;
	float t = sphereCenterLS.y + mSceneBounds.Radius;
	float f = sphereCenterLS.z + mSceneBounds.Radius;
	XMMATRIX P = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX S = V*P*T;

	XMStoreFloat4x4(&mLightView, V);
	XMStoreFloat4x4(&mLightProj, P);
	XMStoreFloat4x4(&mShadowTransform, S);
}

void SceneMgr::BuildScreenQuadGeometryBuffers(ID3D11Device* device)
{
	GeometryGenerator::MeshData quad;

	GeometryGenerator geoGen;
	geoGen.CreateFullscreenQuad(quad);

	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

	std::vector<Vertex::Basic32> vertices(quad.Vertices.size());

	for (UINT i = 0; i < quad.Vertices.size(); ++i)
	{
		vertices[i].Pos = quad.Vertices[i].Position;
		vertices[i].Normal = quad.Vertices[i].Normal;
		vertices[i].Tex = quad.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * quad.Vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(device->CreateBuffer(&vbd, &vinitData, &mScreenQuadVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * quad.Indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &quad.Indices[0];
	HR(device->CreateBuffer(&ibd, &iinitData, &mScreenQuadIB));
}

void SceneMgr::ComputeSceneBoundingBox(const XMFLOAT3& playerPos)
{
	//XMFLOAT3 minPt(+MathHelper::Infinity, +MathHelper::Infinity, +MathHelper::Infinity);
	//XMFLOAT3 maxPt(-MathHelper::Infinity, -MathHelper::Infinity, -MathHelper::Infinity);
	//for (auto iter : allObjects)
	//{
	//	for (UINT i = 0; i < iter->GetMesh()->VerticesPos.size(); ++i)
	//	{
	//		XMFLOAT3 P = iter->GetMesh()->VerticesPos[i];

	//		minPt.x = MathHelper::Min(minPt.x, P.x);
	//		minPt.y = MathHelper::Min(minPt.x, P.x);
	//		minPt.z = MathHelper::Min(minPt.x, P.x);

	//		maxPt.x = MathHelper::Max(maxPt.x, P.x);
	//		maxPt.y = MathHelper::Max(maxPt.x, P.x);
	//		maxPt.z = MathHelper::Max(maxPt.x, P.x);
	//	}
	//}

	////
	//// Derive scene bounding sphere from bounding box.
	////
	//mSceneBounds.Center = XMFLOAT3(
	//	0.5f*(minPt.x + maxPt.x),
	//	0.5f*(minPt.y + maxPt.y),
	//	0.5f*(minPt.z + maxPt.z));

	//XMFLOAT3 extent(
	//	0.5f*(maxPt.x - minPt.x),
	//	0.5f*(maxPt.y - minPt.y),
	//	0.5f*(maxPt.z - minPt.z));

	//mSceneBounds.Radius = sqrtf(extent.x*extent.x + extent.y*extent.y + extent.z*extent.z);

	float halfW = mTerrain.GetWidth() / 2.0f;
	float halfD = mTerrain.GetDepth() / 2.0f;
	mSceneBounds.Radius = sqrtf((halfW*halfW) + (halfD*halfD))/5.0f;
	mSceneBounds.Center.x = playerPos.x + 50.0f;
	mSceneBounds.Center.y = 0.0f;
	mSceneBounds.Center.z = playerPos.z - 50.0f;

}

void SceneMgr::OnResize(UINT width, UINT height, const Camera& cam,
	ID3D11DepthStencilView* dsv, ID3D11RenderTargetView* rtv)
{
	mDepthStencilView = dsv;
	mRenderTargetView = rtv;

	mScreenViewport.TopLeftX = 0;
	mScreenViewport.TopLeftY = 0;
	mScreenViewport.Width = static_cast<float>(width);
	mScreenViewport.Height = static_cast<float>(height);
	mScreenViewport.MinDepth = 0.0f;
	mScreenViewport.MaxDepth = 1.0f;

	if(mSsao)
		mSsao->OnSize(width, height, cam.GetFovY(), cam.GetFarZ());
}


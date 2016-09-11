#include "SceneMgr.h"

SceneMgr::SceneMgr() : mLightRotationAngle(0.0f), mScreenQuadVB(0), mScreenQuadIB(0), mSky(0), mSmap(0), mSsao(0)
{
	mDirLights[0].Ambient = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
	mDirLights[0].Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	mDirLights[0].Direction = XMFLOAT3(0.58f, -0.63735f, 0.57735f);

	mDirLights[1].Ambient = XMFLOAT4(0.3f, 0.2f, 0.3f, 1.0f);
	mDirLights[1].Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[1].Direction = XMFLOAT3(0.707f, -0.707f, 0.0f);

	mDirLights[2].Ambient = XMFLOAT4(0.1f, 0.2f, 0.1f, 1.0f);
	mDirLights[2].Diffuse = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	mDirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Direction = XMFLOAT3(0.0f, 0.0, -1.0f);

	mOriginalLightDir[0] = mDirLights[0].Direction;
	mOriginalLightDir[1] = mDirLights[1].Direction;
	mOriginalLightDir[2] = mDirLights[2].Direction;

	mTreeMat.Ambient = XMFLOAT3(0.5f, 0.5f, 0.5f);
	mTreeMat.Diffuse = XMFLOAT3(0.6f, 0.6f, 0.6f);
	mTreeMat.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	mGrassMat.Ambient = XMFLOAT3(0.3f, 0.3f, 0.3f);
	mGrassMat.Diffuse = XMFLOAT3(0.4f, 0.5f, 0.4f);
	mGrassMat.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
}

SceneMgr::~SceneMgr()
{
	ReleaseCOM(mScreenQuadVB);
	ReleaseCOM(mScreenQuadIB);

	SafeDelete(mSky);
	SafeDelete(mSmap);
	SafeDelete(mSsao);

	ReleaseCOM(mTreeSpritesVB);
	ReleaseCOM(mTreeTextureMapArraySRV);
	ReleaseCOM(mGrassSpritesVB);
	ReleaseCOM(mGrassTextureMapArraySRV);
}

void SceneMgr::Init(ID3D11Device* device, ID3D11DeviceContext * dc,
	ID3D11DepthStencilView* dsv, ID3D11RenderTargetView* rtv,
	UINT width, UINT height)
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

	Camera* cam = Camera::GetInstance();
	mSsao = new Ssao(device, md3dImmediateContext, width, height, cam->GetFovY(), cam->GetFarZ());

	std::vector<std::wstring> treeFilenames;
	treeFilenames.push_back(L"Textures/tree0.dds");
	treeFilenames.push_back(L"Textures/tree1.dds");
	treeFilenames.push_back(L"Textures/tree2.dds");

	mTreeTextureMapArraySRV = d3dHelper::CreateTexture2DArraySRV(
		device, md3dImmediateContext, treeFilenames, DXGI_FORMAT_R8G8B8A8_UNORM);

	std::vector<std::wstring> grassFilenames;
	grassFilenames.push_back(L"Textures/b_grass1.dds");
	grassFilenames.push_back(L"Textures/b_grass2.dds");
	grassFilenames.push_back(L"Textures/b_grass3.dds");

	mGrassTextureMapArraySRV = d3dHelper::CreateTexture2DArraySRV(
		device, md3dImmediateContext, grassFilenames, DXGI_FORMAT_R8G8B8A8_UNORM);

	BuildScreenQuadGeometryBuffers(device);
	BuildTreeSpritesBuffer(device);
	BuildGrassSpritesBuffer(device);
	BuildDebugSphere(device);
}

void SceneMgr::DrawScene()
{
	const Camera& cam = *Camera::GetInstance();
	const auto allObjects = Object_Mgr->GetAllObjects();

	CreateShadowMap();
	CreateSsaoMap();

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
		i->DrawToScene(md3dImmediateContext, mShadowTransform);
	}

	md3dImmediateContext->RSSetState(0);
	md3dImmediateContext->OMSetDepthStencilState(0, 0);

	Terrain::GetInstance()->DrawToScene(md3dImmediateContext, mShadowTransform, mSmap->DepthMapSRV(), mDirLights);
	md3dImmediateContext->RSSetState(0);

	DrawTreeSprites();
	DrawGrassSprites();

	if (GetAsyncKeyState('X') & 0x8000)
		DrawBS();

	if(GetAsyncKeyState('Z') & 0x8000)
		DrawScreenQuadSsao();

	if (GetAsyncKeyState('C') & 0x8000)
		DrawScreenQuadShadow();

	mSky->Draw(md3dImmediateContext, cam);

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
	mSceneBounds.Center = Player::GetInstance()->GetPos();
	BuildShadowTransform();
}

void SceneMgr::CreateSsaoMap()
{
	const Camera& cam = *Camera::GetInstance();
	const auto allObjects = Object_Mgr->GetAllObjects();

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
		i->DrawToSsaoNormalDepthMap(md3dImmediateContext);
	}

	//
	// Now compute the ambient occlusion.
	// 

	mSsao->ComputeSsao(cam);
	mSsao->BlurAmbientMap(3);
}

void SceneMgr::CreateShadowMap()
{
	const Camera& cam = *Camera::GetInstance();
	const auto allObjects = Object_Mgr->GetAllObjects();

	mSmap->BindDsvAndSetNullRenderTarget(md3dImmediateContext);

	XMMATRIX view = XMLoadFloat4x4(&mLightView);
	XMMATRIX proj = XMLoadFloat4x4(&mLightProj);
	XMMATRIX viewProj = XMMatrixMultiply(view, proj);

	for (auto i : allObjects) {
		XMFLOAT3 pos = i->GetPos();
		i->DrawToShadowMap(md3dImmediateContext, viewProj);
	}

	md3dImmediateContext->RSSetState(0);
}

void SceneMgr::DrawTreeSprites()
{
	const Camera& cam = *Camera::GetInstance();

	Effects::TreeSpriteFX->SetDirLights(mDirLights);
	Effects::TreeSpriteFX->SetEyePosW(cam.GetPosition());
	Effects::TreeSpriteFX->SetFogColor(Colors::Silver);
	Effects::TreeSpriteFX->SetFogStart(15.0f);
	Effects::TreeSpriteFX->SetFogRange(175.0f);
	Effects::TreeSpriteFX->SetViewProj(cam.ViewProj());
	Effects::TreeSpriteFX->SetMaterial(mTreeMat);
	Effects::TreeSpriteFX->SetTreeTextureMapArray(mTreeTextureMapArraySRV);

	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	md3dImmediateContext->IASetInputLayout(InputLayouts::TreePointSprite);
	UINT stride = sizeof(Vertex::TreePointSprite);
	UINT offset = 0;

	ID3DX11EffectTechnique* treeTech;
	treeTech = Effects::TreeSpriteFX->Light3TexAlphaClipTech;

	D3DX11_TECHNIQUE_DESC techDesc;
	treeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mTreeSpritesVB, &stride, &offset);
		float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

		//md3dImmediateContext->OMSetBlendState(RenderStates::AlphaToCoverageBS, blendFactor, 0xffffffff);
		treeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->Draw(TreeCount, 0);

		md3dImmediateContext->OMSetBlendState(0, blendFactor, 0xffffffff);
	}
}

void SceneMgr::DrawGrassSprites()
{
	const Camera& cam = *Camera::GetInstance();

	Effects::TreeSpriteFX->SetDirLights(mDirLights);
	Effects::TreeSpriteFX->SetEyePosW(cam.GetPosition());
	Effects::TreeSpriteFX->SetFogColor(Colors::Silver);
	Effects::TreeSpriteFX->SetFogStart(15.0f);
	Effects::TreeSpriteFX->SetFogRange(175.0f);
	Effects::TreeSpriteFX->SetViewProj(cam.ViewProj());
	Effects::TreeSpriteFX->SetMaterial(mGrassMat);
	Effects::TreeSpriteFX->SetTreeTextureMapArray(mGrassTextureMapArraySRV);

	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	md3dImmediateContext->IASetInputLayout(InputLayouts::TreePointSprite);
	UINT stride = sizeof(Vertex::TreePointSprite);
	UINT offset = 0;

	ID3DX11EffectTechnique* treeTech;
	treeTech = Effects::TreeSpriteFX->Light3TexAlphaClipTech;

	D3DX11_TECHNIQUE_DESC techDesc;
	treeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mGrassSpritesVB, &stride, &offset);
		float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

		//md3dImmediateContext->OMSetBlendState(RenderStates::AlphaToCoverageBS, blendFactor, 0xffffffff);
		treeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->Draw(GrassCount, 0);

		md3dImmediateContext->OMSetBlendState(0, blendFactor, 0xffffffff);
	}
}

void SceneMgr::DrawScreenQuadSsao()
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
		Effects::DebugTexFX->SetTexture(mSsao->AmbientSRV());

		tech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(6, 0, 0);
	}
}


void SceneMgr::DrawScreenQuadShadow()
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

void SceneMgr::DrawBS()
{
	UINT stride = sizeof(XMFLOAT3);
	UINT offset = 0;

	md3dImmediateContext->IASetInputLayout(InputLayouts::Pos);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mDebugSphereVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mDebugSphereIB, DXGI_FORMAT_R32_UINT, 0);

	for (auto iterO : Object_Mgr->GetAllObjects())
	{
		FLOAT s = iterO->GetBS().Radius;
		XMFLOAT3 p = iterO->GetBS().Center;
		p.y += Terrain::GetInstance()->GetHeight(p);

		XMMATRIX S = XMMatrixScaling(s, s, s);
		XMMATRIX T = XMMatrixTranslation(p.x, p.y, p.z);
		XMMATRIX world = S*T;

		ID3DX11EffectTechnique* tech = Effects::WireFX->WireFrameTech;
		D3DX11_TECHNIQUE_DESC techDesc;

		Effects::WireFX->SetWorldViewProj(world * Camera::GetInstance()->ViewProj());

		tech->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			tech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
			md3dImmediateContext->DrawIndexed(mDSIndicesNum, 0, 0);
		}
	}
	md3dImmediateContext->RSSetState(0);
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

void SceneMgr::BuildDebugSphere(ID3D11Device * device)
{
	GeometryGenerator::MeshData bs;

	GeometryGenerator geoGen;
	geoGen.CreateSphere(1.0f, 12, 4, bs);
	mDSIndicesNum = bs.Indices.size();

	std::vector<XMFLOAT3> vertices(bs.Vertices.size());

	for (UINT i = 0; i < bs.Vertices.size(); ++i)
		vertices[i] = bs.Vertices[i].Position;
	

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(XMFLOAT3) * bs.Vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(device->CreateBuffer(&vbd, &vinitData, &mDebugSphereVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * bs.Indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &bs.Indices[0];
	HR(device->CreateBuffer(&ibd, &iinitData, &mDebugSphereIB));
}

void SceneMgr::BuildTreeSpritesBuffer(ID3D11Device * device)
{
	Vertex::TreePointSprite v[TreeCount];

	for (UINT i = 0; i < TreeCount; ++i)
	{
		float x = MathHelper::RandF(-200.0f, 200.0f);
		float z = MathHelper::RandF(100.0f, 200.0f);
		float y = Terrain::GetInstance()->GetHeight(x, z);

		// Move tree slightly above land height.
		y += 5.1f;

		v[i].Pos = XMFLOAT3(x, y, z);
		v[i].Size = XMFLOAT2(10.0f + MathHelper::RandF(0.0f, 10.0f),
			10.0f + MathHelper::RandF(0.0f, 10.0f));
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::TreePointSprite) * TreeCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = v;
	HR(device->CreateBuffer(&vbd, &vinitData, &mTreeSpritesVB));
}

void SceneMgr::BuildGrassSpritesBuffer(ID3D11Device * device)
{
	Vertex::TreePointSprite v[GrassCount];

	for (UINT i = 0; i < GrassCount; ++i)
	{
		float x = MathHelper::RandF(-300.0f, 300.0f);
		float z = MathHelper::RandF(-450.0f, 200.0f);
		float y = Terrain::GetInstance()->GetHeight(x, z);

		// Move tree slightly above land height.
		y += 0.5f;

		v[i].Pos = XMFLOAT3(x, y, z);
		v[i].Size = XMFLOAT2(1.0f + MathHelper::RandF(0.0f, 2.0f),
			1.0f + MathHelper::RandF(0.0f, 2.0f));
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::TreePointSprite) * GrassCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = v;
	HR(device->CreateBuffer(&vbd, &vinitData, &mGrassSpritesVB));
}

void SceneMgr::ComputeSceneBoundingBox()
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
	const XMFLOAT3& playerPos = Player::GetInstance()->GetPos();

	float halfW = Terrain::GetInstance()->GetWidth() / 2.0f;
	float halfD = Terrain::GetInstance()->GetDepth() / 2.0f;
	mSceneBounds.Radius = sqrtf((halfW*halfW) + (halfD*halfD))/5.0f;
	mSceneBounds.Center = playerPos;
}

void SceneMgr::OnResize(UINT width, UINT height,
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

	const Camera& cam = *Camera::GetInstance();
	if(mSsao)
		mSsao->OnSize(width, height, cam.GetFovY(), cam.GetFarZ());
}


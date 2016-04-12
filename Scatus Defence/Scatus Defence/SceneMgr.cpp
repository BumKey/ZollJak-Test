#include "SceneMgr.h"

SceneMgr::SceneMgr(int width, int height) : md3dImmediateContext(0), mClientWidth(width), mClientHeight(height),
mLightRotationAngle(0.0f), mScreenQuadVB(0), mScreenQuadIB(0), mSky(0), mSmap(0), mSsao(0)
{
	mCam.SetPosition(0.0f, 2.0f, -15.0f);

	mDirLights[0].Ambient = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
	mDirLights[0].Diffuse = XMFLOAT4(0.8f, 0.7f, 0.7f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.6f, 0.6f, 0.7f, 1.0f);
	mDirLights[0].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

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

void SceneMgr::Init(ID3D11Device* device, ID3D11DeviceContext * dc)
{
	md3dImmediateContext = dc;

	// 오류 원인 : float ratio = static_cast<float>(mClientWidth / mClientHeight);
	// 이거 때문에 일주일을 고생하다니... 대체 뭔 차이엿던거지
	// 차이가 있더라도 렌더링이 그딴식으로 되다니.. 참나

	float ratio = static_cast<float>(mClientWidth) / mClientHeight;
	mCam.SetLens(0.25f*MathHelper::Pi, ratio, 1.0f, 1000.0f);

	mSky = new Sky(device, L"Textures/desertcube1024.dds", 5000.0f);
	mSmap = new ShadowMap(device, SMapSize, SMapSize);
	mSsao = new Ssao(device, dc, mClientWidth, mClientHeight, mCam.GetFovY(), mCam.GetFarZ());

	BuildScreenQuadGeometryBuffers(device);
}

void SceneMgr::DrawAllObjects()
{
	// Set per frame constants.
	Effects::NormalMapFX->SetDirLights(mDirLights);
	Effects::NormalMapFX->SetEyePosW(mCam.GetPosition());
	Effects::NormalMapFX->SetCubeMap(mSky->CubeMapSRV());
	Effects::NormalMapFX->SetShadowMap(mSmap->DepthMapSRV());
	Effects::NormalMapFX->SetSsaoMap(mSsao->AmbientSRV());

	for (auto i : mObjects)
		i->Draw(md3dImmediateContext, mCam, mShadowTransform);
}

void SceneMgr::AnimateAllObjects()
{
	for (auto i : mObjects)
		i->Animate();
}

void SceneMgr::UpdateScene(float dt)
{
	//
	// Control the camera.
	//
	if (GetAsyncKeyState('W') & 0x8000)
		mCam.Walk(10.0f*dt);

	if (GetAsyncKeyState('S') & 0x8000)
		mCam.Walk(-10.0f*dt);

	if (GetAsyncKeyState('A') & 0x8000)
		mCam.Strafe(-10.0f*dt);

	if (GetAsyncKeyState('D') & 0x8000)
		mCam.Strafe(10.0f*dt);


	//
	// Animate the lights (and hence shadows).
	//

	BuildShadowTransform();

	mCam.UpdateViewMatrix();

}


void SceneMgr::CreateSsaoMap(ID3D11DepthStencilView* dsv)
{
	D3D11_VIEWPORT screenViewport;
	screenViewport.TopLeftX = 0;
	screenViewport.TopLeftY = 0;
	screenViewport.Width = static_cast<float>(mClientWidth);
	screenViewport.Height = static_cast<float>(mClientHeight);
	screenViewport.MinDepth = 0.0f;
	screenViewport.MaxDepth = 1.0f;

	//
	// Render the view space normals and depths.  This render target has the
	// same dimensions as the back buffer, so we can use the screen viewport.
	// This render pass is needed to compute the ambient occlusion.
	// Notice that we use the main depth/stencil buffer in this pass.  
	//
	md3dImmediateContext->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	md3dImmediateContext->RSSetViewports(1, &screenViewport);
	SetNormalDepthRenderTarget(dsv);

	for (auto i : mObjects)
		i->DrawObjectToSsaoNormalDepthMap(md3dImmediateContext, mCam);

	//
	// Now compute the ambient occlusion.
	// 
	ComputeSsao();
	BlurAmbientMap(2);
}

void SceneMgr::CreateShadowMap()
{
	BindDsvAndSetNullRenderTarget();

	XMMATRIX view = XMLoadFloat4x4(&mLightView);
	XMMATRIX proj = XMLoadFloat4x4(&mLightProj);
	XMMATRIX viewProj = XMMatrixMultiply(view, proj);

	for (auto i : mObjects)
		i->DrawObjectToShadowMap(md3dImmediateContext, mCam, viewProj);

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
		Effects::DebugTexFX->SetTexture(mSsao->AmbientSRV());

		tech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(6, 0, 0);
	}
}

void SceneMgr::CameraYawPitch(float dx, float dy)
{
	mCam.Pitch(dy);
	mCam.RotateY(dx);
}

void SceneMgr::DrawSky()
{
	mSky->Draw(md3dImmediateContext, mCam);

	// restore default states, as the SkyFX changes them in the effect file.
	md3dImmediateContext->RSSetState(0);
	md3dImmediateContext->OMSetDepthStencilState(0, 0);
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

void SceneMgr::BindDsvAndSetNullRenderTarget()
{
	mSmap->BindDsvAndSetNullRenderTarget(md3dImmediateContext);
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

void SceneMgr::ComputeSceneBoundingBox()
{
	XMFLOAT3 minPt(+MathHelper::Infinity, +MathHelper::Infinity, +MathHelper::Infinity);
	XMFLOAT3 maxPt(-MathHelper::Infinity, -MathHelper::Infinity, -MathHelper::Infinity);
	for (auto i : mObjects)
	{
		if (i->GetType() == Model_Effect::Base) {
			for (UINT j = 0; j < i->GetModel()->Vertices.size(); ++j)
			{
				XMFLOAT3 P = i->GetModel()->Vertices[j].Pos;

				minPt.x = MathHelper::Min(minPt.x, P.x);
				minPt.y = MathHelper::Min(minPt.x, P.x);
				minPt.z = MathHelper::Min(minPt.x, P.x);

				maxPt.x = MathHelper::Max(maxPt.x, P.x);
				maxPt.y = MathHelper::Max(maxPt.x, P.x);
				maxPt.z = MathHelper::Max(maxPt.x, P.x);
			}
		}
	}

	//
	// Derive scene bounding sphere from bounding box.
	//
	mSceneBounds.Center = XMFLOAT3(
		0.5f*(minPt.x + maxPt.x),
		0.5f*(minPt.y + maxPt.y),
		0.5f*(minPt.z + maxPt.z));

	XMFLOAT3 extent(
		0.5f*(maxPt.x - minPt.x),
		0.5f*(maxPt.y - minPt.y),
		0.5f*(maxPt.z - minPt.z));

	mSceneBounds.Radius = sqrtf(extent.x*extent.x + extent.y*extent.y + extent.z*extent.z);
}

void SceneMgr::ReSize(UINT width, UINT height)
{
	mClientWidth = width;
	mClientHeight = height;

	float ratio = static_cast<float>(mClientWidth) / mClientHeight;
	mCam.SetLens(0.25f*MathHelper::Pi, ratio, 1.0f, 1000.0f);

	if(mSsao)
		mSsao->OnSize(width, height, mCam.GetFovY(), mCam.GetFarZ());
}

void SceneMgr::SetNormalDepthRenderTarget(ID3D11DepthStencilView* dsv)
{
	mSsao->SetNormalDepthRenderTarget(dsv);
}

void SceneMgr::ComputeSsao()
{
	mSsao->ComputeSsao(mCam);
}

void SceneMgr::BlurAmbientMap(int blurCount)
{
	mSsao->BlurAmbientMap(blurCount);
}

void SceneMgr::AddObject(BasicModel * mesh, XMFLOAT4X4 world, Model_Effect me)
{
	mObjects.push_back(new GameObject(mesh, world, me));
}

DirectionalLight * SceneMgr::GetDirLight()
{
	return mDirLights;
}

#include "d3dApp.h"
#include "d3dx11Effect.h"
#include "GeometryGenerator.h"
#include "MathHelper.h"
#include "LightHelper.h"
#include "Effects.h"
#include "RenderStates.h"
#include "SceneMgr.h"
#include "ObjectMgr.h"
#include "TextureMgr.h"
#include "ResourceMgr.h"

class GameFrameWork : public D3DApp
{
public:
	GameFrameWork(HINSTANCE hInstance);
	~GameFrameWork();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	bool m_bReady;
	bool m_bAttackAnim;

	TextureMgr  mTexMgr;
	SceneMgr	mSceneMgr;
	ObjectMgr	mObjectMgr;

	Camera mCam;
	Player* mPlayer;

	POINT mLastMousePos;

};

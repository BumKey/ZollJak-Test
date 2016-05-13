#include "Core.h"


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
	ResourceMgr mResourceMgr;
	ObjectMgr	mObjectMgr;
	SceneMgr	mSceneMgr;

	Camera mCam;
	Player* mPlayer;

	POINT mLastMousePos;

};

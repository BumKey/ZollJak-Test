#include "d3dApp.h"
#include "TextureMgr.h"
#include "SceneMgr.h"
#include "ObjectMgr.h"
#include "ResourceMgr.h"
#include "GameRogicManager.h"
#include "CollisionMgr.h"
#include "Goblin.h"
#include "Cyclop.h"
#include "UI_Manager.h"
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

	ResourceMgr mResourceMgr;
	ObjectMgr	mObjectMgr;
	SceneMgr	mSceneMgr;
	CollisionMgr mCollisionMgr;
	GameRogicManager *mGameRogicMgr;
	Camera mCam;
	Player* mPlayer;

	POINT mLastMousePos;

};

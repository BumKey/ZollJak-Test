#include "d3dApp.h"
#include "TextureMgr.h"
#include "SceneMgr.h"
#include "PacketMgr.h"
#include "ObjectMgr.h"
#include "ResourceMgr.h"
#include "Sound_Manager.h"
#include "Singletone.h"
#include "Player.h"
//#include "GameStateMgr.h"
class GameFrameWork : public D3DApp
{
public:
	GameFrameWork(HINSTANCE hInstance);
	~GameFrameWork();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void ProcessPacket(char* packet, float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);
	void OnKeyDown(WPARAM keyState);

private:
	POINT				mLastMousePos;
};

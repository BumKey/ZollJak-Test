#include "Player.h"

Player::Player() : SkinnedObject()
{
	mProperty.name = "Player";

	mProperty.movespeed = 6.0f;
	mProperty.hp_now = 100;
	mProperty.attackspeed = 2.0f;
	mProperty.attakpoint = 50.0f;

	mObjectType = ObjectType::Player;

	// 임시로 고블린 애니메이션
	mAnimNames[Anims::attack1] = "attack01";
	mAnimNames[Anims::attack2] = "attack02";
	mAnimNames[Anims::hit] = "damage";
	mAnimNames[Anims::dead] = "dead";
	mAnimNames[Anims::drop_down] = "drop_down";
	mAnimNames[Anims::run] = "run";
	mAnimNames[Anims::sit_up] = "sit_up";
	mAnimNames[Anims::idle] = "stand";
	mAnimNames[Anims::walk] = "walk";
	mAnimNames[Anims::look_around] = "stand2";
}


Player::~Player()
{
}

void Player::Update(float dt)
{
	Move(dt);

	if (mActionState == ActionState::Attack && CurrAnimEnd())
		mActionState = ActionState::Idle;

	SkinnedObject::Update(dt);
}

void Player::Move(float dt)
{
	cs_packet_success packet;

	packet.size = sizeof(packet);
	packet.type = CS_SUCCESS;
	packet.success = true;

	if (mActionState != ActionState::Attack)
	{
		if ((GetAsyncKeyState('W') & 0x8000) && (GetAsyncKeyState('A') & 0x8000))
		{
			Walk(-dt);
			Strafe(-dt);
			mActionState = ActionState::Run;
		}
		else if ((GetAsyncKeyState('W') & 0x8000) && (GetAsyncKeyState('D') & 0x8000))
		{
			Walk(-dt);
			Strafe(dt);
			mActionState = ActionState::Run;
			Packet_Mgr->SendPacket(packet);

		}
		else if ((GetAsyncKeyState('S') & 0x8000) && (GetAsyncKeyState('A') & 0x8000))
		{
			Walk(dt);
			Strafe(-dt);
			mActionState = ActionState::Run;
		}
		else if ((GetAsyncKeyState('S') & 0x8000) && (GetAsyncKeyState('D') & 0x8000))
		{
			Walk(dt);
			Strafe(dt);
			mActionState = ActionState::Run;
		}
		else if (GetAsyncKeyState('W') & 0x8000) 
		{ 
			Walk(-dt); 
			mActionState = ActionState::Run; 
		}
		else if (GetAsyncKeyState('S') & 0x8000) { Walk(dt); mActionState = ActionState::Run; }
		else if (GetAsyncKeyState('A') & 0x8000) { Strafe(-dt); mActionState = ActionState::Run; }
		else if (GetAsyncKeyState('D') & 0x8000) { Strafe(dt); mActionState = ActionState::Run; }
		else
			mActionState = ActionState::Idle;
	}
}

void Player::Release(ResourceMgr & rMgr)
{
	rMgr.ReleaseMesh(mObjectType); // 임시
}


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
	//mAnimNames[Anims::dead] = "dead";
	//mAnimNames[Anims::drop_down] = "drop_down";
	mAnimNames[Anims::run] = "run";
	mAnimNames[Anims::idle] = "idle";
	mAnimNames[Anims::walk] = "walk";
}


Player::~Player()
{
}

void Player::Init(SkinnedMesh * mesh, const InstanceDesc & info)
{
	SkinnedObject::Init(mesh, info);
}

void Player::Update(float dt)
{
	ProccessKeyInput(dt);

	if ( (mActionState == ActionState::Attack || 
		mActionState == ActionState::Damage) && CurrAnimEnd())
		mActionState = ActionState::Idle;

	SkinnedObject::Update(dt);
}

void Player::CollisionMoving(const XMFLOAT3& dPos, float dt)
{
	XMFLOAT3 target;
	XMStoreFloat3(&target, MathHelper::TargetVector2D(dPos, mPosition));

	mPosition = mPosition - target*dt*mProperty.movespeed;
}

void Player::Move(float walk, float strafe)
{
	cs_packet packet;

	packet.size = sizeof(packet);
	packet.type = CS_TEST;
	packet.cInfo.Pos = mPosition;

	if (walk != 0.0f)
		Walk(walk);

	if (strafe != 0.0f)
		Strafe(strafe);

	mActionState = ActionState::Run;
	Packet_Mgr->SendPacket(packet);
}

void Player::ProccessKeyInput(float dt)
{
	if (mActionState != ActionState::Attack && mActionState != ActionState::Damage && mActionState != ActionState::Die)
	{
		if ((GetAsyncKeyState('W') & 0x8000) && (GetAsyncKeyState('A') & 0x8000)) { Move(-dt, -dt);	}
		else if ((GetAsyncKeyState('W') & 0x8000) && (GetAsyncKeyState('D') & 0x8000)) { Move(-dt, dt);	}
		else if ((GetAsyncKeyState('S') & 0x8000) && (GetAsyncKeyState('A') & 0x8000)) { Move(dt, -dt);	}
		else if ((GetAsyncKeyState('S') & 0x8000) && (GetAsyncKeyState('D') & 0x8000)) { Move(dt, dt);	}
		else if (GetAsyncKeyState('W') & 0x8000) { Move(-dt, 0); }
		else if (GetAsyncKeyState('S') & 0x8000) { Move(dt, 0);	}
		else if (GetAsyncKeyState('A') & 0x8000) { Move(0, -dt);}
		else if (GetAsyncKeyState('D') & 0x8000) { Move(0, dt);	}
		else
			mActionState = ActionState::Idle;
	}
}
#include "Player.h"

Player::Player() : SkinnedObject()
{
	mProperty.name = "Player";

	mObjectType = ObjectType::Player;

	mAnimNames[Anims::attack1] = "attack01";
	mAnimNames[Anims::attack2] = "attack02";
	mAnimNames[Anims::hit] = "damage";
	mAnimNames[Anims::run] = "run";
	mAnimNames[Anims::idle] = "idle";
	mAnimNames[Anims::walk] = "walk";

	mTimer.Reset();
}


Player::~Player()
{
}

void Player::Init(SkinnedMesh * mesh, const SO_InitDesc & info)
{
	mProperty.attackspeed = info.AttackSpeed;
	mProperty.movespeed = info.MoveSpeed;
	mProperty.hp_now = info.Hp;
	mProperty.attakpoint = info.AttackPoint;

	SkinnedObject::Init(mesh, info);
}

void Player::Update(float dt)
{
	ProccessKeyInput(dt);

	if ( (mActionState == ActionState::Attack || mActionState == ActionState::Damage) 
		&& CurrAnimEnd())
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
	mActionState = ActionState::Run;

	float dt = 0.0f;
	if (walk != 0.0f) 
	{
		Walk(walk);
		dt = abs(walk);
	}
	if (strafe != 0.0f)
	{
		Strafe(strafe);
		dt = abs(strafe);
	}

	if (mTimer.TotalTime() > 0.01f)
	{
		CS_Move packet;
		packet.Pos = mPosition;
		packet.Pos.y = Terrain::GetInstance()->GetHeight(mPosition);
		packet.Rot = mRotation;
		packet.MoveSpeed = mProperty.movespeed;
		packet.DeltaTime = dt;
		Packet_Mgr->SendPacket(packet);
		mTimer.Reset();
	}
	else
		mTimer.Tick();
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
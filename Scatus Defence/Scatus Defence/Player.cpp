#include "Player.h"
#include "Sound_Manager.h"
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
	mTimer_s.Reset();
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

void Player::Animate(float dt)
{
	if (mActionState == ActionState::Damage)
		mTimePos += dt*mProperty.movespeed * 2.0f;

	SkinnedObject::Animate(dt);
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

	if (mTimer.TotalTime() > 0.2f)
	{
		Sound_Mgr->Play3DEffect(Sound_p_footstep1, Player::GetInstance()->GetPos().x, Player::GetInstance()->GetPos().y, Player::GetInstance()->GetPos().z);
		CS_Move packet;
		packet.Pos = mPosition;
		packet.Pos.y = Terrain::GetInstance()->GetHeight(mPosition);
		packet.Rot = mRotation;
		packet.ActionState = mActionState;
		packet.MoveSpeed = mProperty.movespeed;
		packet.DeltaTime = dt;

		Packet_Mgr->SendPacket(packet);
		mTimer.Reset();
	}
	else
		mTimer.Tick();

	//if (((mTimer.TotalTime()) / 3.0f) > 1.0f)
	{
		Sound_Mgr->Play3DEffect(Sound_p_footstep1, Player::GetInstance()->GetPos().x, Player::GetInstance()->GetPos().y, Player::GetInstance()->GetPos().z);

	}
	

}

void Player::ProccessKeyInput(float dt)
{
	if (mActionState != ActionState::Attack && mActionState != ActionState::Damage && mActionState != ActionState::Die)
	{
		for (UINT i = 0; i < COLL_OBJ_NUM; ++i)
		{
			XMFLOAT3 cp = Object_Mgr->GetCollisionPos()[i];
			if (MathHelper::DistanceVector(mPosition, Object_Mgr->GetCollisionPos()[i]) < 3.0f)
				CollisionMoving(Object_Mgr->GetCollisionPos()[i], dt);
		}

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
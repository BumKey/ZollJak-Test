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

void Player::Damage(float damage)
{
	//target->SetHP(mTarget_hp + (damage*(1 - (armor*0.06)) / (1 + 0.06*armor)));
	mProperty.hp_now -= damage;

	DEBUG_MSG("ÇÃ·¹ÀÌ¾îÇÇ°Ý, Ã¼·Â : " << target->GetProperty().hp_now);
	Sound_Mgr->Play3DEffect(Sound_impact, Player::GetInstance()->GetPos().x, Player::GetInstance()->GetPos().y, Player::GetInstance()->GetPos().z);
	//	Sound_Mgr->Play3DEffect(Sound_Giant_attack1, GetPos().x, GetPos().y, GetPos().z);
	if (mProperty.hp_now < 200 || Sound_Mgr->hpdown == false)
	{
		Sound_Mgr->hpdown = true;
		Sound_Mgr->Play3DEffect(Sound_p_almostdie, Camera::GetInstance()->GetPosition().x,
			Camera::GetInstance()->GetPosition().y, Camera::GetInstance()->GetPosition().z);
		//target->Die();
		DEBUG_MSG("Å¸°Ù »ç¸Á");
		Time_Mgr->Set_P_HP((mProperty.hp_now));
	}

	if (mProperty.hp_now <= 0)
	{
		//Sound_Mgr->Play3DEffect(Sound_p_die, Player::GetInstance()->GetPos().x, Player::GetInstance()->GetPos().y, Player::GetInstance()->GetPos().z);
		//target->Die();
		DEBUG_MSG("Å¸°Ù »ç¸Á");
	}
	else
		ChangeActionState(ActionState::Damage);

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
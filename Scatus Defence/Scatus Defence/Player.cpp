#include "Player.h"
#include "Sound_Manager.h"
#include "UI_Manager.h"

Player::Player() : SkinnedObject(), mDamage_Timer_flag(false)
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

	mDamage_Timer_flag = false;
	SkinnedObject::Init(mesh, info);
}

void Player::Update(float dt)
{
	ProccessKeyInput(dt);

	if ( (mActionState == ActionState::Attack || mActionState == ActionState::Damage) 
		&& CurrAnimEnd())
		mActionState = ActionState::Idle;

	if (mProperty.hp_now < 200)
	{
		Sound_Mgr->Play3DEffect(Sound_p_almostdie, Camera::GetInstance()->GetPosition().x,
			Camera::GetInstance()->GetPosition().y, Camera::GetInstance()->GetPosition().z);
		UI_Mgr->Active_damage_Screen(true, true);
	}

	if (mProperty.hp_now <= 0)
	{
		//Sound_Mgr->Play3DEffect(Sound_p_die, Player::GetInstance()->GetPos().x, Player::GetInstance()->GetPos().y, Player::GetInstance()->GetPos().z);
		//target->Die();
		DEBUG_MSG("타겟 사망");
	}

	if (mDamage_Timer_flag) // 몬스터가 공격에 성공하면 틱타이머가 돌기시작한다.
	{
		if (!(UI_Mgr->Tick_dmage_Timer()))  // 참을 리턴하면 0.5초가 안지난것 false를 리턴하면 0.5초 지난 것
		{
			UI_Mgr->Active_damage_Screen(false); // 0.5초가 지나면 맞은 화면을 끈다. Timer reset도 함께 된다.
			mDamage_Timer_flag = false;
		}
	}

	SkinnedObject::Update(dt);
}

void Player::Damage(float damage)
{
	SkinnedObject::Damage(damage);

	mDamage_Timer_flag = true;
	UI_Mgr->Active_damage_Screen(true); //Timer reset도 함께된다.
	Time_Mgr->Set_P_HP(mProperty.hp_now);
	Sound_Mgr->Play3DEffect(Sound_impact, Player::GetInstance()->GetPos().x, Player::GetInstance()->GetPos().y, Player::GetInstance()->GetPos().z);
	//	Sound_Mgr->Play3DEffect(Sound_Giant_attack1, GetPos().x, GetPos().y, GetPos().z);

	DEBUG_MSG("플레이어피격, 체력 : " << mProperty.hp_now);
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

		//auto temple = Temple::GetInstance()->GetAABB();
		//if (XNA::IntersectAxisAlignedBoxAxisAlignedBox(&temple, &mAABB))
		//	CollisionMoving(Object_Mgr->GetTemplePos(), dt);

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
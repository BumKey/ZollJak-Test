#include "GameObject.h"
UINT GameObject::GeneratedCount = 0;

GameObject::GameObject(GameMesh* mesh) : mMesh(mesh),
mPosition(0.0f, 0.0f, 0.0f), mScaling(1.0f), mRotation(0.0f, 0.0f, 0.0f),
mRight(1.0f, 0.0f, 0.0f), mUp(0.0f, 1.0f, 0.0f), mCurrLook(0.0f, 0.0f, -1.0f), mPrevLook(0.0f, 0.0f, -1.0f)
{
	XMStoreFloat4x4(&mWorld, XMMatrixIdentity());
	mID = GeneratedCount++;
}


GameObject::~GameObject()
{
}

void  GameObject::printlocation()
{
	
		printf("%s ��ġ ���%f %f", Get_Properties()->name, mPosition.x, mPosition.z);


}

void GameObject::SettingTarget(std::list<GameObject*> Oppenent)
{

	double dis = 10000.0f;

	if (Oppenent.size() == 0)
	{
		current_target_obj = NULL;
	}
	else {


		for (auto it : Oppenent) // Oppenent�� 0 �϶��� ����������
		{
			if (dis > Vec2DDistance(this->GetPos2D(), it->GetPos2D()))
			{
				dis = Vec2DDistance(this->GetPos2D(), it->GetPos2D());
				current_target_obj = it;
			};

		}
	}


}

void GameObject::Move2D(Vector2D direction, float dt)
{
	mPosition.x = mPosition.x + direction.x*Get_Properties()->movespeed*dt;
	mPosition.z = mPosition.z + direction.y*Get_Properties()->movespeed*dt;
}
void GameObject::SetHeading(Vector2D new_heading)
{
	// assert( (new_heading.LengthSq() - 1.0) < 0.00001); // ��ǥ���� �Ÿ��� �ſ������� � ������ ���ٴµ�?? �츮�� ������ ���缭 �����ؼ� �ϴܺ��� 

	m_vHeading = Vec2DNormalize(new_heading);
}
void GameObject::Attack(GameObject * Target)
{
	if (Target != NULL)
	{

		//if(Target->Get_States == type_die)
		//Target = NULL:
		//else()
		int Target_hp = Target->Get_Properties()->hp_now;
		int armor = Target->Get_Properties()->guardpoint;
		int damage = Get_Properties()->attakpoint;

		Target->Get_Properties()->hp_now =
			Target_hp + (damage*(1 - (armor*0.06)) / (1 + 0.06*armor));
		printf("������ �����߽��ϴ�. ����� ü�� : %d \n", Target->Get_Properties()->hp_now);

		if (Target->Get_Properties()->hp_now <= -500)
		{
			printf("����� �̸� : %s\n", Target->Get_Properties()->name);
		}
		if (Target->Get_Properties()->hp_now <= 0)
		{
			Target->SetObj_State(type_die);
			printf("Ÿ�� ���");
			this->current_target_obj = NULL; //Ÿ�� �ʱ�ȭ
			this->SetObj_State(type_idle);

		}

	}
	else
	{
		printf("Ÿ���� �������� �ʾҽ��ϴ�.");

	}
}
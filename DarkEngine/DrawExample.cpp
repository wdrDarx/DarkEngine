#if 1
#define ASYNC
//#define DEBUGMODE
#include "Engine.h"


class P : public Object
{
public:
	vec2d pos;
	vec2d scale;
	BoxCollider* b;
	Sprite* s;
	RigidComp* rc;
	
	P(vec2d p, vec2d se)
	{
		pos = p;
		scale = se;
		
	}
	
	void collide(CollisionSweep col)
	{
		if(col.Other->GetComponent<BoxCollider>(col.Other->Components)->pos.y > b->pos.y)
		rc->collides = false;
		 
	}

	void OnCreate() override
	{
		Object::OnCreate();

		b = new BoxCollider(pos, scale, true);
		s = new Sprite(b, olc::WHITE);
		rc = new RigidComp(b, vec2d(0, 0), true);
		rc->collideDelegate = std::bind(&P::collide, this, std::placeholders::_1);
		rc->Gravity = true;
		rc->optimize = true;
		rc->friction = 0.1f;
		rc->bounce = 0.9f;
		AddComponent(b);
		AddComponent(s);
		AddComponent(rc);		
	}

	
};

class Bomb : public Object
{
public:
	vec2d pos;
	vec2d scale;
	BoxCollider* b;
	Sprite* s;
	RigidComp* rc;

	Bomb(vec2d p)
	{
		pos = p;
		scale = vec2d(20, 20);

	}

	void Explode()
	{
		
		std::vector<BoxCollider*> boo = DEngine::BoxTrace(eng, vec2d(b->pos.x - 150, b->pos.y - 150), vec2d(300, 300), b);
		for (int i = 0; i < boo.size(); ++i)
		{
			if (boo.at(i) != b)
			{
				RigidComp* r = boo.at(i)->parent->GetComponent<RigidComp>(boo.at(i)->parent->Components);
				if (r != nullptr)
				{
					r->collides = true;
					r->vel.y = (pos.y - r->pos.y) * -10;
					r->vel.x = (pos.x - r->pos.x) * -10;
				}
			}
		}
		eng->RemoveObject(this);
	}
	void OnCreate() override
	{
		Object::OnCreate();

		b = new BoxCollider(pos, scale, true);
		s = new Sprite(b, olc::RED);
		rc = new RigidComp(b, vec2d(0, 0), true);
		rc->Gravity = true;
		rc->optimize = true;
		AddComponent(b);
		AddComponent(s);
		AddComponent(rc);

		Delay* d = new Delay(0.5);
		d->DelayDelegate = std::bind(&Bomb::Explode, this);
		eng->CreateObject(d);
	}
};


class Game : public Engine
{
public:

	Object* Ground;
	BoxCollider* GroundC;
	//
	void DrawSquare()
	{
		P* d = new P(vec2d(GetMouseX(), GetMouseY()), vec2d(4, 4));
		CreateObject(d);
	}

	void SpawnBomb()
	{
		Bomb* d = new Bomb(vec2d(GetMouseX(), GetMouseY()));
		CreateObject(d);
	}
	//
	bool OnCreate() override
	{
		//
		Gravity = 800.f;
		Ground = new Object();
		GroundC = new BoxCollider(vec2d(0, 0), vec2d(10, 700), true);
		Ground->AddComponent(GroundC);
		CreateObject(Ground);
		//
		Ground = new Object();
		GroundC = new BoxCollider(vec2d(0, 650), vec2d(700, 10), true);
		Ground->AddComponent(GroundC);
		CreateObject(Ground);
		//
		Ground = new Object();
		GroundC = new BoxCollider(vec2d(700, 0), vec2d(10, 700), true);
		Ground->AddComponent(GroundC);
		CreateObject(Ground);
		return true;
	}

	bool OnUpdate(float ET) override
	{
		
		if (GetMouse(0).bHeld)
		{
			DrawSquare();
			
		}
		if (GetMouse(1).bPressed)
		{
			
			SpawnBomb();

		}
				
		return true;
	}
};


int main()
{
	Game game;
	game.Instanciate(&game);
	game.Construct(700, 700, 1, 1);
	game.Start();

	return 0;
}
#endif

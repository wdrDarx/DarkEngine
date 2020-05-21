#if 1
#include "Engine.h"

class Ground : public Object
{
public:
	using Object::Object;

	BoxCollider* b;

	Ground(vec2d p, vec2d s)
	{
		b = new BoxCollider(p, s, true);
	}


	void OnCreate() override
	{
		Object::OnCreate();
		AddComponent(b);
	}
};



class Player : public Object
{
public:
	using Object::Object;

	BoxCollider* b;
	RigidComp* rc;
	Sprite* s;

	int Score;

	void OnCreate() override
	{
		Object::OnCreate();
		Score = 0;
		b = new BoxCollider(vec2d(10, 10), vec2d(3, 30), true);
		s = new Sprite(b, olc::WHITE);
		rc = new RigidComp(b, vec2d(0, 0), true);
		rc->Gravity = false;
		AddComponent(b);
		AddComponent(s);
		AddComponent(rc);
	}
};

class Goal : public Object
{
public:
	using Object::Object;

	BoxCollider* b;
	Player* Scorer;

	Goal(vec2d p, vec2d s, Player* scorer)
	{
		b = new BoxCollider(p, s, true);
		Scorer = scorer;
	}


	void OnCreate() override
	{
		Object::OnCreate();
		AddComponent(b);
	}
};

class Ball: public Object
{
public:
	using Object::Object;

	BoxCollider* b;
	RigidComp* rc;
	Sprite* s;
	bool over;

	void collide(CollisionSweep col)
	{
		if (dynamic_cast<Goal*>(col.Other) != nullptr)
		{
			dynamic_cast<Goal*>(col.Other)->Scorer->Score++;
			b->pos = vec2d(115, 55);
			rc->vel = vec2d(0, 0);
			over = true;
		}
		else
		if (dynamic_cast<Ground*>(col.Other) != nullptr)
		{
			rc->vel.y *= -1;
		}
		else
		if (dynamic_cast<Player*>(col.Other) != nullptr)
		{
			Player* coll = static_cast<Player*>(col.Other);
			rc->vel.x *= -1;
			if(coll->rc->vel.y > 0)
			rc->vel.y *= -1;
			else
			if (coll->rc->vel.y < 0)
				rc->vel.y *= 1;
		}				
	}

	void OnCreate() override
	{
		Object::OnCreate();
		over = false;
		b = new BoxCollider(vec2d(115,55 ), vec2d(3, 3), true);
		s = new Sprite(b, olc::RED);
		rc = new RigidComp(b, vec2d(0, 0), true);
		rc->Gravity = false;
		rc->collideDelegate = std::bind(&Ball::collide, this, std::placeholders::_1);
		rc->friction = 0;
		AddComponent(b);
		AddComponent(s);
		AddComponent(rc);
	}
};



class Game : public Engine
{
public:

	Player* p1;
	Player* p2;
	Ball* b;
	//
	Ground* Top;
	Ground* Bottom;
	//
	Goal* p1g;
	Goal* p2g;

	vec2d Ballvell;

	void SetBallVelocity()
	{
		b->rc->vel.x = Ballvell.x * ((rand() % 100 < 50) ? -1 : 1);
		b->rc->vel.y = Ballvell.y * ((rand() % 100 < 50) ? -1 : 1);
		
	}
	void reset()
	{
		Delay* d = new Delay(2.f);
		d->DelayDelegate = std::bind(&Game::SetBallVelocity, this);
		CreateObject(d);
	}

	bool OnCreate() override
	{
		Ballvell = vec2d(150, 50);
		//
		p1 = new Player();
		p2 = new Player();
		b = new Ball();
		Top = new Ground(vec2d(0, -10), vec2d(240, 10));
		Bottom = new Ground(vec2d(0, 120), vec2d(240, 10));
		p1g = new Goal(vec2d(-10, 0), vec2d(10, 120), p1);
		p2g = new Goal(vec2d(240, 0), vec2d(10, 120), p2);
		//
		//
		CreateObject(p1);
		CreateObject(p2);
		CreateObject(b);
		CreateObject(Top);
		CreateObject(Bottom);
		CreateObject(p1g);
		CreateObject(p2g);
		//
		p2->b->pos.x = 220.f;
		reset();
		//
		return true;
	}

	bool OnUpdate(float ET) override
	{
		// p1
		p1->rc->vel.y = 0.f;
		p2->rc->vel.y = 0.f;
		if (GetKey(olc::Key::W).bHeld)
		{
			p1->rc->vel.y = -100.f;
		}
		if (GetKey(olc::Key::S).bHeld)
		{
			p1->rc->vel.y = 100.f;
		}
		//p2
		if (GetKey(olc::Key::UP).bHeld)
		{
			p2->rc->vel.y = -100.f;
		}
		if (GetKey(olc::Key::DOWN).bHeld)
		{
			p2->rc->vel.y = 100.f;
		}

		if (b->over == true)
		{
			b->over = false;
			reset();
		}

		return true;
	}
};


int main()
{

	Game game;
	game.Instanciate(&game);
	game.Construct(240, 120, 4, 4);
	game.Start();


	return 0;
}
#endif
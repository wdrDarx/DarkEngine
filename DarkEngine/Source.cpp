#if 0

#include "Engine.h"


class Player : public Object
{
public:
	Player()
	{

	}
	
	BoxCollider* b;
	RigidComp* rc;
	Sprite* s;

	void OnCreate() override
	{
		Object::OnCreate();
		b = new BoxCollider(vec2d(50, 50), vec2d(10, 10), true);
		s = new Sprite(b, olc::RED);
		rc = new RigidComp(b, vec2d(0, 0), true);
		rc->Gravity = true;
		AddComponent(b);
		AddComponent(s);
		AddComponent(rc);
	}
};

class Ground : public Object
{
public:
	vec2d pos;
	vec2d scale;

	Ground(vec2d p, vec2d s)
	{
		pos = p;
		scale = s;
	}

	BoxCollider* b;	
	Sprite* s;

	void OnCreate() override
	{
		Object::OnCreate();
		b = new BoxCollider(pos, scale, true);
		s = new Sprite(b, olc::WHITE);
		AddComponent(b);
		AddComponent(s);		
	}
};


class Game : public Engine
{
public:
	
	
	Player* Pl;
	Ground* Gr;
	

		
	bool OnCreate() override
	{
		// player
		Pl = new Player();
    	CreateObject(Pl);
		// ground
		Gr = new Ground(vec2d(0,150), vec2d(150,10));
		CreateObject(Gr);
		//Ground2 
		Gr = new Ground(vec2d(150, 0), vec2d(10, 150));
		CreateObject(Gr);



		return true;
	}

	bool OnUpdate(float ET) override
	{ 
		if (GetKey(olc::Key::D).bHeld)
		{
			//Pl->rc->vel.x = 60;
			Cam->pos.x += ET * 1;
		}
		if (GetKey(olc::Key::A).bHeld)
		{
		//	Pl->rc->vel.x = -60;
			Cam->pos.x -= ET * 1;
		}

		if (GetKey(olc::Key::W).bHeld)
		{
			//Pl->rc->vel.x = 60;
			Cam->pos.y -= ET * 1;
		}
		if (GetKey(olc::Key::S).bHeld)
		{
			//	Pl->rc->vel.x = -60;
			Cam->pos.y += ET * 1;
		}


		if (GetKey(olc::Key::RIGHT).bHeld)
		{
			Pl->rc->vel.x = 60;
			
		}
		if (GetKey(olc::Key::LEFT).bHeld)
		{
				Pl->rc->vel.x = -60;
			
		}
		else if (!GetKey(olc::Key::RIGHT).bHeld) Pl->rc->vel.x = 0;

		if (GetKey(olc::Key::SPACE).bPressed)
		{
			if (Pl->rc->onGround)
				Pl->rc->vel.y = -150;
		}
				
		return true;
	}
};


int main()
{
	
	Game game;
	game.Instanciate(&game);
	game.Construct(240, 240, 1, 1);	
	game.Start();
	

	return 0;
}
#endif
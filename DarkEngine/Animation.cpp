#if 0

#define ASYNC

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
	Sprite* sky;
	Animator* An;

	void OnCreate() override
	{
		Object::OnCreate();
		
		b = new BoxCollider(vec2d(150, 150), vec2d(150, 200), true);
		s = new Sprite(b, olc::WHITE);
		s->scaleOffset = vec2d(125.f, 0.f);
		s->posOffset = vec2d(0.f, 20.f);
		rc = new RigidComp(b, vec2d(0, 0), true);
		rc->Gravity = true;
		std::vector<std::string> IdleAnims = {  "Dinosaur/Idle (1).png", "Dinosaur/Idle (2).png",
												"Dinosaur/Idle (3).png", "Dinosaur/Idle (4).png",
												"Dinosaur/Idle (5).png", "Dinosaur/Idle (6).png",
												"Dinosaur/Idle (7).png", "Dinosaur/Idle (8).png",
												"Dinosaur/Idle (9).png", "Dinosaur/Idle (10).png" };
		std::vector<std::string> WalkAnims = { "Dinosaur/Walk (1).png", "Dinosaur/Walk (2).png",
												"Dinosaur/Walk (3).png", "Dinosaur/Walk (4).png",
												"Dinosaur/Walk (5).png", "Dinosaur/Walk (6).png",
												"Dinosaur/Walk (7).png", "Dinosaur/Walk (8).png",
												"Dinosaur/Walk (9).png", "Dinosaur/Walk (10).png" };
		std::vector<std::string> JumpAnims = { "Dinosaur/Jump (5).png", "Dinosaur/Jump (5).png" };
													
		std::vector<AnimState*>* Anims = new std::vector<AnimState*>{ new AnimState(Flipbook::AnimFromFiles(s,IdleAnims, 0.1f,true), "Idle"),new AnimState(Flipbook::AnimFromFiles(s,WalkAnims, 0.1f,true), "Walk"), new AnimState(Flipbook::AnimFromFiles(s,JumpAnims, 0.1f,true), "Jump") };
		An = new Animator(s, Anims);
		sky = new Sprite(b);
		sky->spr = new olc::Sprite("Dinosaur/Sky.png");
		sky->scale = vec2d(1500, 1200);
		sky->optimize = false;
		sky->posOffset = vec2d(-500,-500);
		sky->inheritScale = false;
		AddComponent(sky);
		AddComponent(An);
		AddComponent(b);
		AddComponent(s);
		AddComponent(rc);	
		
	}

	void OnUpdate(float ET) override
	{
		Object::OnUpdate(ET);
		eng->Cam->pos.x = b->pos.x + b->scale.x / 2 - 500;
		eng->Cam->pos.y = b->pos.y + b->scale.y / 2 - 500;

		if(abs(rc->vel.x) > 1 && An->currentState->name != "Walk" && rc->onGround)
			An->ChangeStates("Walk");
		else if(An->currentState->name != "Idle" && abs(rc->vel.x) < 1 && rc->onGround)
			An->ChangeStates("Idle");
		else if(!rc->onGround && An->currentState->name != "Jump")
			An->ChangeStates("Jump");

		if (rc->vel.x < -1.f)
			s->Scalar.x = -1.f;
		else if (rc->vel.x > 1.f)
			s->Scalar.x = 1.f;

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
		s = new Sprite(b, olc::DARK_GREEN);
		AddComponent(b);
		AddComponent(s);
	}
};


class Game : public Engine
{
public:


	Player* Pl;
	Ground* Gr;
	Object* Sky;
	PrimitiveComponent* origin;
	
	bool OnCreate() override
	{
		//Sky
		/*Sky = new Object();
		origin = new PrimitiveComponent(vec2d(0,0), vec2d(ScreenWidth(), ScreenHeight()));
		Sprite* skySprite = new Sprite(origin);
		skySprite->spr = new olc::Sprite("Dinosaur/Sky.png");
		CreateObject(Sky);
		Sky->AddComponent(origin);
		Sky->AddComponent(skySprite)*/;
		

		// player
		Pl = new Player();
		CreateObject(Pl);
	
		// ground
		Gr = new Ground(vec2d(0, 500), vec2d(500, 10));
		CreateObject(Gr);
		//Ground2 
		Gr = new Ground(vec2d(500, 0), vec2d(10, 500));
		CreateObject(Gr);
		
		Gravity = 1500.f;
		
		return true;
	}

	bool OnUpdate(float ET) override
	{
		
		if (GetKey(olc::Key::RIGHT).bHeld)
		{
			Pl->rc->vel.x = 300;
		}
		if (GetKey(olc::Key::LEFT).bHeld)
		{
			Pl->rc->vel.x = -300;
			

		}
		else if (!GetKey(olc::Key::RIGHT).bHeld) Pl->rc->vel.x = 0;

		if (GetKey(olc::Key::SPACE).bPressed)
		{
			if (Pl->rc->onGround)
				Pl->rc->vel.y = -800;

			
		}
		
		//origin->pos.x = Pl->b->pos.x - ScreenWidth() / 2;
		//origin->pos.y = Pl->b->pos.y - ScreenHeight() / 2;

		return true;
		
	}
};


int main()
{

	Game game;
	game.Instanciate(&game);
	game.Construct(1000, 1000, 1, 1);
	game.Start();

	return 0;
}
#endif
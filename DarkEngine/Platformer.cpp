#if 1

#define ASYNC
#define DEBUGMODE
#include "Engine.h"

class Sky : public Object
{
public:
	Sprite* s;
	
	void OnCreate() override
	{
		Object::OnCreate();
		s = new Sprite(vec2d((float)eng->ScreenWidth() * -0.5f, (float)eng->ScreenHeight() * -0.5f), vec2d((float)eng->ScreenWidth() *2 , (float)eng->ScreenHeight() *2 ), olc::DARK_GREY);
		s->optimize = false;
		s->CameraTransform = false;
		s->spr = new olc::Sprite("Dinosaur/Sky.png");
		AddComponent(s);
	}

};

class Player : public Object
{
public:
	Player()
	{

	}

	BoxCollider* b;
	RigidComp* rc;
	Sprite* s;	
	Animator* An;

	void OnCreate() override
	{
		Object::OnCreate();

		b = new BoxCollider(vec2d(150, 150), vec2d(40, 100), true);
		s = new Sprite(b, olc::WHITE);
		s->scaleOffset = vec2d(60.f, 0.f);		
		s->posOffset = vec2d(-27.f, 0.f);
		rc = new RigidComp(b, vec2d(0, 0), true);
		rc->Gravity = true;
		std::vector<std::string> IdleAnims = { "Human/tile064.png" };
		std::vector<std::string> WalkAnims = DEngine::SpriteNames("Human/tile0", 32, 39, ".png");
		std::vector<std::string> JumpAnims = { "Human/tile046.png" };

		std::vector<AnimState*>* Anims = new std::vector<AnimState*>{ new AnimState(Flipbook::AnimFromFiles(s,IdleAnims, 0.1f,true), "Idle"),new AnimState(Flipbook::AnimFromFiles(s,WalkAnims, 0.1f,true), "Walk"), new AnimState(Flipbook::AnimFromFiles(s,JumpAnims, 0.1f,true), "Jump") };
		An = new Animator(s, Anims);
		
		AddComponent(An);
		AddComponent(b);
		AddComponent(s);
		AddComponent(rc);

	}

	void OnUpdate(float ET) override
	{
		Object::OnUpdate(ET);
		eng->Cam->pos.x = b->pos.x + b->scale.x / 2 - (float)eng->ScreenWidth() /2;
		eng->Cam->pos.y = b->pos.y + b->scale.y / 2 - (float)eng->ScreenHeight() /2;		
		//State machine
		if (abs(rc->vel.x) > 1 && An->currentState->name != "Walk" && rc->onGround)
			An->ChangeStates("Walk");
		else if (An->currentState->name != "Idle" && abs(rc->vel.x) < 1 && rc->onGround)
			An->ChangeStates("Idle");
		else if (!rc->onGround && An->currentState->name != "Jump")
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
	
	Ground(vec2d p)
	{
		pos = p;		
	}

	BoxCollider* b;
	Sprite* s;

	void OnCreate() override
	{
		Object::OnCreate();
		b = new BoxCollider(pos, vec2d(100,100), true);
		s = new Sprite(b);
		s->spr = new olc::Sprite("DirtBlock.png");
		AddComponent(b);
		AddComponent(s);
	}
};


class Game : public Engine
{
public:


	Player* Pl;
	Ground* Gr;
	Sky* Sk;	

	bool OnCreate() override
	{		

	//sky
	Sk = new Sky();
	CreateObject(Sk);

	// player
	Pl = new Player();
	CreateObject(Pl);

	// ground
	Gr = new Ground(vec2d(0, 500));
	CreateObject(Gr);

	//Ground2 
	Gr = new Ground(vec2d(100, 500));
	CreateObject(Gr);
	Gr = new Ground(vec2d(200, 500));
	CreateObject(Gr);
	Gr = new Ground(vec2d(300, 500));
	CreateObject(Gr);
	Gr = new Ground(vec2d(400, 500));
	CreateObject(Gr);
	Gr = new Ground(vec2d(500, 500));
	CreateObject(Gr);
	Gr = new Ground(vec2d(600, 500));
	CreateObject(Gr);
	Gr = new Ground(vec2d(500, 400));
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
		if (GetMouse(0).bPressed)
		{
			DEngine::RayTrace(this, vec2d(Pl->b->pos.x, Pl->b->pos.y + Pl->b->scale.y + 10), vec2d(Pl->b->pos.x, Pl->b->pos.y  +Pl->b->scale.y + 100));
		}
		
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
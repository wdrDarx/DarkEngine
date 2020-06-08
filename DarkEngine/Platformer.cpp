#if 1

#define ASYNC

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
		DEngine::SetLayer(this, 0);
	}

	void OnUpdate(float ET) override
	{
		Object::OnUpdate(ET);
		//s->pos = vec2d((float)eng->ScreenWidth() * -0.5f, (float)eng->ScreenHeight() * -0.5f);
		LOG(s->scale.x);
		s->scale = vec2d((float)eng->ScreenWidth() * 2, (float)eng->ScreenHeight() * 2);
	}

};

class Bomb : public Object
{
public:
	vec2d p;
	vec2d vel;
	Bomb(vec2d pp, vec2d v)
	{
		p = pp;
		vel = v;
	}

	BoxCollider* b;
	Sprite* s;
	RigidComp* rc;


	void explode()
	{
		b->scale = vec2d(300, 300);
		b->pos.x -= 150;
		b->pos.y -= 150;
		for (int i = 0; i < b->BoundingColliders().size(); i++)
		{
			if(b->BoundingColliders().at(i) != b)
			eng->RemoveObject(b->BoundingColliders().at(i)->parent);
		}
		eng->RemoveObject(this);
	}
	void OnCreate() override
	{
		b = new BoxCollider(p, vec2d(100, 100), true);
		s = new Sprite(b);
		s->spr = new olc::Sprite("Bomb.png");
		rc = new RigidComp(b, vel, true);
		rc->Gravity = true;
	
		AddComponent(b);
		AddComponent(s);
		AddComponent(rc);
		Delay* d = new Delay(1.f);
		d->DelayDelegate = std::bind(&Bomb::explode, this);
		eng->CreateObject(d);
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

		b = new BoxCollider(vec2d(0, 0), vec2d(40, 95), true);
		s = new Sprite(b, olc::WHITE);
		s->scaleOffset = vec2d(60.f, 5.f);		
		s->posOffset = vec2d(-30.f, -1.f);
		rc = new RigidComp(b, vec2d(0, 0), true);
		rc->Gravity = true;
		rc->GroundLength = 10;
		std::vector<std::string> IdleAnims = { "Human/tile064.png" };
		std::vector<std::string> WalkAnims = DEngine::SpriteNames("Human/tile0", 32, 39, ".png");
		std::vector<std::string> JumpAnims = { "Human/tile046.png" };

		std::vector<AnimState*>* Anims = new std::vector<AnimState*>{ new AnimState(Flipbook::AnimFromFiles(s,IdleAnims, 0.1f,true), "Idle"),new AnimState(Flipbook::AnimFromFiles(s,WalkAnims, 0.1f,true), "Walk"), new AnimState(Flipbook::AnimFromFiles(s,JumpAnims, 0.1f,true), "Jump") };
		An = new Animator(s, Anims);
		DEngine::SetLayer(this, 2);

		AddComponent(An);
		AddComponent(b);
		AddComponent(s);
		AddComponent(rc);

	}

	void OnUpdate(float ET) override
	{
		Object::OnUpdate(ET);
		eng->Cam->pos.x = (b->pos.x + b->scale.x / 2 - (float)eng->ScreenWidth() / 2) + eng->Cam->offset.x;
		eng->Cam->pos.y = (b->pos.y + b->scale.y / 2 - (float)eng->ScreenHeight() /2) + eng->Cam->offset.y;
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
		s->Dc = DEngine::GetTexture(eng, "DirtBlock.png");
		DEngine::SetLayer(this, 1);
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



	void makeGround(vec2d pos)
	{
		Gr = new Ground(pos);
		CreateObject(Gr);
	}
	bool OnCreate() override
	{		

	//sky
	Sk = new Sky();
	CreateObject(Sk);

	// player
	Pl = new Player();
	CreateObject(Pl);

	// ground
	makeGround({ 0, 500 });
		
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
			vec2d v = DEngine::ScreenToWorld(this, vec2d((float)GetMouseX(), (float)GetMouseY()));
			RayHit check = DEngine::PointCollisionCheck(this, v);
			if (check.Hit)
				RemoveObject(check.HitObject);
			
		}
		if (GetMouse(2).bPressed)
		{
			Bomb* b = new Bomb(vec2d(Pl->b->pos.x + Pl->b->scale.x / 2 + (100 * Pl->s->Scalar.x * 2), Pl->b->pos.y - 100), vec2d(Pl->b->pos.x + Pl->b->scale.x / 2 + Pl->b->scale.x / 2 + 100 * Pl->s->Scalar.x, -200));
			CreateObject(b);
		}
		if (GetMouse(1).bPressed)
		{
			vec2d v = DEngine::ScreenToWorld(this, vec2d((float)GetMouseX(), (float)GetMouseY()));
			vec2d v2 = DEngine::Snap(v, vec2d(100, 100));
			BoxCollider* check = new BoxCollider(v2, vec2d(100, 100), true);
			Objects.at(0)->AddComponent(check);
			
			if (!check->CollisionCheck(check).collides)
			{				
				Ground* G = new Ground(v2);
				CreateObject(G);
			}
			Objects.at(0)->RemoveComponent(check);
			
		}
		if (GetKey(olc::Key::UP).bHeld)
		{
			Cam->zoom *= 1 + 0.6f * ET;
		}
		if (GetKey(olc::Key::DOWN).bHeld)
		{
			Cam->zoom *= 1 - 0.6f * ET;
			
		}
		if (GetKey(olc::Key::W).bHeld)
		{
			Cam->offset.y -= 400.f * Cam->zoom * ET;

		}
		if (GetKey(olc::Key::S).bHeld)
		{
			Cam->offset.y += 400.f * Cam->zoom * ET;
		}
		if (GetKey(olc::Key::A).bHeld)
		{
			Cam->offset.x -= 400.f * Cam->zoom * ET;

		}
		if (GetKey(olc::Key::D).bHeld)
		{
			Cam->offset.x += 400.f * Cam->zoom * ET;
		}
		if (GetKey(olc::Key::O).bHeld)
		{
			Pl->s->angle += 100.f * ET;

		}
		if (GetKey(olc::Key::P).bHeld)
		{
			Pl->s->angle -= 100.f * ET;
			std::cout << Pl->s->angle << std::endl;		
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
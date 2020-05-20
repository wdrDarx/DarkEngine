#if 0
#include "Engine.h"
#include <iostream>
#include <windows.h>

class Player : public Object
{
public:
	Player()
	{

	}
	int Score;
	BoxCollider* b;
	
	Sprite* s;

	void OnCreate() override
	{
		Score = 0;
		Object::OnCreate();
		b = new BoxCollider(vec2d(50, 50), vec2d(1, 1), true);
		s = new Sprite(b, olc::RED);				
		AddComponent(b);
		AddComponent(s);	
	}
};

class Square : public Object
{
public:
	vec2d pos;
	int Score;

	Square(vec2d p)
	{
		pos = p;
		
	}

	BoxCollider* b;
	Sprite* bb;
	Sprite* s;

	void OnCreate() override
	{
		
		Score = 30;
		Object::OnCreate();
		b = new BoxCollider(pos, vec2d(20,20), true);
		bb = new  Sprite(vec2d(pos.x - 10, pos.y - 10), vec2d(40,40), olc::BLUE);
		s = new Sprite(b, olc::WHITE);
		AddComponent(bb);
		AddComponent(b);
		AddComponent(s);
		
	}

	void OnUpdate(float ET) override
	{
		Object::OnUpdate(ET);
		bb->scale.x -= ET * 10.f;
		bb->scale.y -= ET * 10.f;
		bb->pos.x += ET * 5.f;
		bb->pos.y += ET * 5.f;
		
		if (bb->scale.x <= 18)
		{
			eng->RemoveObject(this);
		}
	}

};


class Game : public Engine
{
public:


	Player* Pl;
	float TimeToSpawn;
	float Time;
	void CreateSquareRandom()
	{
		Square* Gr = new Square(vec2d(rand() % 220, rand() % 220));
		CreateObject(Gr);
	}



	bool OnCreate() override
	{
		TimeToSpawn = 1.0f;
		// player
		Pl = new Player();
		CreateObject(Pl);		
		
		return true;
	}

	bool OnUpdate(float ET) override
	{
		
		// time
		Time += ET;
		if (Time >= TimeToSpawn)
		{
			Time = 0.0f;
			Square* Gr = new Square(vec2d(rand() % 220, rand() % 220));
			CreateObject(Gr);
		}
		// movement
		Pl->b->pos.x = GetMouseX();
		Pl->b->pos.y = GetMouseY();
		//
		if (GetMouse(0).bPressed)
		{		
			Object* col = Pl->b->CollisionCheck(Pl->b).Other;
			if (Square* s = dynamic_cast<Square*>(col))
			{
				Pl->Score += s->Score;
				RemoveObject(s);
			}				
			
		}
		// print score
		//system("cls");
		//std::cout << "Score: " << Pl->Score << std::endl;


		return true;
	}
};


int main()
{

	Game game;
	game.Instanciate(&game);
	game.Construct(240, 240, 4, 4);
	game.Start();


	return 0;
}
#endif
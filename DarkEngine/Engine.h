#include <iostream>
#include <chrono>
#include <vector>
#include <typeinfo>
#include <windows.h>
#include <math.h>
#include <cstdint>
#include <algorithm>
#include <iomanip>
#include <random>
#include <functional>
#include <array>
#include <typeinfo>
#include <future>
#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

template< typename TContainer >
static bool EraseFromUnorderedByIndex(TContainer& inContainer, size_t inIndex)
{
	if (inIndex < inContainer.size())
	{
		if (inIndex != inContainer.size() - 1)
			inContainer[inIndex] = inContainer.back();
		inContainer.pop_back();
		return true;
	}
	return false;
}

struct vec2d
{
public:
	float x, y;

	vec2d()
	{

	}

	vec2d(float ix, float iy)
	{
		x = ix;
		y = iy;
	};


};
class Object;
class Component
{
public:
	Object* parent;

	Component()
	{
		
	}

	virtual void onUpdate(float ET)
	{

	}
	virtual void onAdd()
	{

	}
	virtual void OnDelete()
	{

	}
	~Component()
	{
		OnDelete();
	}
};

class Engine;
class Object
{
public:

	Engine* eng;
	std::vector<Component*> Components;

	Object()
	{

	}

	virtual void OnCreate()
	{
		for (int i = 0; i < Components.size(); ++i)
		{
			Components.at(i)->onAdd();
		}
	}

	virtual void  OnUpdate(float ET)
	{
		if (eng != nullptr)
		{		
			for (int i = 0; i < Components.size(); ++i)
			{
				Components.at(i)->onUpdate(ET);
			}
		}
	}

	virtual void OnDelete()
	{
		
			for (int i = 0; i < Components.size(); ++i)
			{
				RemoveComponent(Components.at(i));
;			}
		

	}

	void AddComponent(Component* c)
	{
		c->parent = this;
		Components.push_back(c);
		c->onAdd();
	}

	void RemoveComponent(Component* c)
	{
		std::vector<Component*>::iterator it = std::find(Components.begin(), Components.end(), c);
		int index = std::distance(Components.begin(), it);
		EraseFromUnorderedByIndex(Components, index);
		delete c;
	}

	template<typename T, typename C> auto* GetComponent(C& container) {
		for (auto& item : container) {
			auto p = dynamic_cast<T*>(item);
			if (p != nullptr) return p;
		}

		return static_cast<T*>(nullptr);
	}


	~Object()
	{
		OnDelete();
	}

};

struct CollisionSweep
{
public:
	vec2d Correction;
	bool collides;
	Object* Other;

	CollisionSweep(vec2d c, bool col)
	{
		Other = nullptr;
		Correction = c;
		collides = col;
	}

	CollisionSweep()
	{
		Correction = vec2d(0, 0);
		collides = false;
		Other = nullptr;
	}
};



class PrimitiveComponent : public Component
{
public:
	PrimitiveComponent* TransformParent;
	vec2d pos;
	vec2d scale;

	PrimitiveComponent( PrimitiveComponent* TP)
	{
		TransformParent = TP;
		pos = TP->pos;
		scale = TP->scale;
	}

	PrimitiveComponent( vec2d po, vec2d s) 
	{		
		pos = po;
		scale = s;
	}

	virtual void onUpdate(float ET) override
	{
		//Super
		Component::onUpdate(ET);
		//
		if (TransformParent != nullptr)
		{
			pos.x = TransformParent->pos.x;
			pos.y = TransformParent->pos.y;
			scale.x = TransformParent->scale.x;
			scale.y = TransformParent->scale.y;
		}
	}
};

class Engine : public olc::PixelGameEngine
{
public:
	Engine* Inst;

	Engine()
	{
		sAppName = "Engine";
		
	}
	float Gravity = 200.0f;
	std::vector<Object*> Objects;
	bool ClearOnFrame = true;


public:
	


	void Instanciate(Engine* e)
	{
		Inst = e;
	}
	bool OnUserCreate() override
	{
		OnCreate();
		return true;
	}

public:
	
	virtual bool OnCreate()
	{

		return true;
	}

	virtual bool OnUpdate(float ET)
	{

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{

		if (ClearOnFrame)
			FillRect(0, 0, ScreenWidth(), ScreenHeight(), olc::BLACK);
		//int s = sizeof(bounds) / sizeof(vec2d);



		for (int i = 0; i < Objects.size(); i++)
		{
			Objects.at(i)->OnUpdate(fElapsedTime);
		}

		OnUpdate(fElapsedTime);

		return true;
	}

	void CreateObject(Object * newObject)
	{
		newObject->eng = Inst;
		newObject->OnCreate();
		Objects.push_back(newObject);
	}
	void RemoveObject(Object * d)
	{
		std::vector<Object*>::iterator it = std::find(Objects.begin(), Objects.end(), d);
		int index = std::distance(Objects.begin(), it);
		EraseFromUnorderedByIndex(Objects, index);
		delete d;
	}

private:

};

class Delay : public Object
{
public:
	
	float Time;
	float SetTime;
	Delay(float t)
	{
		
		SetTime = t;
	}
	virtual void fire()
	{
		if(DelayDelegate != nullptr)
		DelayDelegate();
	}
	void OnUpdate(float ET) override
	{
		Object::OnUpdate(ET);
		Time += ET;
		if (Time >= SetTime)
		{
			fire();
			eng->RemoveObject(this);
		}
	}
	std::function<void()> DelayDelegate;
	
};


class Sprite : public PrimitiveComponent
{

public:	
	olc::Pixel color;
	bool Render;

	Sprite(vec2d Pos, vec2d size, olc::Pixel col = olc::WHITE) : PrimitiveComponent(Pos,size)
	{				
		color = col;
		Render = true;
	}

	Sprite( PrimitiveComponent* tp, olc::Pixel col = olc::WHITE) : PrimitiveComponent(tp)
	{
		color = col;
		Render = true;
	}



	void onUpdate(float ET) override
	{
		PrimitiveComponent::onUpdate(ET);
		if (parent->eng != nullptr)
		{
			if (Render)
			{
				parent->eng->FillRect(pos.x, pos.y, scale.x, scale.y, color);
			}

			
		}
	}

};

class BoxCollider : public PrimitiveComponent
{
public:
	bool Collides;

	BoxCollider(vec2d Pos, vec2d size, bool col) : PrimitiveComponent(Pos, size)
	{
		Collides = col;
	}

	

	std::vector<BoxCollider*> BoundingColliders()
	{
		std::vector<Object*> scan = parent->eng->Objects;
		std::vector<BoxCollider*> Bounders;
		for (int i = 0; i < scan.size(); ++i)
		{
			if (scan.at(i)->GetComponent<BoxCollider>(scan.at(i)->Components) != nullptr)
			{
				BoxCollider* check = scan.at(i)->GetComponent<BoxCollider>(scan.at(i)->Components);
				if (check != this)
				{
					if (pos.x < check->pos.x + check->scale.x &&
						pos.x + scale.x > check->pos.x &&
						pos.y < check->pos.y + check->scale.y &&
						pos.y + scale.y > check->pos.y) Bounders.push_back(check);
				}
			}
		}
		return Bounders;
	}

	bool IsColliding(BoxCollider* caller, BoxCollider* check)
	{
		if (caller->pos.x < check->pos.x + check->scale.x - 1 &&
			caller->pos.x + caller->scale.x > check->pos.x + 1 &&
			caller->pos.y < check->pos.y + check->scale.y - 1 &&
			caller->pos.y + caller->scale.y > check->pos.y + 1 && caller->Collides && check->Collides) return true;
		else return false;
	}

	CollisionSweep CollisionCheck(BoxCollider* s)
	{
		CollisionSweep col;
		std::vector<BoxCollider*> checks = s->BoundingColliders();
		if (checks.size() > 0)
		{
			for (int i = 0; i < checks.size(); i++)
			{
				BoxCollider* check = checks.at(i);
				if (check != s)
				{
					if (IsColliding(s, check))
					{
						col.collides = true;
						col.Other = check->parent;
					}

				}
			}
		}
		return col;
	}
};


class RigidComp : public PrimitiveComponent
{
public:
	
	bool collides;
	bool Gravity;
	vec2d vel;
	bool onGround;
	float friction;
	bool optimize;
	BoxCollider* Box;




	RigidComp(BoxCollider* bc, vec2d Vel, bool col) : PrimitiveComponent(bc)
	{
		Box = bc;
		vel = Vel;
		collides = col;		
		onGround = false;
		friction = 1;
		optimize = false;
		
	}
	

	virtual void OnCollide(CollisionSweep col)
	{
		if(collideDelegate != nullptr)
		collideDelegate(col);		
	}

	void onUpdate(float ET) override
	{
		PrimitiveComponent::onUpdate(ET);
		if (collides)
		{
			bool calculate = true;
			if (optimize)
			{
				if (Box->pos.x > 0 && Box->pos.x < parent->eng->ScreenWidth() &&
					Box->pos.y > 0 && Box->pos.y < parent->eng->ScreenHeight()) calculate = true;
				else
				{
					calculate = false; parent->eng->RemoveObject(parent);
				}

				
			}
			if (calculate)
			{

				//std::cout << "update" << std::endl;
				Box->pos.x += vel.x * ET;
				CollisionSweep c1 = Box->CollisionCheck(Box);
				if (c1.collides)
				{
					Box->pos.x -= vel.x * ET;
					vel.x -= vel.x * ET;
					OnCollide(c1);
				};

				Box->pos.y += vel.y * ET;
				 c1 = Box->CollisionCheck(Box);
				if (c1.collides)
				{
					Box->pos.y -= vel.y * ET;
					vel.y -= vel.y * ET;
					onGround = true;
					OnCollide(c1);
				}
				else onGround = false;

				if (Gravity) vel.y += parent->eng->Gravity * ET;


				vel.x -= friction * vel.x * ET;
				vel.y -= friction * vel.y * ET;

			}
		}

	}
	// bind like this -> rc->collideDelegate = std::bind(&P::collide, this, std::placeholders::_1);
	std::function<void(CollisionSweep)> collideDelegate;
};












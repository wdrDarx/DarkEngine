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
#include <thread>
#include <mutex>
#include <typeinfo>
#include <future>
#define OLC_PGE_APPLICATION
#define ASYNC 0
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

class Camera : public Object
{
public:
	vec2d pos;
	Camera(vec2d p)
	{
		pos = p;
	}
};




class Engine : public olc::PixelGameEngine
{
public:
	Engine* Inst;
	Camera* Cam;

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
		Cam = new Camera(vec2d(0, 0));
		OnCreate();
		return true;
	}

public:
	
	static void EngineUpdate(bool ClearOnFrame, Engine* eng, float fElapsedTime)
	{
		if (ClearOnFrame)
			eng->FillRect(0, 0, eng->ScreenWidth(), eng->ScreenHeight(), olc::BLACK);
		//int s = sizeof(bounds) / sizeof(vec2d);



		for (int i = 0; i < eng->Objects.size(); i++)
		{
			eng->Objects.at(i)->OnUpdate(fElapsedTime);
		}

		eng->OnUpdate(fElapsedTime);
	}

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
		std::future<void> update = std::async(std::launch::async, EngineUpdate, ClearOnFrame, this, fElapsedTime);

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
	bool loop = false;
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
			if (!loop)
				eng->RemoveObject(this);
			else
				Time = 0.f;
		}
	}
	std::function<void()> DelayDelegate;
	
};

class PrimitiveComponent : public Component
{
public:
	PrimitiveComponent* TransformParent;
	vec2d pos;
	vec2d scale;
	bool CameraTransform;

	PrimitiveComponent(PrimitiveComponent* TP)
	{
		TransformParent = TP;
		pos = TP->pos;
		scale = TP->scale;
	}

	PrimitiveComponent(vec2d po, vec2d s)
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
		if (TransformParent != nullptr)
		{
			if (!TransformParent->CameraTransform && CameraTransform)
			{
				pos.x = pos.x - parent->eng->Cam->pos.x;
				pos.y = pos.y - parent->eng->Cam->pos.y;
			}
		}
		else if (CameraTransform)
		{
			pos.x = pos.x - parent->eng->Cam->pos.x;
			pos.y = pos.y - parent->eng->Cam->pos.y;
		}
	}
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
		CameraTransform = true;
	}

	Sprite( PrimitiveComponent* tp, olc::Pixel col = olc::WHITE) : PrimitiveComponent(tp)
	{
		color = col;
		Render = true;
		CameraTransform = true;
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
		CameraTransform = true;
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
				if (check != this && check != nullptr)
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
	
	//static std::mutex colMutex;

	virtual void OnCollide(CollisionSweep col)
	{
		if(collideDelegate != nullptr)
		collideDelegate(col);		
		//collisionChecks.clear();
	}

	static void CollisionCheck(float ET, RigidComp* rc)
	{
		//std::lock_guard<std::mutex> lock(colMutex);

		BoxCollider* Box = rc->Box;
		
		//std::lock_guard<std::mutex> guard(colMutex);

		Box->pos.x += rc->vel.x * ET;
		CollisionSweep c1 = Box->CollisionCheck(Box);
		if (c1.collides)
		{
			Box->pos.x -= rc->vel.x * ET;
			rc->vel.x -= rc->vel.x * ET;
			rc->OnCollide(c1);
		};

		Box->pos.y += rc->vel.y * ET;
		c1 = Box->CollisionCheck(Box);
		if (c1.collides)
		{
			Box->pos.y -= rc->vel.y * ET;
			rc->vel.y -= rc->vel.y * ET;
			rc->onGround = true;
			rc->OnCollide(c1);
		}
		else rc->onGround = false;

		if (rc->Gravity) rc->vel.y += rc->parent->eng->Gravity * ET;


		rc->vel.x -= rc->friction * rc->vel.x * ET;
		rc->vel.y -= rc->friction * rc->vel.y * ET;
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
				//if (abs(vel.x) < 0.1f && abs(vel.y) < 0.1f && !onGround) calculate = false;

				
			}
			if (calculate)
			{

#ifdef ASYNC
				std::future<void> col = std::async(std::launch::async, CollisionCheck, ET, this);
				//std::thread t1(&RigidComp::CollisionCheck, ET, this);
				//t1.join();
#else
				CollisionCheck(ET,this);
#endif

			}
		}
		
	}
	// bind like this -> rc->collideDelegate = std::bind(&class::functionName, this, std::placeholders::_1);
	std::function<void(CollisionSweep)> collideDelegate;
	//std::vector<std::future<void>> collisionChecks;
};












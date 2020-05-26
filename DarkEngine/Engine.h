
#include <iostream>
#include <chrono>
#include <vector>
#include <typeinfo>
#include <math.h>
#include <cstdint>
#include <algorithm>
#include <iomanip>
#include <random>
#include <functional>
#include <array>
#include <thread>
#include<cmath>
#include <mutex>
#include <typeinfo>
#include <future>
#define OLC_PGE_APPLICATION
#define OLC_GFX_OPENGL10
constexpr float BLANKSIZE = 0.00092592592f;
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
		x = 0.f;
		y = 0.f;
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
	float zoom;
	Camera(vec2d p)
	{
		pos = p;
		zoom = 1.f;
	}
};

struct RayHit
{
public:
	vec2d HitPos;
	Object* HitObject;

	RayHit(vec2d p, Object* o)
	{
		HitPos = p;
		HitObject = o;
	}


};



class Engine : public olc::PixelGameEngine
{
public:
	Engine* Inst;
	Camera* Cam;
	olc::Decal* Blank;
	
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
		Blank = new olc::Decal(new olc::Sprite("Blank.png"));
		OnCreate();
		return true;
	}

public:
	
	static void EngineUpdate(bool ClearOnFrame, Engine* eng, float fElapsedTime)
	{
		
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
#if defined(ASYNC)
		std::future<void> update = std::async(std::launch::async, EngineUpdate, ClearOnFrame, this, fElapsedTime);
#else
		EngineUpdate(ClearOnFrame, this, fElapsedTime);
#endif

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

	static void DelayUpdate(Delay* d, float ET)
	{
		d->Time += ET;
		if (d->Time >= d->SetTime)
		{
			d->fire();
			if (!d->loop)
				d->eng->RemoveObject(d);
			else
				d->Time = 0.f;
		}
	}
	void OnUpdate(float ET) override
	{
		Object::OnUpdate(ET);
		std::future<void> tick = std::async(std::launch::async, DelayUpdate, this, ET);
	}
	std::function<void()> DelayDelegate;
	
};

class PrimitiveComponent : public Component
{
public:
	PrimitiveComponent* TransformParent;
	vec2d pos;
	vec2d scale;
	vec2d posOffset;
	vec2d scaleOffset;
	vec2d ScreenPos;
	vec2d ScreenScale;
	bool CameraTransform;
	bool inheritPos;
	bool inheritScale;

	PrimitiveComponent(PrimitiveComponent* TP)
	{
		TransformParent = TP;
		pos = TP->pos;
		scale = TP->scale;
		posOffset = vec2d();
		scaleOffset = vec2d();
		CameraTransform = true;
		inheritPos = true;
		inheritScale = true;
		ScreenPos = TP->pos;
		scale = TP->scale;
	}

	PrimitiveComponent(vec2d po, vec2d s)
	{
		pos = po;
		scale = s;
		posOffset = vec2d();
		scaleOffset = vec2d();
		CameraTransform = true;
		inheritPos = true;
		inheritScale = true;
		ScreenPos = po;
		ScreenScale = s;
	}

	virtual void onUpdate(float ET) override
	{
		//Super
		Component::onUpdate(ET);
		//
		if (TransformParent != nullptr)
		{
			if (inheritPos)
			{
				pos.x = TransformParent->pos.x;
				pos.y = TransformParent->pos.y;
			}
			if (inheritScale)
			{
				scale.x = TransformParent->scale.x;
				scale.y = TransformParent->scale.y;
			}
			
		}
		//
		if (TransformParent != nullptr)
		{
			pos.x += posOffset.x;
			pos.y += posOffset.y;
			scale.x += scaleOffset.x;
			scale.y += scaleOffset.y;
			
		}
		//
		ScreenPos = pos;
		ScreenScale = scale;
	
		if (CameraTransform)
		{
			ScreenScale.x *= parent->eng->Cam->zoom;
			ScreenScale.y *= parent->eng->Cam->zoom;
			ScreenPos.x = parent->eng->Cam->zoom * (ScreenPos.x - parent->eng->Cam->pos.x);
			ScreenPos.y = parent->eng->Cam->zoom * (ScreenPos.y - parent->eng->Cam->pos.y);
		}
				
	}
};

class Sprite : public PrimitiveComponent
{

public:	
	olc::Pixel color;
	olc::Sprite* spr;
	olc::Decal* Dc;
	olc::Decal* Debug;
	std::array<olc::vf2d, 4> Points;
	
	vec2d Scalar;
	bool Render;
	bool optimize;


	Sprite(vec2d Pos, vec2d size, olc::Pixel col = olc::WHITE) : PrimitiveComponent(Pos,size)
	{				
		color = col;
		Render = true;
		CameraTransform = true;
		Scalar = vec2d(1.f, 1.f);	
		optimize = true;
		
	}

	Sprite( PrimitiveComponent* tp, olc::Pixel col = olc::WHITE) : PrimitiveComponent(tp)
	{
		color = col;
		Render = true;
		CameraTransform = true;
		Scalar = vec2d(1.f, 1.f);	
		optimize = true;
		
	}

	void onAdd() override
	{
		PrimitiveComponent::onAdd();
		
		if (spr != nullptr)
			Dc = new olc::Decal(spr);
		else
		{		
			Dc = parent->eng->Blank;
		}	
		Debug = parent->eng->Blank;
		
	}


	void onUpdate(float ET) override
	{
		PrimitiveComponent::onUpdate(ET);
		if (parent->eng != nullptr)
		{
			if((ScreenPos.x > 0 || ScreenPos.x + ScreenScale.x < (float)parent->eng->ScreenWidth() && ScreenPos.y > 0 || ScreenPos.y + ScreenScale.y < (float)parent->eng->ScreenHeight()) || !optimize)
			if (Render)
			{
				
				if (Dc == nullptr || Dc->sprite == nullptr)
				{
				}
				else
				{
					if (Dc->sprite != nullptr)
					{
						Points[0] = olc::vf2d(ScreenPos.x + ((1 - Scalar.x) / 2 * ScreenScale.x), ScreenPos.y + ((1 - Scalar.y) / 2) * ScreenScale.y);
						Points[1] = olc::vf2d(ScreenPos.x + ((1 - Scalar.x) / 2 * ScreenScale.x), ScreenPos.y + (ScreenScale.y * (Scalar.y + 1) / 2));
						Points[2] = olc::vf2d(ScreenPos.x + (ScreenScale.x * (Scalar.x + 1) / 2), ScreenPos.y + (ScreenScale.y * (Scalar.y + 1) / 2));
						Points[3] = olc::vf2d(ScreenPos.x + (ScreenScale.x * (Scalar.x + 1) / 2), ScreenPos.y + ((1 - Scalar.y) / 2) * ScreenScale.y);

						parent->eng->DrawWarpedDecal(Dc, Points, color);
#if defined(DEBUGMODE)
						parent->eng->DrawDecal(Points[0], Debug, { BLANKSIZE * ScreenScale.x / scale.x  * 3,  BLANKSIZE * ScreenScale.y / scale.y * 3 }, olc::YELLOW);
						parent->eng->DrawDecal(Points[1], Debug, { BLANKSIZE * ScreenScale.x / scale.x * 3,  BLANKSIZE * ScreenScale.y / scale.y * 3 }, olc::YELLOW);
						parent->eng->DrawDecal(Points[2], Debug, { BLANKSIZE * ScreenScale.x / scale.x * 3,  BLANKSIZE * ScreenScale.y / scale.y * 3 }, olc::YELLOW);
						parent->eng->DrawDecal(Points[3], Debug, { BLANKSIZE * ScreenScale.x / scale.x * 3,  BLANKSIZE * ScreenScale.y / scale.y * 3 }, olc::YELLOW);
#endif
					}
				}				
			}			
		}
	}

};

class Flipbook : public Component
{
public:
	std::vector<olc::Decal*> Frames;
	int index;
	float Changetime;
	bool loop;
	bool play;
	
	Sprite* ref;

	Flipbook(Sprite* r, std::vector<olc::Decal*> F, float ct, bool l) : Component()
	{
		Frames = F;
		Changetime = ct;
		loop = l;
		index = 0;
		ref = r;
		play = true;
		
	}

	static Flipbook* AnimFromFiles(Sprite* spr, std::vector<std::string> files, float ct, bool l)
	{
		std::vector<olc::Decal*> Frames;
		for (int i = 0; i < files.size(); ++i)
		{
			olc::Decal* d = new olc::Decal(new olc::Sprite(files.at(i)));
			Frames.push_back(d);
		}
		Flipbook* f = new Flipbook(spr, Frames, ct, l);
		return f;
	}

	void changeFrames()
	{
		index++;
		if (index > Frames.size() - 1)
		{
			if (loop) index = 0;
			else
				parent->eng->RemoveObject(d);
		}
		
		if (play)
		{								
			ref->Dc = Frames.at(index);
			ref->Dc->Update();						
		}
		
	}

	static void playFrame(Flipbook* f)
	{
		if (f->Frames.size() > 0 && f->ref != nullptr)
		{
			//f->ref->Dc = &f->Frames.at(f->index);

			f->d = new Delay(f->Changetime);
			f->d->DelayDelegate = std::bind(&Flipbook::changeFrames, f);
			f->d->loop = f->loop;
			f->parent->eng->CreateObject(f->d);
		}
	}

	void onAdd() override
	{
		Component::onAdd();
		std::future<void> frame = std::async(std::launch::async, playFrame, this);
	}

private: 
		Delay* d;
};

struct AnimState
{
	std::string name;
	Flipbook* State;

	AnimState(Flipbook* f, std::string n)
	{
		State = f;
		name = n;
	}
};

class Animator : public Component
{
public:
	Sprite* target;
	std::vector<AnimState*>* States;
	AnimState* currentState;
	Animator(Sprite* s, std::vector<AnimState*>* S)
	{
		target = s;
		States = S;		
	}

	void ChangeStates(std::string name)
	{
		for (int i = 0; i < States->size(); ++i)
		{
			if (States->at(i)->name == name)
			{
				if (States->at(i)->State->parent == nullptr)
					parent->AddComponent(States->at(i)->State);
				States->at(i)->State->ref = target;
				States->at(i)->State->play = true;
				if (!States->at(i)->State->loop)
					States->at(i)->State->playFrame(States->at(i)->State);
				currentState = States->at(i);								
			}
			else
				States->at(i)->State->play = false;
		}
	}

	void onAdd() override
	{
		Component::onAdd();
		ChangeStates(States->at(0)->name);
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

	void onUpdate(float ET) override
	{
		PrimitiveComponent::onUpdate(ET);
#if defined(DEBUGMODE)
		
		parent->eng->DrawDecal({ ScreenPos.x,  ScreenPos.y }, parent->eng->Blank, { BLANKSIZE * ScreenScale.x , BLANKSIZE }, olc::BLUE);
		parent->eng->DrawDecal({ ScreenPos.x + ScreenScale.x,  ScreenPos.y }, parent->eng->Blank, { BLANKSIZE ,BLANKSIZE * ScreenScale.y }, olc::BLUE);
		parent->eng->DrawDecal({ ScreenPos.x + ScreenScale.x,  ScreenPos.y + ScreenScale.y }, parent->eng->Blank, { BLANKSIZE * -ScreenScale.x ,BLANKSIZE}, olc::BLUE);
		parent->eng->DrawDecal({ ScreenPos.x ,  ScreenPos.y + ScreenScale.y }, parent->eng->Blank, { BLANKSIZE  ,BLANKSIZE * -ScreenScale.y }, olc::BLUE);
#endif
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

	static void CollisionCheck(float ET, RigidComp* rc)
	{
		BoxCollider* Box = rc->Box;
			
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
								
			}
			if (calculate)
			{

#if defined(ASYNC)
				std::future<void> col = std::async(std::launch::async, CollisionCheck, ET, this);				
#else
				CollisionCheck(ET,this);
#endif

			}
		}
		
	}
	// bind like this -> rc->collideDelegate = std::bind(&class::functionName, this, std::placeholders::_1);
	std::function<void(CollisionSweep)> collideDelegate;
};



namespace DEngine
{

	static std::vector<std::string> SpriteNames(std::string pre, int start, int end, std::string post)
	{
		std::vector<std::string> temp;
		for (int i = start; i < end; i++)
		{
			temp.push_back(pre + std::to_string(i) + post);
		}
		return temp;
	}
	static float VecLength(vec2d vec)
	{
		float one = std::pow((float)abs(vec.x), 2.f);
		float two = std::pow((float)abs(vec.y), 2.f);
		return std::sqrt(one + two);
	}

	static RayHit RayTrace(Engine * eng, vec2d start, vec2d end)
	{
		float length = VecLength(vec2d(end.x - start.x, end.y - start.y));
		std::vector<Object*> Colliders;

		for (int i = 0; i < eng->Objects.size(); i++)
		{
			if (eng->Objects.at(i)->GetComponent<BoxCollider>(eng->Objects.at(i)->Components) != nullptr)
				Colliders.push_back(eng->Objects.at(i));
		}
		for (int i = 0; i < std::floor(length); i++)
		{
			vec2d check = vec2d((start.x + end.x) * i / length, (start.y + end.y) * i / length);
			for (int j = 0; j < Colliders.size(); j++)
			{
				BoxCollider* Box = Colliders.at(j)->GetComponent<BoxCollider>(Colliders.at(j)->Components);
				if (Box != nullptr && Box->Collides && check.x > Box->pos.x && check.x < (Box->pos.x + Box->scale.x) && check.y > Box->pos.y && check.y < (Box->pos.y + Box->scale.y))
				{
#if defined(DEBUGMODE)
					eng->DrawLine(start.x, start.y, check.x, check.y);
					std::cout << check.x << std::endl;
#endif
					return RayHit(check, Colliders.at(j));
				}

			}
			//std::cout << check.y << std::endl;

		}
	}

}








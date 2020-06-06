
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
			;
		}


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
	vec2d offset;

	Camera(vec2d p)
	{
		pos = p;
		zoom = 1.f;
		offset = vec2d(0, 0);
	}

};

struct RayHit
{
public:
	vec2d StartPos;
	vec2d HitPos;
	Object* HitObject;
	bool Hit;

	RayHit(bool h, vec2d p, Object* o)
	{
		HitPos = p;
		HitObject = o;
		Hit = h;
		StartPos = vec2d(0, 0);
	}


};



class Engine : public olc::PixelGameEngine
{
public:
	Engine* Inst;
	Camera* Cam;
	olc::Decal* Blank;
	float DeltaTime;
	int Background;
	int Foreground;

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
		//CreateObject(Cam);
		Blank = new olc::Decal(new olc::Sprite("Blank.png"));
		Background = CreateLayer();
		Foreground = CreateLayer();
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
		eng->DeltaTime = fElapsedTime;
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
#if defined(ENGINEASYNC)
		std::future<void> update = std::async(std::launch::async, EngineUpdate, ClearOnFrame, this, fElapsedTime);
#else
		EngineUpdate(ClearOnFrame, this, fElapsedTime);
#endif

		return true;
	}

	void CreateObject(Object* newObject)
	{
		newObject->eng = Inst;
		newObject->OnCreate();
		Objects.push_back(newObject);
	}
	void RemoveObject(Object* d)
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
		if (DelayDelegate != nullptr)
			DelayDelegate();
	}

	static void DelayUpdate(Delay * d, float ET)
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
	float angle = 0;
	int Layer;

	vec2d Scalar;
	bool Render;
	bool optimize;


	Sprite(vec2d Pos, vec2d size, olc::Pixel col = olc::WHITE) : PrimitiveComponent(Pos, size)
	{
		color = col;
		Render = true;
		CameraTransform = true;
		Scalar = vec2d(1.f, 1.f);
		optimize = true;
		Layer = 0;



	}

	Sprite(PrimitiveComponent* tp, olc::Pixel col = olc::WHITE) : PrimitiveComponent(tp)
	{
		color = col;
		Render = true;
		CameraTransform = true;
		Scalar = vec2d(1.f, 1.f);
		optimize = true;
		Layer = 0;

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

		//if(Layer == 0)
	//	Layer = parent->eng->Foreground;
		//parent->eng->EnableLayer(Layer, true);

	}

	vec2d RotateVector(vec2d origin, vec2d input, float iangle)
	{
		if (iangle < -360) iangle += 360;
		float angle = iangle * 3.1415926535 / 180;
		vec2d v = vec2d(input.x - origin.x, input.y - origin.y);
		float x = cos(angle) * v.x - sin(angle) * v.y;
		float y = sin(angle) * v.x + cos(angle) * v.y;
		v.x = origin.x + x;
		v.y = origin.y + y;
		return v;

	}


	void onUpdate(float ET) override
	{
		PrimitiveComponent::onUpdate(ET);
		if (parent->eng != nullptr)
		{
			if ((ScreenPos.x > 0 || ScreenPos.x + ScreenScale.x < (float)parent->eng->ScreenWidth() && ScreenPos.y > 0 || ScreenPos.y + ScreenScale.y < (float)parent->eng->ScreenHeight()) || !optimize)
				if (Render)
				{

					if (Dc == nullptr || Dc->sprite == nullptr)
					{
					}
					else
					{
						if (Dc->sprite != nullptr)
						{
							vec2d origin = vec2d(ScreenPos.x + ScreenScale.x / 2, ScreenPos.y + ScreenScale.y / 2);
							if (angle > 360) angle -= 360;
							if (angle < -360) angle += 360;
							vec2d one = RotateVector(origin, vec2d(ScreenPos.x + ((1 - Scalar.x) / 2 * ScreenScale.x), ScreenPos.y + ((1 - Scalar.y) / 2) * ScreenScale.y), angle);
							vec2d two = RotateVector(origin, vec2d(ScreenPos.x + ((1 - Scalar.x) / 2 * ScreenScale.x), ScreenPos.y + (ScreenScale.y * (Scalar.y + 1) / 2)), angle);
							vec2d three = RotateVector(origin, vec2d(ScreenPos.x + (ScreenScale.x * (Scalar.x + 1) / 2), ScreenPos.y + (ScreenScale.y * (Scalar.y + 1) / 2)), angle);
							vec2d four = RotateVector(origin, vec2d(ScreenPos.x + (ScreenScale.x * (Scalar.x + 1) / 2), ScreenPos.y + ((1 - Scalar.y) / 2) * ScreenScale.y), angle);

							Points[0] = olc::vf2d(one.x, one.y);
							Points[1] = olc::vf2d(two.x, two.y);
							Points[2] = olc::vf2d(three.x, three.y);
							Points[3] = olc::vf2d(four.x, four.y);

							parent->eng->DrawWarpedDecal(Dc, Points, color);
#if defined(DEBUGMODE)
							parent->eng->DrawDecal(Points[0], Debug, { BLANKSIZE * ScreenScale.x / scale.x * 3,  BLANKSIZE * ScreenScale.y / scale.y * 3 }, olc::YELLOW);
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

	bool IsColliding(BoxCollider * caller, BoxCollider * check)
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
		parent->eng->DrawDecal({ ScreenPos.x + ScreenScale.x,  ScreenPos.y + ScreenScale.y }, parent->eng->Blank, { BLANKSIZE * -ScreenScale.x ,BLANKSIZE }, olc::BLUE);
		parent->eng->DrawDecal({ ScreenPos.x ,  ScreenPos.y + ScreenScale.y }, parent->eng->Blank, { BLANKSIZE  ,BLANKSIZE * -ScreenScale.y }, olc::BLUE);
#endif
	}


	CollisionSweep CollisionCheck(BoxCollider * s)
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

namespace DEngine
{
	static vec2d RotateVector(vec2d origin, vec2d input, float iangle)
	{
		if (iangle > 360) iangle -= 360;
		if (iangle < -360) iangle += 360;
		float angle = iangle * 3.1415926535 / 180;
		vec2d v = vec2d(input.x - origin.x, input.y - origin.y);
		float x = cos(angle) * v.x - sin(angle) * v.y;
		float y = sin(angle) * v.x + cos(angle) * v.y;
		v.x = origin.x + x;
		v.y = origin.y + y;
		return v;
	}

	static void RotateVector(vec2d origin, vec2d * input, float iangle)
	{
		if (iangle > 360) iangle -= 360;
		if (iangle < -360) iangle += 360;
		float angle = iangle * 3.1415926535 / 180;
		vec2d v = vec2d(input->x - origin.x, input->y - origin.y);
		float x = cos(angle) * v.x - sin(angle) * v.y;
		float y = sin(angle) * v.x + cos(angle) * v.y;
		v.x = origin.x + x;
		v.y = origin.y + y;
		*input = v;
	}

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

	static vec2d WorldToScreen(Engine * eng, vec2d pos)
	{
		vec2d ScreenPos = pos;
		ScreenPos.x = eng->Cam->zoom * (ScreenPos.x - eng->Cam->pos.x);
		ScreenPos.y = eng->Cam->zoom * (ScreenPos.y - eng->Cam->pos.y);
		return ScreenPos;
	}

	static vec2d ScreenToWorld(Engine * eng, vec2d pos)
	{
		vec2d WorldPos = pos;
		WorldPos.x = (WorldPos.x / eng->Cam->zoom) + eng->Cam->pos.x;
		WorldPos.y = (WorldPos.y / eng->Cam->zoom) + eng->Cam->pos.y;
		return WorldPos;
	}

	static vec2d Snap(vec2d init, vec2d bounds)
	{
		vec2d v = init;
		v.x -= (int)v.x % (int)bounds.x;
		v.y -= (int)v.y % (int)bounds.y;
		return v;
	}

	static RayHit PointCollisionCheck(Engine * eng, vec2d check)
	{
		RayHit ray = RayHit(false, vec2d(0, 0), nullptr);
		std::vector<Object*> Colliders;

		for (int i = 0; i < eng->Objects.size(); i++)
		{
			if (eng->Objects.at(i)->GetComponent<BoxCollider>(eng->Objects.at(i)->Components) != nullptr)
				Colliders.push_back(eng->Objects.at(i));
		}
		for (int j = 0; j < Colliders.size(); j++)
		{
			BoxCollider* Box = Colliders.at(j)->GetComponent<BoxCollider>(Colliders.at(j)->Components);

			if (Box != nullptr && Box->Collides && check.x > Box->pos.x && check.x < (Box->pos.x + Box->scale.x) && check.y > Box->pos.y && check.y < (Box->pos.y + Box->scale.y))
			{
				ray = RayHit(true, check, Colliders.at(j));
			}

		}
		return ray;
	}

	static RayHit RayTrace(Engine * eng, vec2d start, vec2d end)
	{
		RayHit* Ray = new RayHit(false, vec2d(0, 0), nullptr);
		float length = VecLength(vec2d(end.x - start.x, end.y - start.y));
		vec2d check;

		std::vector<Object*> Colliders;

		for (int i = 0; i < eng->Objects.size(); i++)
		{
			if (eng->Objects.at(i)->GetComponent<BoxCollider>(eng->Objects.at(i)->Components) != nullptr)
				Colliders.push_back(eng->Objects.at(i));
		}
		for (int i = 0; i < (int)length; i++)
		{
			check = vec2d(start.x + ((end.x - start.x) * i / length), start.y + ((end.y - start.y) * i / length));
			for (int j = 0; j < Colliders.size(); j++)
			{
				BoxCollider* Box = Colliders.at(j)->GetComponent<BoxCollider>(Colliders.at(j)->Components);

				if (Box != nullptr && Box->Collides && check.x > Box->pos.x && check.x < (Box->pos.x + Box->scale.x) && check.y > Box->pos.y && check.y < (Box->pos.y + Box->scale.y))
				{
					Ray = new RayHit(true, check, Colliders.at(j));
					Ray->StartPos = start;
				}

			}
		}
		if (!Ray->Hit)
		{
			Ray->StartPos = start;
			Ray->HitPos = check;
		}

		return *Ray;
	}

	static std::vector<BoxCollider*> BoxTrace(Engine * eng, vec2d pos, vec2d scale, BoxCollider * ignore)
	{
		std::vector<Object*> scan = eng->Objects;
		std::vector<BoxCollider*> Bounders;
		for (int i = 0; i < scan.size(); ++i)
		{
			BoxCollider* b = scan.at(i)->GetComponent<BoxCollider>(scan.at(i)->Components);
			if (b != nullptr)
			{
				if (b != ignore)
				{

					if (pos.x < b->pos.x + b->scale.x &&
						pos.x + scale.x > b->pos.x &&
						pos.y < b->pos.y + b->scale.y &&
						pos.y + scale.y > b->pos.y) Bounders.push_back(b);
				}
			}
		}

		return Bounders;
	}

	class Debug : public Object
	{
	public:
		using Object::Object;

		vec2d p1;
		vec2d p2;
		bool Hit;

		void OnUpdate(float ET) override
		{
			Object::OnUpdate(ET);
			vec2d p11 = DEngine::WorldToScreen(eng, p1);
			vec2d p21 = DEngine::WorldToScreen(eng, p2);
			eng->DrawDecal({ p11.x, p11.y }, eng->Blank, { BLANKSIZE * 2 ,BLANKSIZE * 2 }, olc::RED);
			eng->DrawDecal({ p21.x, p21.y }, eng->Blank, { BLANKSIZE * 2 ,BLANKSIZE * 2 }, Hit ? olc::DARK_RED : olc::BLUE);
		}
	};

	static void DrawDebugRay(Engine* eng, RayHit& ray)
	{
#if defined(DEBUGMODE)
		Debug* D = new Debug();
		D->p1 = ray.StartPos;
		D->p2 = ray.HitPos;
		D->Hit = ray.Hit;
		eng->CreateObject(D);
#endif
	}
}


class RigidComp : public PrimitiveComponent
{
public:

	bool collides;
	bool Gravity;
	vec2d vel;
	bool onGround;
	float friction;
	float GroundLength;
	float bounce;
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
		GroundLength = 0;

	}

	virtual void OnCollide(CollisionSweep col)
	{
		if (collideDelegate != nullptr)
			collideDelegate(col);
	}

	void onAdd() override
	{
		Component::onAdd();
		if (Gravity) vel.y += parent->eng->Gravity * parent->eng->DeltaTime;
	}

	static void CollisionCheck(float ET, RigidComp * rc)
	{

		BoxCollider* Box = rc->Box;

		Box->pos.x += rc->vel.x * ET;
		CollisionSweep c1 = Box->CollisionCheck(Box);
		if (c1.collides)
		{
			Box->pos.x -= rc->vel.x * ET;
			if (rc->bounce > 0.f)
			{
				RigidComp* oc = c1.Other->GetComponent<RigidComp>(c1.Other->Components);
				if (oc != nullptr)
					rc->vel.x = oc->vel.x * rc->bounce;
				else
					rc->vel.x = rc->vel.x * -1 * rc->bounce;
			}
			else
				rc->vel.x -= rc->vel.x * ET;

			rc->OnCollide(c1);
		};

		Box->pos.y += rc->vel.y * ET;

		c1 = Box->CollisionCheck(Box);
		if (c1.collides)
		{
			Box->pos.y -= rc->vel.y * ET;
			if (rc->bounce > 0.f)
			{
				RigidComp* oc = c1.Other->GetComponent<RigidComp>(c1.Other->Components);
				if (oc != nullptr)
					rc->vel.y = oc->vel.y * rc->bounce;
				else
					rc->vel.y = rc->vel.y * -1 * rc->bounce;
			}
			else
				rc->vel.x -= rc->vel.x * ET;

			rc->OnCollide(c1);
			// check ground

		}
		else
		{
			if (rc->Gravity) rc->vel.y += rc->parent->eng->Gravity * ET;
		}
		if (rc->vel.y >= 0)
		{
			if (rc->GroundLength > 0)
			{
				RayHit R1 = DEngine::RayTrace(rc->parent->eng, vec2d(rc->pos.x, rc->pos.y + rc->scale.y + 5), vec2d(rc->pos.x, rc->pos.y + rc->scale.y + rc->GroundLength + 5));
				DEngine::DrawDebugRay(rc->parent->eng, R1);
				RayHit R2 = DEngine::RayTrace(rc->parent->eng, vec2d(rc->pos.x + rc->scale.x, rc->pos.y + rc->scale.y + 5), vec2d(rc->pos.x + rc->scale.x, rc->pos.y + rc->scale.y + rc->GroundLength + 5));
				DEngine::DrawDebugRay(rc->parent->eng, R2);
				if (R1.Hit || R2.Hit)
					rc->onGround = true;
				else
					rc->onGround = false;
			}
			else
				rc->onGround = false;
		}
		else
			rc->onGround = false;



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

				if (!(abs(vel.x) > 0.1f || abs(vel.y) > 0.1f))
					calculate = false;
			}

			if (calculate)
			{

#if defined(ASYNC)
				std::future<void> col = std::async(CollisionCheck, ET, this);
#else
				CollisionCheck(ET, this);
#endif

			}
		}

	}
	// bind like this -> rc->collideDelegate = std::bind(&class::functionName, this, std::placeholders::_1);
	std::function<void(CollisionSweep)> collideDelegate;
};













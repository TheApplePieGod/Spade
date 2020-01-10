#pragma once

class level;

enum class actor_flag
{
	Ready,
	PendingDestroy,
};

class actor
{

public:

	actor(level* _Level);

	rotator Rotation = rotator{ 0.f, 0.f, 0.f };
	v3 Scale = v3{ 1.f, 1.f, 1.f };

	bool Active = true;

	//cArray<renderingComponent*> RenderingComponents;

	// used for internally removing actors
	u32 ActorID = 0;
	// saved actors are capped at 50 char tag
	std::string ActorTag = "";
	actor_flag Flag = actor_flag::Ready;

	virtual void Tick();
	virtual void Destroy();
	void SetLocation(v3 NewLocation);

	inline level* GetLevel()
	{
		return Level;
	}

	inline v3 GetLocation()
	{
		return Location;
	}

private:

	level* Level;
	v3 Location = v3{ 0.f, 0.f, 0.f };

};
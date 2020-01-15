#pragma once
#include "Component.h"

class level;

enum class actor_flag
{
	Idle,
	PositionUpdated,
};

class actor_component : public component
{

public:

	actor_component(level* _Level);

	// saved actors are capped at 50 char tag
	std::string ActorTag = "";
	actor_flag Flag = actor_flag::Idle;

	inline level* GetLevel()
	{
		return Level;
	}

	inline const transform GetTransform()
	{
		return Transform;
	}

	inline v3 GetLocation()
	{
		return Transform.Location;
	}

	inline rotator GetRotation()
	{
		return Transform.Rotation;
	}

	inline v3 GetScale()
	{
		return Transform.Scale;
	}

	inline void SetTransform(transform _Transform)
	{
		Transform = _Transform;
		Flag = actor_flag::PositionUpdated;
	}

	inline void SetLocation(v3 Location) // set physics body location as well
	{
		Transform.Location = Location;
		Flag = actor_flag::PositionUpdated;
	}

	inline void SetRotation(rotator Rotation)
	{
		Transform.Rotation = Rotation;
		Flag = actor_flag::PositionUpdated;
	}

	inline void SetScale(v3 Scale)
	{
		Transform.Scale = Scale;
		Flag = actor_flag::PositionUpdated;
	}

private:

	level* Level;
	transform Transform;

};

class actor
{
public:

	// used to identify subclasses of actor
	const char* TypeIdentifier = "Default";

	s32 ActorComponentID = -1;

	// used for internally removing actors
	u32 ActorID = 0;
};

class renderer_actor : public actor
{
public:

	renderer_actor()
	{
		TypeIdentifier = "Renderer";
	}

	/* used to access global array of rendering components
	 * call the engine function CreateRenderingComponent() to create component
	 * -1 means no component
	*/
	s32 RenderingComponentID = -1;

};
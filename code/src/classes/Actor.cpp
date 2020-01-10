#include "pch.h"
#include "MathTypes.h"
#include "Actor.h"

actor::actor(level* _Level)
{
	Level = _Level;
}

void actor::Tick()
{

}

void actor::Destroy()
{
	// todo
}

void actor::SetLocation(v3 NewLocation)
{
	Location = NewLocation;
}
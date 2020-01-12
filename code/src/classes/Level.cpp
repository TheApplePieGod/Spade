#include "pch.h"
#include "MathTypes.h"
#include "Actor.h"
//#include "Level.h"
#include "../engine/Engine.h"

extern engine* Engine;

void level::Initialize()
{
	LoadLevelData();
}

void level::LoadLevelData()
{

}

void level::RemoveActorFromRegistry(actor* Actor)
{
	u32 ActorID = Actor->ActorID;
	for (u32 i = 0; i < ActorRegistry.size(); i++)
	{
		if (ActorRegistry[i]->ActorID == ActorID)
		{
			ActorRegistry.erase(ActorRegistry.begin() + i);
			return;
		}
	}
}

u32 level::AddActorToRegistry(actor* Actor)
{
	Actor->ActorID = ActorsAdded;
	ActorRegistry.push_back(Actor);
	return ActorsAdded++;
}

actor* level::FindActorByID(u32 ID)
{
	for (actor* Actor : ActorRegistry)
	{
		if (Actor->ActorID == ID)
			return Actor;
	}
	return nullptr;
}

void level::ResetLevelData()
{
	//for (actor* Actor : ActorRegistry)
	//{
	//	Actor->Destroy();
	//}
	ActorRegistry.clear();
}
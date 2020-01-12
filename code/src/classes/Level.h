#pragma once

class actor;
class level
{

public:
	// do not add to directly, use AddActorToRegistry
	std::vector<actor*> ActorRegistry;
	
	// calls LoadLevelData
	void Initialize();
	void LoadLevelData();
	void RemoveActorFromRegistry(actor* Actor);
	u32 AddActorToRegistry(actor* Actor);
	actor* FindActorByID(u32 ID);
	// deletes all loaded actor & level data
	void ResetLevelData();
	
	bool LevelLoaded = false;

	u32 ActorsAdded = 0;

};
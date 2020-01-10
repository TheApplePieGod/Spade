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
	void AddActorToRegistry(actor* Actor);
	// deletes all loaded actor & level data
	void ResetLevelData();
	
	bool LevelLoaded = false;

	u32 ActorsAdded = 0;

};
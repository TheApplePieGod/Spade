#pragma once
#include "Input.h"
#include "platform/rendering/PlatformRenderer.h"
#include "../classes/Camera.h"
#include "../classes/Level.h"
#include "../classes/RenderingComponent.h"
#include "../classes/Actor.h"
#include "MemoryManager.h"

class engine
{
public:

	std::vector<cAsset*> AssetRegistry;
	std::vector<cTextureAsset*> TextureRegistry;

	bool IsRunning = true;

	memory_manager MemoryManager;
	camera MainCamera;
	level MainLevel;
	user_inputs UserInputs;

	// Called from platform
	void Tick(); // every frame
	void Initialize(void* Window, int WindowWidth, int WindowHeight);
	void Cleanup();

	void ProcessUserInput();
	void RenderScene();

	s32 CreateMaterial();
	material& GetMaterial(s32 ID);

	// Engine classes
	renderer Renderer;

private:

	// component registries
	component_registry<rendering_component> RenderingComponents;
	component_registry<actor_component> ActorComponents;

	std::vector<material> MaterialRegistry;
	s32 MaterialsAdded = 0;

};
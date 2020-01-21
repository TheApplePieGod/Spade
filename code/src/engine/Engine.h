#pragma once
#include "Input.h"
#include "platform/rendering/PlatformRenderer.h"
#include "../classes/Camera.h"
#include "../classes/Level.h"
#include "../classes/RenderingComponent.h"
#include "../classes/Actor.h"
#include "../classes/Pipeline.h"
#include "MemoryManager.h"

struct debug_data
{
	f32 SunAngle = 0.f;
};

class engine
{
public:

	std::vector<cAsset*> AssetRegistry;
	std::vector<cTextureAsset*> TextureRegistry;
	std::vector<shader> ShaderRegistry; // shader entry point names must be unique

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
	void UpdateComponents();

	renderer Renderer;
	v2 ScreenSize = { 0.f, 0.f };

#if SPADE_DEBUG
	debug_data DebugData;
#endif

private:

	// component registries (high performance)
	component_registry<rendering_component> RenderingComponents;
	component_registry<actor_component> ActorComponents;
	component_registry<material> MaterialRegistry;
	component_registry<pipeline_state> PipelineStates;
};
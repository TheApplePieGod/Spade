#pragma once
#include "Input.h"
#include "platform/rendering/PlatformRenderer.h"
#include "../classes/Camera.h"
#include "../classes/Level.h"
#include "../classes/RenderingComponent.h"
#include "../classes/Actor.h"
#include "../classes/Pipeline.h"
#include "../classes/TerrainManager.h"
#include "MemoryManager.h"

struct debug_data
{
	f32 SunAngle = 0.f;
	f32 CameraSpeed = 0.08f;
	u32 NumTerrainVertices = 0;
	bool EnableWireframe = false;
	bool VisibleChunkUpdates = true;
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
	planet_terrain_manager TerrainManager;

	std::thread ChunkUpdateThread;

	// Called from platform
	void Tick(); // every frame
	void Initialize(void* Window, int WindowWidth, int WindowHeight);
	void Cleanup();

	void InitializeAssetSystem();
	void ProcessUserInput();
	void RenderPlanet();
	void RenderScene();
	void UpdateComponents();
	void RenderDebugWidgets();

	float PlanetRadius = 998.f;
	s32 AtmosphereRenderingID = -1;
	s32 PlanetRenderingID = -1;

	renderer Renderer;
	v2 ScreenSize = { 0.f, 0.f };

#if SPADE_DEBUG
	debug_data DebugData;
#endif

	// component registries (high performance)
	component_registry<rendering_component> RenderingComponents;
	component_registry<actor_component> ActorComponents;
	component_registry<material> MaterialRegistry;
	component_registry<pipeline_state> PipelineStates;
};
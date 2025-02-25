#pragma once
#include "Input.h"
#include "platform/rendering/PlatformRenderer.h"
#include "../classes/Camera.h"
#include "../classes/Level.h"
#include "../classes/RenderingComponent.h"
#include "../classes/Actor.h"
#include "../classes/Pipeline.h"
#include "../classes/TerrainManager.h"
#include "../classes/Foliage.h"
#include "MemoryManager.h"

struct debug_data
{
	f32 SunAngle = 0.f;
	f32 CameraSpeed = 1.f;//0.08f;
	u32 NumTerrainVertices = 0;
	bool EnableWireframe = false;
	bool DrawNormals = false;
	bool VisibleChunkUpdates = true;
	bool SlowMode = false;
	bool FreeCam = true;
	bool SpawnCube = false;
	int IntersectingIndex = -1;
	int IntersectingTree = -1;
	vertex FrustumTrianglePoints[3];
	u64 FrameCount = 0;
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
	foliage_manager FoliageManager;

	std::thread ChunkUpdateThread;

	// Called from platform
	void Tick(); // every frame
	void Initialize(void* Window, int WindowWidth, int WindowHeight);
	void Cleanup();

	void FinishFrame();
	void UpdateState();
	void InitializeAssetSystem();
	void ProcessUserInput();
	void UpdateCamera();
	void UpdateTerrain();
	void RenderPlanet();
	void RenderScene();
	void RenderDirectionalShadowCascade();
	void RenderVarianceShadowMap();
	void RenderPlanetGeometry();
	void UpdateComponents();
	void RenderDebugWidgets();
	void RenderDebugShadowmap();

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
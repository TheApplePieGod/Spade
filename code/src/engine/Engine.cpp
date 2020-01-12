#include "pch.h"
#include "Engine.h"
#include "AssetUtils.h"
#include "../classes/Actor.h"
#include "MathUtils.h"

#if PLATFORM_WINDOWS
#include "../startup/WindowsMain.cpp"
#endif

extern shader_constants_actor ActorConstants;

void engine::Tick()
{
	ProcessUserInput();

	MainCamera.UpdateFromInput();

	MainCamera.UpdateProjectionType(projection_type::Perspective);
	MainCamera.ViewMatrix = renderer::GenerateViewMatrix(true, MainCamera.CameraInfo, MainCamera.LookAtVector);
	RenderScene();

	ImGui::ShowDemoWindow();
	Renderer.FinishFrame();
}

void destruct(actor* Actor)
{

}

void engine::Initialize(void* Window, int WindowWidth, int WindowHeight)
{
	MainLevel = level();
	MainCamera = camera();
	MemoryManager = memory_manager();

	//debug
	s32 matid = CreateMaterial();
	material& Mat = GetMaterial(matid);
	Mat.DiffuseColor = colors::Green;

	MemoryManager.Initialize();
	Renderer.Initialize(Window, WindowWidth, WindowHeight);
	MainLevel.Initialize();

#if SPADE_DEBUG
	assetLoader::ScanAssets("assets\\", false);
	assetLoader::InitializeAssetsInDirectory("assets\\", &(assetLoader::asset_load_callbacks)AssetLoadCallbacks);
#else
	assetLoader::InitializeAssetsFromPac(&AssetLoadCallbacks);
#endif
}

void engine::Cleanup()
{
	Renderer.Cleanup();
}

void engine::ProcessUserInput()
{

}

bool CompareRenderComponents(rendering_component& Comp1, rendering_component& Comp2)
{
	return Comp1.RenderResources.MaterialID < Comp2.RenderResources.MaterialID;
}

void engine::RenderScene()
{
	// Sort by material ID (optimize?)
	std::sort(RenderingComponents.begin(), RenderingComponents.end(), CompareRenderComponents);
	s32 MaterialID = -1;

	for (u32 i = 0; i < RenderingComponents.size(); i++)
	{
		if (RenderingComponents[i].RenderResources.MaterialID != -1 && RenderingComponents[i].Active)
		{
			if (RenderingComponents[i].RenderResources.MaterialID != MaterialID)
			{
				MaterialID = RenderingComponents[i].RenderResources.MaterialID;
				Renderer.BindMaterial(GetMaterial(MaterialID));
			}

			// optimize?
			for (u32 z = 0; z < MainLevel.ActorRegistry.size(); z++)
			{
				if (MainLevel.ActorRegistry[z]->Active && MainLevel.ActorRegistry[z]->RenderingComponentID == RenderingComponents[i].ComponentID)
				{
					ActorConstants.ViewProjectionMatrix = MainCamera.ProjectionMatrix * MainCamera.ViewMatrix;
					Renderer.MapActorConstants(MainLevel.ActorRegistry[z], RenderingComponents[i]);

					// todo: draw instanced
					//Renderer.Draw(RenderingComponents[i].RenderResources.Vertices, RenderingComponents[i].RenderResources.TopologyType);
				}
			}
		}
	}
}

s32 engine::CreateMaterial()
{
	material Mat;
	Mat.MaterialID = MaterialsAdded;
	MaterialRegistry.push_back(Mat);
	return MaterialsAdded++;
}

material& engine::GetMaterial(s32 ID)
{
	if (ID != -1)
	{
		u32 size = (u32)MaterialRegistry.size();
		for (u32 i = 0; i < size; i++)
		{
			if (MaterialRegistry[i].MaterialID == ID)
				return MaterialRegistry[i];
		}
		Assert(1 == 2); // not found
	}
	Assert(1 == 2); // cant get id of -1
	return MaterialRegistry[0];
}
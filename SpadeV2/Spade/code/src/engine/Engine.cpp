#include "pch.h"
#include "Engine.h"
#include "../utils/AssetUtils.h"
#include "../classes/Camera.h"

void engine::Tick()
{
	ProcessUserInput();
	MainCamera->UpdateFromInput();
	MainCamera->ViewMatrix = renderer::GenerateViewMatrix(true, MainCamera->CameraInfo, MainCamera->LookAtVector);
}

void engine::Initialize(void* Window, int WindowWidth, int WindowHeight)
{
	// Setup ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	Renderer.Initialize(Window, WindowHeight, WindowWidth);

	MainCamera = new camera();

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


#if PLATFORM_WINDOWS
#include "../windows/WindowsMain.cpp"
#endif
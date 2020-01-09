#include "pch.h"
#include "Engine.h"
#include "../utils/AssetUtils.h"
#include "../classes/Camera.h"

#if PLATFORM_WINDOWS
#include "../windows/WindowsMain.cpp"
#endif

void engine::Tick()
{
	ProcessUserInput();

	MainCamera->UpdateFromInput();
	MainCamera->ViewMatrix = renderer::GenerateViewMatrix(true, MainCamera->CameraInfo, MainCamera->LookAtVector);

	ImGui::ShowDemoWindow();

	Renderer.FinishFrame();
}

void engine::Initialize(void* Window, int WindowWidth, int WindowHeight)
{
	Renderer.Initialize(Window, WindowWidth, WindowHeight);

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

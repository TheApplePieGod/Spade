#include "pch.h"
#include "Engine.h"
#include "../classes/Level.h"
#include "../classes/Camera.h"
#include "../utils/AssetUtils.h"

#if PLATFORM_WINDOWS
#include "../startup/WindowsMain.cpp"
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
	MainLevel = new level();
	MainCamera = new camera();

	Renderer.Initialize(Window, WindowWidth, WindowHeight);
	MainLevel->Initialize();

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

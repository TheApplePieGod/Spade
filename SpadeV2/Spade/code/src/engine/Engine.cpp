#include "pch.h"
#include "Engine.h"
#include "../utils/AssetUtils.h"


void engine::Tick()
{

}

void engine::Initialize(void* Window, int WindowWidth, int WindowHeight)
{
	// Setup ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	Renderer.Initialize(Window, WindowHeight, WindowWidth);

#if SPADE_DEBUG
	assetLoader::ScanAssets(false);
	assetLoader::InitializeAssetsInDirectory("assets\\", &(assetLoader::asset_load_callbacks)AssetLoadCallbacks);
#else
	assetLoader::InitializeAssetsFromPac(&AssetLoadCallbacks);
#endif


}

void engine::Cleanup()
{

}

#if PLATFORM_WINDOWS
#include "../windows/WindowsMain.cpp"
#endif
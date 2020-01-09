#pragma once
#include "Input.h"
#include "platform/rendering/PlatformRenderer.h"

class camera;

class engine
{
public:

	std::vector<cAsset*> AssetRegistry;
	std::vector<cTextureAsset*> TextureRegistry;

	bool IsRunning = true;

	camera* MainCamera;
	user_inputs UserInputs;

	// Called from platform
	void Tick(); // every frame
	void Initialize(void* Window, int WindowWidth, int WindowHeight);
	void Cleanup();

	void ProcessUserInput();

	// Engine classes
	renderer Renderer;

private:
};
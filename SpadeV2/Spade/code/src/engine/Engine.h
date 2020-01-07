#pragma once
#include "Input.h"
#include "platform/rendering/PlatformRenderer.h"

class engine
{
public:

	bool IsRunning = true;

	user_inputs UserInputs;

	// Called every frame
	void Tick();
	void Initialize(void* Window, int WindowWidth, int WindowHeight);
	void Cleanup();

	// Engine classes
	renderer Renderer;

private:
};
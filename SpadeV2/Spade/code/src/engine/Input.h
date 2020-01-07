#pragma once
#include "MathUtils.h"

struct input_button
{
	bool Pressed = false;
};

struct user_inputs // controller support later?
{
	// all keys
	input_button KeysDown[256];

	// right and left mouse buttons
	input_button RMB;
	input_button LMB;

	bool MouseMovement = false;
	bool PlayerMovement = false;

	// view space
	f32 MousePosX;
	f32 MousePosY;
	v2 MousePosWorldSpace;

	float DeltaTime = 0;
};
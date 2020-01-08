#pragma once
#include "../engine/platform/rendering/RendererShared.h"

class camera
{
public:
	camera_info CameraInfo;

	v3 ForwardVector = { 0.f, 0.f, 1.f };
	v3 RightVector = { 1.f, 0.f, 0.f };
	v3 UpVector = { 0.f, 1.f, 0.f }; // static for now
	v3 LookAtVector = { 0.f, 0.f, 1.f };

	matrix4x4 ViewMatrix;
	matrix4x4 ProjectionMatrix;

	void UpdateFromInput();
};
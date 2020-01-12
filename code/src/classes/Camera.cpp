#include "pch.h"
#include "Camera.h"
#include "../engine/Engine.h"
#include "MathUtils.h"

extern engine* Engine;

void camera::UpdateFromInput()
{
	ForwardVector = Normalize(LookAtVector);
	RightVector = CrossProduct(UpVector, ForwardVector);

	if (Engine->UserInputs.MouseMovement)
	{
		CameraInfo.Rotation.x += (Engine->UserInputs.MouseDeltaX * 0.1f);
		CameraInfo.Rotation.y += (Engine->UserInputs.MouseDeltaY * 0.1f);
	}
}

void camera::UpdateProjectionType(projection_type NewType)
{
	if (CameraInfo.ProjectionType != NewType)
	{
		CameraInfo.ProjectionType = NewType;

		switch (NewType)
		{
			default:
			{} break;

			case projection_type::Orthographic:
			{
				Engine->Renderer.GetOrthographicProjectionLH(true, CameraInfo);
			} break;

			case projection_type::Perspective:
			{
				Engine->Renderer.GetPerspectiveProjectionLH(true, CameraInfo);
			} break;
		}
	}
}
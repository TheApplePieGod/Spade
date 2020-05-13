#include "pch.h"
#include "Camera.h"
#include "../engine/Engine.h"
#include "MathUtils.h"

extern engine* Engine;

using namespace DirectX;
void camera::UpdateFromInput()
{
	ForwardVector = Normalize(LookAtVector);
	RightVector = CrossProduct(UpVector, ForwardVector);

	if (!Engine->UserInputs.GuiMouseFocus)
	{
		if (Engine->UserInputs.MouseMovement)
		{
			CameraInfo.Transform.Rotation.x += (Engine->UserInputs.MouseDeltaX * MouseInputScale);
			CameraInfo.Transform.Rotation.y += (Engine->UserInputs.MouseDeltaY * MouseInputScale);
		}
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
				ProjectionMatrix = Engine->Renderer.GetOrthographicProjectionLH(true, CameraInfo);
			} break;

			case projection_type::Perspective:
			{
				ProjectionMatrix = Engine->Renderer.GetPerspectiveProjectionLH(true, CameraInfo);
			} break;
		}
	}
}
#pragma once
#include "Component.h"

class material : public component
{
public:
	s32 DiffuseTextureID = -1;
	v4 DiffuseColor = colors::White;

	s32 NormalTextureID = -1;

	s32 ReflectiveTextureID = -1;
	f32 Reflectivity = 0.0f;
};
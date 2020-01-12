#pragma once

struct material
{
	s32 MaterialID = -1;

	// Leave blank to use color
	s32 DiffuseShaderID = -1;
	v4 DiffuseColor = colors::White;

	s32 NormalShaderID = -1;
};
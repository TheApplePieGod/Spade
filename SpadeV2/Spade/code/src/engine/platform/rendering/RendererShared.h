#pragma once
#include "MathUtils.h"

enum class shader_type
{
	VertexShader,
	PixelShader,
	GeometryShader
};

enum class shader_presets : u32
{
	Default,
	Font,
	Tiles,
	Circle,
};

enum class draw_topology_types
{
	TriangleList,
	LineList,
	PointList,
};

const float VoidColor[4] = { 0.3f, 0.7f, 0.9f, 1.f };
#pragma once
#include "MathTypes.h"

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

enum class projection_type
{
	Perspective,
	Orthographic,
};

struct camera_info
{
	projection_type ProjectionType;
	v3 Position;
	rotator Rotation;
	f32 FOV; // field of view (degrees)
	f32 Width;
	f32 Height;
	f32 NearPlane;
	f32 FarPlane;
};


const float VoidColor[4] = { 0.3f, 0.7f, 0.9f, 1.f };
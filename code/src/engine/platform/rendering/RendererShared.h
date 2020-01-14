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

enum class map_operation
{
	Actor,
	Material,
	Frame
};

struct camera_info
{
	projection_type ProjectionType;
	v3 Position = { 0.f, 0.f, -1.f };
	rotator Rotation = rotator();
	f32 FOV = 70.f; // field of view (degrees)
	f32 Width = 0.f;
	f32 Height = 0.f;
	f32 NearPlane = 0.01f;
	f32 FarPlane = 50000.f;
};

struct actor_instance
{
	matrix4x4 WorldMatrix;
};

#define MAX_INSTANCES (u64)(65536.f / sizeof(actor_instance))

struct shader_constants_actor
{
	actor_instance Instances[MAX_INSTANCES];
};

struct shader_constants_frame
{
	matrix4x4 ViewProjectionMatrix;
};

struct shader_constants_material
{
	bool TextureDiffuse = false;
	v3 padding;

	v4 DiffuseColor = colors::White;

	bool TextureNormal = false;
	v3 padding2;
};

const float VoidColor[4] = { 0.3f, 0.7f, 0.9f, 1.f };
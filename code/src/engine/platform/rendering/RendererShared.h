#pragma once
#include "MathTypes.h"

enum class shader_type
{
	VertexShader,
	PixelShader,
	GeometryShader,
	HullShader,
	DomainShader
};

enum class render_state
{
	ShadowMap,
	VarianceMapX,
	VarianceMapY,
	Main,
};

enum class shader_presets : u32
{
	Default,
	Font,
	Tiles,
	Circle,
};

enum class draw_topology_type
{
	TriangleList,
	TriangleStrip,
	LineList,
	PointList,
};

enum class projection_type
{
	NotSet,
	Perspective,
	Orthographic,
};

enum class map_operation
{
	Actor,
	Material,
	Frame,
	Lighting
};

enum class rasterizer_state
{
	DefaultCullBackface,
	DefaultCullFrontface,
	DefaultCullNone,
	Wireframe
};

struct camera_info
{
	projection_type ProjectionType = projection_type::NotSet;
	transform Transform;
	f32 FOV = 80.f; // field of view (degrees)
	f32 Width = 0.f;
	f32 Height = 0.f;
	f32 NearPlane = 0.01f;
	f32 FarPlane = 50000.f;
};

struct actor_instance
{
	matrix4x4 WorldMatrix;
	matrix4x4 InverseTransposeWorldMatrix;
};

#define MAX_INSTANCES (u64)(65536.f / sizeof(actor_instance))
#define NUM_CASCADES 4
#define SHADOWMAP_SIZE 2048.f

struct shader_constants_actor
{
	actor_instance Instances[MAX_INSTANCES];
};

struct shader_constants_frame
{
	matrix4x4 CameraViewProjectionMatrix;
	matrix4x4 CameraWorldMatrix;
	matrix4x4 CameraViewMatrix;
	matrix4x4 SunViewProjectionMatrix[NUM_CASCADES];
	v3 CameraPosition;
	u32 CurrentCascade;
};

struct shader_constants_material
{
	b32 TextureDiffuse = false;
	b32 TextureNormal = false;
	b32 TextureReflective = false;
	f32 Reflectivity = 0.0f;

	v4 DiffuseColor = colors::White;
};

struct shader_constants_lighting
{
	v3 AmbientColor;
	f32 SunIntensity;

	v3 SunColor;
	u32 NumCascades;

	v3 SunDirection;
	u32 CurrentCascadePS;
};

const float VoidColor[4] = { 0.f, 0.f, 0.f, 1.f };//{ 0.3f, 0.7f, 0.9f, 1.f };
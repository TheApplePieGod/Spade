#pragma once
#include "Component.h"
#include "../engine/platform/rendering/RendererShared.h"
#include "../utils/AssetUtils.h"

class shader
{
public:
	shader_type ShaderType;
	void* ShaderRef;
	const char* Name;
};

class pipeline_state : public component
{
public:
	s32 VertexShaderID = -1;
	s32 PixelShaderID = -1;
	s32 GeometryShaderID = -1;
	rasterizer_state RasterizerState;
	std::string UniqueIdentifier;
};

class material : public component
{
public:
	s32 DiffuseTextureID = -1;
	v4 DiffuseColor = colors::White;

	s32 NormalTextureID = -1;

	s32 ReflectiveTextureID = -1;
	f32 Reflectivity = 0.0f;

	assetTypes::material_data Serialize();
	void Deserialize(assetTypes::material_data Data);
};
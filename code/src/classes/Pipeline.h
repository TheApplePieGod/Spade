#pragma once
#include "Component.h"
#include "../engine/platform/rendering/RendererShared.h"

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
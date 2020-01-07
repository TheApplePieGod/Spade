#include "pch.h"
#include "MathUtils.h"
#include "PlatformRenderer.h"

#if SDK_DIRECTX11
#include "DX11Renderer.cpp"
#endif

platform_renderer* PlatformRenderer;

void renderer::Initialize(void* _Window, int WindowWidth, int WindowHeight)
{
	PlatformRenderer = new platform_renderer();
	Window = _Window;
	PlatformRenderer->Initialize(_Window, WindowWidth, WindowHeight);
}

void renderer::Cleanup()
{
	PlatformRenderer->Cleanup();
}

void renderer::Draw(std::vector<vertex>& VertexArray, draw_topology_types TopologyType)
{
	PlatformRenderer->Draw(VertexArray, TopologyType);
}

void renderer::Draw(std::vector<v3>& PositionArray, draw_topology_types TopologyType)
{
	PlatformRenderer->Draw(PositionArray, TopologyType);
}

void renderer::SetViewport(float Width, float Height)
{
	PlatformRenderer->SetViewport(Width, Height);
}

void renderer::SetDrawTopology(draw_topology_types TopologyType)
{
	PlatformRenderer->SetDrawTopology(TopologyType);
}

void renderer::CompileShaderFromFile(std::string Filename, std::string EntryPoint, shader_type ShaderType, void* ShaderRef)
{
	PlatformRenderer->CompileShaderFromFile(Filename, EntryPoint, ShaderType, ShaderRef);
}
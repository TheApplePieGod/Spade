#include "pch.h"
#include "MathTypes.h"
#include "PlatformRenderer.h"

#if SDK_DIRECTX11
#include "DX11Renderer.cpp"
#endif

platform_renderer* PlatformRenderer;

void renderer::Initialize(void* _Window, int WindowWidth, int WindowHeight)
{
	// Setup ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	PlatformRenderer = new platform_renderer();
	Window = _Window;
	PlatformRenderer->Initialize(_Window, WindowWidth, WindowHeight);

	ImGui::NewFrame();
}

void renderer::Cleanup()
{
	PlatformRenderer->Cleanup();
}

void renderer::FinishFrame()
{
	PlatformRenderer->FinishFrame();
	ImGui::NewFrame();
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

void renderer::RegisterTexture(cAsset* Asset, bool GenerateMIPs)
{
	PlatformRenderer->RegisterTexture(Asset, GenerateMIPs);
}

matrix4x4 renderer::GetPerspectiveProjectionLH(bool Transpose, camera_info& CameraInfo)
{
	return PlatformRenderer->GetPerspectiveProjectionLH(Transpose, CameraInfo);
}

matrix4x4 renderer::GetOrthographicProjectionLH(bool Transpose, camera_info& CameraInfo)
{
	return PlatformRenderer->GetOrthographicProjectionLH(Transpose, CameraInfo);
}

matrix4x4 renderer::GenerateViewMatrix(bool Transpose, camera_info& CameraInfo, v3& OutLookAtMatrix, bool OrthoUseMovement)
{
	return PlatformRenderer->GenerateViewMatrix(Transpose, CameraInfo, OutLookAtMatrix, OrthoUseMovement);
}
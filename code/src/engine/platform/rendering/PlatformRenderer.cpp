#include "pch.h"
#include "PlatformRenderer.h"

#if SDK_DIRECTX11
#include "DX11Renderer.cpp"
#endif

extern engine* Engine;

platform_renderer* PlatformRenderer; // move into renderer class?

shader_constants_actor ActorConstants;
shader_constants_frame FrameConstants;
shader_constants_material MaterialConstants;
shader_constants_lighting LightingConstants;

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

	// Default vertex shaders for are registered during platform init
	RegisterShader("shader/DefaultShader.hlsl", "mainps", shader_type::PixelShader);
	RegisterShader("shader/SkyboxShader.hlsl", "skyboxvs", shader_type::VertexShader);
	RegisterShader("shader/SkyboxShader.hlsl", "skyboxps", shader_type::PixelShader);
	
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

void renderer::Draw(vertex* InVertexArray, u32 NumVertices, draw_topology_types TopologyType)
{
	PlatformRenderer->Draw(InVertexArray, NumVertices, TopologyType);
}

void renderer::Draw(v3* InPositionArray, u32 NumVertices, draw_topology_types TopologyType)
{
	PlatformRenderer->Draw(InPositionArray, NumVertices, TopologyType);
}

void renderer::DrawInstanced(vertex* InVertexArray, u32 NumVertices, u32 NumInstances, draw_topology_types TopologyType)
{
	PlatformRenderer->DrawInstanced(InVertexArray, NumVertices, NumInstances, TopologyType);
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

void renderer::BindMaterial(const material& InMaterial)
{
	PlatformRenderer->BindMaterial(InMaterial);
}

void renderer::MapConstants(map_operation Type)
{
	PlatformRenderer->MapConstants(Type);
}

void renderer::MapTextureArray()
{
	PlatformRenderer->MapTextureArray();
}

void renderer::SetPipelineState(const pipeline_state& InState)
{
	PlatformRenderer->SetPipelineState(InState);
}

matrix4x4 renderer::GetPerspectiveProjectionLH(bool Transpose, camera_info CameraInfo)
{
	return PlatformRenderer->GetPerspectiveProjectionLH(Transpose, CameraInfo);
}

matrix4x4 renderer::GetOrthographicProjectionLH(bool Transpose, camera_info CameraInfo)
{
	return PlatformRenderer->GetOrthographicProjectionLH(Transpose, CameraInfo);
}

matrix4x4 renderer::GenerateViewMatrix(bool Transpose, camera_info CameraInfo, v3& OutLookAtMatrix, bool OrthoUseMovement)
{
	return PlatformRenderer->GenerateViewMatrix(Transpose, CameraInfo, OutLookAtMatrix, OrthoUseMovement);
}

matrix4x4 renderer::GenerateWorldMatrix(transform Transform)
{
	return PlatformRenderer->GenerateWorldMatrix(Transform);
}

matrix4x4 renderer::InverseMatrix(const matrix4x4& Matrix, bool Transpose)
{
	return PlatformRenderer->InverseMatrix(Matrix, Transpose);
}

void renderer::RegisterShader(std::string Filename, const char* Entrypoint, shader_type ShaderType)
{
	shader Shader = shader();
	Shader.Name = Entrypoint;
	Shader.ShaderType = ShaderType;
	CompileShaderFromFile(Filename, Entrypoint, ShaderType, &Shader.ShaderRef);
	Engine->ShaderRegistry.push_back(Shader);
}
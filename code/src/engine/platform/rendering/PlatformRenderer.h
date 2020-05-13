#pragma once
#include "RendererShared.h"

class material;
class actor_component;
class rendering_component;
class pipeline_state;
class renderer
{
public:
	void Initialize(void* Window, int WindowWidth, int WindowHeight);
	void Cleanup();
	void FinishFrame();
	void Draw(vertex* InVertexArray, u32 NumVertices, draw_topology_type TopologyType);
	void Draw(v3* InPositionArray, u32 NumVertices, draw_topology_type TopologyType);
	void DrawInstanced(vertex* InVertexArray, u32 NumVertices, u32 NumInstances, draw_topology_type TopologyType);
	void DrawIndexedInstanced(vertex* InVertexArray, u32* InIndexArray, u32 NumVertices, u32 NumIndices, u32 IndexOffset, u32 NumInstances, draw_topology_type TopologyType);
	void DrawIndexedTerrainChunk(vertex* InVertexArray, u32* InIndexArray, u32 NumVertices, u32 NumIndices); // assumes triangle list w 4 vertices and 6 indices
	void SetViewport(float Width, float Height);
	void SetDrawTopology(draw_topology_type TopologyType);
	void CompileShaderFromFile(std::string Filename, std::string EntryPoint, shader_type ShaderType, void* ShaderRef);
	void RegisterTexture(cAsset* Asset, bool GenerateMIPs);
	void BindMaterial(const material& InMaterial);
	void MapConstants(map_operation Type);
	// 6 ids: left, right, up, down, front, back. Textures must be uniform size & channels
	void UpdateSkybox(s32* TextureIDs);
	void SetPipelineState(const pipeline_state& InState);

	static matrix4x4 GetPerspectiveProjectionLH(bool Transpose, camera_info CameraInfo);
	static matrix4x4 GetOrthographicProjectionLH(bool Transpose, camera_info CameraInfo);
	static matrix4x4 GenerateViewMatrix(bool Transpose, camera_info CameraInfo, v3& OutLookAtVector, v3& OutUpVector, bool OrthoUseMovement = true);
	static matrix4x4 GeneratePlanetaryViewMatrix(bool Transpose, camera_info CameraInfo, v2 MouseDelta, v3 ForwardVector, v3& OutLookAtVector, v3& OutUpVector, bool OrthoUseMovement = true);
	static matrix4x4 GenerateWorldMatrix(transform Transform);
	static matrix4x4 InverseMatrix(const matrix4x4& Matrix, bool Transpose);

	void* Window = nullptr;

// specific

	void RegisterShader(std::string Filename, const char*, shader_type ShaderType);

};


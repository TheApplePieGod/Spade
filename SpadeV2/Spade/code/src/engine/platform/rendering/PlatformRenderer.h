#pragma once
#include "RendererShared.h"

class renderer
{
public:
	void Initialize(void* Window, int WindowWidth, int WindowHeight);
	void Cleanup();
	void Draw(std::vector<vertex>& VertexArray, draw_topology_types TopologyType);
	void Draw(std::vector<v3>& PositionArray, draw_topology_types TopologyType);
	void SetViewport(float Width, float Height);
	void SetDrawTopology(draw_topology_types TopologyType);
	void CompileShaderFromFile(std::string Filename, std::string EntryPoint, shader_type ShaderType, void* ShaderRef);
	void RegisterTexture(cAsset* Asset, bool GenerateMIPs);

	static matrix4x4 GetPerspectiveProjectionLH(bool Transpose, camera_info& CameraInfo);
	static matrix4x4 GetOrthographicProjectionLH(bool Transpose, camera_info& CameraInfo);
	static matrix4x4 GenerateViewMatrix(bool Transpose, camera_info& CameraInfo, v3& OutLookAtMatrix, bool OrthoUseMovement = true);

	void* Window = nullptr;
};


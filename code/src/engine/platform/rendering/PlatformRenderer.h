#pragma once
#include "RendererShared.h"

struct material;
class actor_component;
class rendering_component;
class renderer
{
public:
	void Initialize(void* Window, int WindowWidth, int WindowHeight);
	void Cleanup();
	void FinishFrame();
	void Draw(vertex* InVertexArray, u32 NumVertices, draw_topology_types TopologyType);
	void Draw(v3* InPositionArray, u32 NumVertices, draw_topology_types TopologyType);
	void SetViewport(float Width, float Height);
	void SetDrawTopology(draw_topology_types TopologyType);
	void CompileShaderFromFile(std::string Filename, std::string EntryPoint, shader_type ShaderType, void* ShaderRef);
	void RegisterTexture(cAsset* Asset, bool GenerateMIPs);
	void BindMaterial(const material& InMaterial);
	void MapConstants(constants_type Type);

	/* Generates world matrix & maps the constants
	 * Call after other fields are set
	*/
	void MapActorConstants(actor_component& InActor, const rendering_component& InComponent);

	static matrix4x4 GetPerspectiveProjectionLH(bool Transpose, camera_info CameraInfo);
	static matrix4x4 GetOrthographicProjectionLH(bool Transpose, camera_info CameraInfo);
	static matrix4x4 GenerateViewMatrix(bool Transpose, camera_info CameraInfo, v3& OutLookAtMatrix, bool OrthoUseMovement = true);

	void* Window = nullptr;

};


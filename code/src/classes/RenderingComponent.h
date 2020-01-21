#pragma once
#include "Material.h"
#include "Component.h"
#include "../engine/platform/rendering/PlatformRenderer.h"

struct mesh_render_resources
{
	s32 MeshAssetID = -1;
	s32 MaterialID = -1;
	s32 PipelineStateID = -1;
	draw_topology_types TopologyType = draw_topology_types::TriangleList;
};

class rendering_component : public component
{

public:

	rendering_component(s32 ActorCompID)
	{
		ActorComponentID = ActorCompID;
	}

	mesh_render_resources RenderResources;

	s32 ActorComponentID = -1;

	inline const transform GetTransform()
	{
		return Transform;
	}

	inline v3 GetLocation()
	{
		return Transform.Location;
	}

	inline rotator GetRotation()
	{
		return Transform.Rotation;
	}

	inline v3 GetScale()
	{
		return Transform.Scale;
	}

	inline void SetTransform(transform _Transform)
	{
		Transform = _Transform;
		WorldMatrix = renderer::GenerateWorldMatrix(Transform);
	}

	inline void SetLocation(v3 Location) // set physics body location as well
	{
		Transform.Location = Location;
		WorldMatrix = renderer::GenerateWorldMatrix(Transform);
	}

	inline void SetRotation(rotator Rotation)
	{
		Transform.Rotation = Rotation;
		WorldMatrix = renderer::GenerateWorldMatrix(Transform);
	}

	inline void SetScale(v3 Scale)
	{
		Transform.Scale = Scale;
		WorldMatrix = renderer::GenerateWorldMatrix(Transform);
	}

	inline matrix4x4& GetWorldMatrix()
	{
		return WorldMatrix;
	}

	inline void SetWorldMatrix(matrix4x4 Matrix)
	{
		WorldMatrix = Matrix;
	}

private:

	matrix4x4 WorldMatrix;
	transform Transform;

};
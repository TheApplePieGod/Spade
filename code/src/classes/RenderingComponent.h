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
		UpdateMatrices();
	}

	inline void SetLocation(v3 Location) // set physics body location as well
	{
		Transform.Location = Location;
		UpdateMatrices();
	}

	inline void SetRotation(rotator Rotation)
	{
		Transform.Rotation = Rotation;
		UpdateMatrices();
	}

	inline void SetScale(v3 Scale)
	{
		Transform.Scale = Scale;
		UpdateMatrices();
	}

	inline matrix4x4& GetWorldMatrix()
	{
		return WorldMatrix;
	}

	inline matrix4x4& GetInverseWorldMatrix()
	{
		return InverseWorldMatrix;
	}

	inline void SetWorldMatrix(matrix4x4 Matrix)
	{
		WorldMatrix = Matrix;
	}

	inline void SetITPWorldMatrix(matrix4x4 Matrix)
	{
		InverseWorldMatrix = Matrix;
	}

private:

	inline void UpdateMatrices()
	{
		WorldMatrix = renderer::GenerateWorldMatrix(Transform);
		InverseWorldMatrix = renderer::InverseMatrix(WorldMatrix, false);
	}

	matrix4x4 WorldMatrix;
	matrix4x4 InverseWorldMatrix; // inverse transpose (normal transformations)
	transform Transform;

};
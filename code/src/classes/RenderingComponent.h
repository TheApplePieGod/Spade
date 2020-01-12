#pragma once
#include "Material.h"
#include "Component.h"

struct mesh_render_resources
{
	cMeshAsset* MeshAsset = nullptr;
	draw_topology_types TopologyType = draw_topology_types::TriangleList;
	s32 MaterialID = -1;
	transform LocalTransform;
};

class rendering_component : public component
{

public:

	mesh_render_resources RenderResources;

};
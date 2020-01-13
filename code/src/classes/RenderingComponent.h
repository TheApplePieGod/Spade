#pragma once
#include "Material.h"
#include "Component.h"

struct mesh_render_resources
{
	s32 MeshAssetID = -1;
	draw_topology_types TopologyType = draw_topology_types::TriangleList;
	s32 MaterialID = -1;
	transform LocalTransform;
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

};
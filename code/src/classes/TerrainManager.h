#pragma once

struct terrain_chunk
{
	vertex Vertices[4];
	v3 Midpoint;
};

class planet_terrain_manager
{
public:

	void Initialize(cMeshAsset* _PlanetMesh, f32 PlanetRadius);
	std::vector<terrain_chunk> ChunkArray;

private:

	cMeshAsset* PlanetMesh;

};
#include "pch.h"
#include "MathTypes.h"
#include "TerrainManager.h"

void planet_terrain_manager::Initialize(cMeshAsset* _PlanetMesh, f32 PlanetRadius)
{
	Assert(_PlanetMesh->Loaded);

	PlanetMesh = _PlanetMesh;

	vertex* PlanetVertices = (vertex*)PlanetMesh->Data;
	u32* PlanetIndices = (u32*)(PlanetVertices + PlanetMesh->MeshData.NumVertices);
	for (u32 i = 0; i < PlanetMesh->MeshData.NumIndices; i += 6)
	{
		terrain_chunk Chunk;

		Chunk.Vertices[0] = PlanetVertices[PlanetIndices[i]];
		Chunk.Vertices[1] = PlanetVertices[PlanetIndices[i + 1]];
		Chunk.Vertices[2] = PlanetVertices[PlanetIndices[i + 2]];
		Chunk.Vertices[3] = PlanetVertices[PlanetIndices[i + 4]];

		//Chunk.Vertices[0].u = 0.f; Chunk.Vertices[0].v = 0.f;
		//Chunk.Vertices[1].u = 1.f; Chunk.Vertices[1].v = 1.f;
		//Chunk.Vertices[2].u = 0.f; Chunk.Vertices[2].v = 1.f;
		//Chunk.Vertices[3].u = 1.f; Chunk.Vertices[3].v = 0.f;

		vertex v1 = Chunk.Vertices[0];
		v1.x *= PlanetRadius;
		v1.y *= PlanetRadius;
		v1.z *= PlanetRadius;
		vertex v2 = Chunk.Vertices[1];
		v2.x *= PlanetRadius;
		v2.y *= PlanetRadius;
		v2.z *= PlanetRadius;
		Chunk.Midpoint = v3{ (v1.x + v2.x) * 0.5f, (v1.y + v2.y) * 0.5f, (v1.z + v2.z) * 0.5f };

		ChunkArray.push_back(Chunk);
	}
}
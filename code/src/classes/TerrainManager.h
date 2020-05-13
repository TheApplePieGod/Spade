#pragma once

struct terrain_chunk
{
	std::vector<vertex> Vertices;
	std::vector<u32> Indices;
	v3 Midpoint;
	bool Generated = false;
	bool RenderSubChunk = true;
	int FaceDivision;
	byte Direction;
	byte CurrentLOD = 255;
};

class planet_terrain_manager
{
public:

	void Initialize(cMeshAsset* _PlanetMesh, f32 _PlanetRadius);
	void Initialize(f32 _PlanetRadius);
	void GenerateChunkIndices(int LOD, terrain_chunk& Chunk);
	void GenerateChunkVerticesAndIndices(int LOD, terrain_chunk& Chunk, bool Noise);
	std::vector<terrain_chunk> ChunkArray;
	std::vector<u32> VisibleChunkIDs;

	std::vector<vertex> LowLODVertices;
	std::vector<u32> LowLODIndices;

	std::mutex ChunkDataSwapMutex;
	std::mutex VisibleChunkSwapMutex;

	bool UpdatingChunkData = false;
	bool CombiningLowLOD = false;

	inline f32 GetPlanetRadius()
	{
		return PlanetRadius;
	}

	bool IsChunkVisible(const terrain_chunk& Chunk, v3 CameraPosition);

private:

	f32 PlanetRadius = 0.f;
	const int FaceDivision = 80;
	const int Resolution = 73;
	const int MapSeed = 1337;
	cMeshAsset* PlanetMesh;

};
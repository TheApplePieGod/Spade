#pragma once
#include "FreeList.h"

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

struct binary_terrain_chunk
{
	// 0 
	// 2 1
	vertex Vertices[3];
	//u32 Indices[6] = { 1, 2, 0, 2, 1, 3 };
	v3 Midpoint;
};

struct binary_node
{
	int FirstChildIndex = -1;
	bool IsLeaf = true;
	byte Depth = 0;
};

struct binary_tree
{
	std::vector<binary_node> Nodes;
	free_list<binary_terrain_chunk> ChunkData;
	byte MaxDepth = 10;
	int FirstFreeNode = -1;

	binary_tree(vertex InitialVertices[3]);

	// returns index to first new child node
	int SplitNode(binary_node Parent);

	// returns a list of indexes into the ChunkData array
	std::vector<int> Traverse(v3 CameraPosition, f32 LodSwitchIncrement);
};

class planet_terrain_manager
{
public:

	void Initialize(cMeshAsset* _PlanetMesh, f32 _PlanetRadius);
	void Initialize(f32 _PlanetRadius);
	void GenerateChunkIndices(int LOD, terrain_chunk& Chunk);
	void GenerateChunkVerticesAndIndices(int LOD, terrain_chunk& Chunk, bool Noise);
	static f32 GetTerrainNoise(v3 Location);

	std::vector<terrain_chunk> ChunkArray;
	std::vector<u32> VisibleChunkIDs;

	std::vector<vertex> LowLODVertices;
	std::vector<u32> LowLODIndices;

	std::mutex ChunkDataSwapMutex;
	std::mutex VisibleChunkSwapMutex;

	bool UpdatingChunkData = false;
	bool CombiningLowLOD = false;

	std::vector<binary_tree> Trees;

	inline f32 GetPlanetRadius()
	{
		return PlanetRadius;
	}

	bool IsChunkVisible(const terrain_chunk& Chunk, v3 CameraPosition);

private:

	f32 PlanetRadius = 0.f;
	static const int FaceDivision = 80;
	static const int Resolution = 73;
	static const int MapSeed = 1337;
	cMeshAsset* PlanetMesh;

};
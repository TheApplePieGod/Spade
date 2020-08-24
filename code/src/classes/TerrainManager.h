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

	// unloads with the node that it was force split by
	int ForceSplitBy = -1;
	s8 ForceSplitByTree = -1;

	bool JustSplit = false;

	bool Free = false;

	int LeftNeighbor = -1;
	s8 LeftNeighborTree = -1;
	int RightNeighbor = -1;
	s8 RightNeighborTree = -1;
	int BottomNeighbor = -1;
	s8 BottomNeighborTree = -1;
};

struct binary_tree
{
	v3 TreeMidpoint;
	std::vector<binary_node> Nodes;
	free_list<binary_terrain_chunk> ChunkData;
	std::vector<int> NodesToRender;
	byte MaxDepth = 26;//24;
	byte MinDepth = 12;//8;
	int FirstFreeNode = -1;

	binary_tree(vertex InitialVertices[3], s8 LeftTreeNeighbor, s8 RightTreeNeighbor, s8 BottomTreeNeighbor);

	// returns node index
	int RayIntersectsTriangle(v3 RayDirection, v3 RayOrigin, f32 PlanetRadius);
};

class planet_terrain_manager
{
public:

	void Initialize(f32 _PlanetRadius);

	int SplitNode(int Parent, s8 TreeIndex);
	void SmartSplitNode(int Parent, s8 TreeIndex, std::vector<std::array<int, 2>>& ToProcess);
	void CombineNodes(int Parent, s8 TreeIndex, bool Force);
	void Traverse(v3 CameraPosition, s8 TreeIndex, f32 LodSwitchIncrement);

	static f32 GetTerrainNoise(v3 Location);
	static v3 GetTerrainColor(v3 Location);

	std::vector<vertex> TerrainVertices;

	std::mutex TerrainVerticesSwapMutex;

	bool UpdatingChunkData = false;

	std::vector<binary_tree> Trees;

	inline f32 GetPlanetRadius()
	{
		return PlanetRadius;
	}

private:

	static const int MapSeed = 1337;
	f32 PlanetRadius = 0.f;
};
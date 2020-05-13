#include "pch.h"
#include "MathUtils.h"
#include "TerrainManager.h"

binary_tree::binary_tree(vertex InitialVertices[3])
{
	binary_node RootNode;

	v3 Mid = Midpoint(InitialVertices[0], InitialVertices[1], InitialVertices[2]);

	binary_terrain_chunk InitialData = { { InitialVertices[0], InitialVertices[1], InitialVertices[2] }, Mid };

	RootNode.FirstChildIndex = ChunkData.Insert(InitialData);

	Nodes.push_back(RootNode);
}

int binary_tree::SplitNode(binary_node Parent)
{
	int ParentNodeDataIndex = Parent.FirstChildIndex;
	const binary_terrain_chunk& Data = ChunkData[ParentNodeDataIndex];

	vertex NewVertex = Midpoint(Data.Vertices[0], Data.Vertices[1]); // becomes vertex 2 of the new nodes;
	
	binary_node NewNode1 = { -1, true, static_cast<byte>(Parent.Depth + 1) };
	binary_node NewNode2 = { -1, true, static_cast<byte>(Parent.Depth + 1) };

	bool Reverse = ((Parent.Depth + 1) % 2 == 1);
	binary_terrain_chunk NewData1 = { { Data.Vertices[2], Data.Vertices[0], NewVertex }, Midpoint(Data.Vertices[0], Data.Vertices[2], NewVertex) };
	binary_terrain_chunk NewData2 = { { Data.Vertices[1], Data.Vertices[2], NewVertex }, Midpoint(Data.Vertices[1], Data.Vertices[2], NewVertex) };

	ChunkData.Erase(ParentNodeDataIndex);
	NewNode1.FirstChildIndex = ChunkData.Insert(NewData1);
	NewNode2.FirstChildIndex = ChunkData.Insert(NewData2);

	int NewNodeIndex = -1;
	if (FirstFreeNode == -1)
	{
		NewNodeIndex = static_cast<int>(Nodes.size());
		Nodes.push_back(NewNode1);
		Nodes.push_back(NewNode2);
	}
	else
	{
		NewNodeIndex = FirstFreeNode;
		FirstFreeNode = Nodes[FirstFreeNode].FirstChildIndex;
		Nodes[FirstFreeNode] = NewNode1;
		Nodes[FirstFreeNode + 1] = NewNode2;
	}

	return NewNodeIndex;
}

std::vector<int> binary_tree::Traverse(v3 CameraPosition, f32 LodSwitchIncrement)
{
	f32 StartingDistance = LodSwitchIncrement;//* MaxDepth;
	std::vector<int> ToProcess;
	std::vector<int> OutIndexes;
	ToProcess.push_back(0); // root

	while (ToProcess.size() > 0)
	{
		int NodeIndex = ToProcess[ToProcess.size() - 1];
		ToProcess.pop_back();

		f32 LodDistance = StartingDistance * powf(0.75f, Nodes[NodeIndex].Depth);

		if (Nodes[NodeIndex].IsLeaf)
		{
			if (static_cast<byte>(Nodes[NodeIndex].Depth + 1) <= MaxDepth && Length(CameraPosition - ChunkData[Nodes[NodeIndex].FirstChildIndex].Midpoint) < LodDistance)
			{
				int ChildIndex = SplitNode(Nodes[NodeIndex]);
				Nodes[NodeIndex].IsLeaf = false;
				Nodes[NodeIndex].FirstChildIndex = ChildIndex;
				ToProcess.push_back(ChildIndex);
				ToProcess.push_back(ChildIndex + 1);
			}
			else
				OutIndexes.push_back(Nodes[NodeIndex].FirstChildIndex);
		}
		else
		{
			ToProcess.push_back(Nodes[NodeIndex].FirstChildIndex);
			ToProcess.push_back(Nodes[NodeIndex].FirstChildIndex + 1);
		}
	}

	return OutIndexes;
}

bool planet_terrain_manager::IsChunkVisible(const terrain_chunk& Chunk, v3 CameraPosition)
{
	v3 ChunkNormal = Normalize(Chunk.Midpoint);
	v3 PositionNormal = Normalize(CameraPosition);
	f32 DotProd = DotProduct(PositionNormal, ChunkNormal);
	f32 Angle = acos(DotProd);
	f32 MaxAngle = min((Length(CameraPosition) / GetPlanetRadius()) * (Pi32 * 0.2f), Pi32 * 0.35f);
	if (Angle < MaxAngle)
		return true;
	else
		return false;
}

void planet_terrain_manager::GenerateChunkVerticesAndIndices(int LOD, terrain_chunk& Chunk, bool _Noise)
{
	if (Chunk.CurrentLOD != LOD)
	{
		Chunk.CurrentLOD = LOD;

		if (LOD == -1)
		{
			Chunk.Vertices.clear();
			Chunk.Indices.clear();
			return;
		}

		srand(MapSeed);
		FastNoise Noise;
		f32 NoiseScale = 0.005f;
		Noise.SetSeed(MapSeed);
		Noise.SetFrequency(60.f);
		Noise.SetFractalOctaves(4);
		Noise.SetNoiseType(FastNoise::PerlinFractal);

		v3 Directions[6] = { directions::Up, directions::Down, directions::Left, directions::Right, directions::Forward, directions::Backward };
		v3 UpDirection = Directions[Chunk.Direction];
		f32 InvFace = (1.f / FaceDivision);

		int LodScaleFactor = (LOD == 0 ? 1 : LOD * 2);
		int VerticesPerLine = (Resolution - 1) / LodScaleFactor + 1;
		u32 ScaledRes = (Resolution * LodScaleFactor);

		v3 AxisA = v3{ UpDirection.y, UpDirection.z, UpDirection.x };
		v3 AxisB = CrossProduct(UpDirection, AxisA);

		f32 xOffset = (1.f - InvFace);
		f32 yOffset = (1.f - InvFace);
		f32 OffsetChange = (1.f - InvFace) / (FaceDivision - 1) * 2.f;
		for (int d = 0; d < FaceDivision * FaceDivision; d++)
		{
			if (d % FaceDivision == 0 && d != 0)
			{
				xOffset = (1.f - InvFace);
				yOffset -= OffsetChange;
			}

			if (d == Chunk.FaceDivision)
			{
				std::vector<vertex> NewVertices;
				std::vector<u32> NewIndices;

				NewVertices.resize(VerticesPerLine * VerticesPerLine);
				NewIndices.resize(VerticesPerLine * VerticesPerLine * 6);

				u32 Index = 0;
				u32 IndicesIndex = 0;
				for (int y = 0; y < Resolution; y += LodScaleFactor)
				{
					for (int x = 0; x < Resolution; x += LodScaleFactor)
					{
						v2 Percent = v2{ (f32)x, (f32)y } / (f32)(Resolution - 1);
						v3 AxisAPosition = AxisA * (((Percent.x - 0.5f) * 2 * InvFace) + xOffset);
						v3 AxisBPosition = AxisB * (((Percent.y - 0.5f) * 2 * InvFace) + yOffset);
						v3 VertexPosition = Normalize(UpDirection + AxisAPosition + AxisBPosition);
						v3 Normal = VertexPosition;
						f32 NoiseValue = Noise.GetNoise(VertexPosition.x, VertexPosition.y, VertexPosition.z) * NoiseScale;

						if (_Noise)
							VertexPosition += NoiseValue * Normal;

						vertex Vertex = vertex(VertexPosition.x, VertexPosition.y, VertexPosition.z, Percent.x, Percent.y, Normal.x, Normal.y, Normal.z);

						NewVertices[Index] = Vertex;

						if (x != Resolution - 1 && y != Resolution - 1) // not on the edges
						{
							NewIndices[IndicesIndex] = Index;
							NewIndices[IndicesIndex + 1] = (Index + VerticesPerLine + 1);
							NewIndices[IndicesIndex + 2] = (Index + VerticesPerLine);

							NewIndices[IndicesIndex + 3] = (Index);
							NewIndices[IndicesIndex + 4] = (Index + 1);
							NewIndices[IndicesIndex + 5] = (Index + VerticesPerLine + 1);

							IndicesIndex += 6;
						}

						Index++;
					}
				}

				//if (LOD == 0 || LOD == 2) // every other LOD level adjusts for the next one
				//{
				//	for (int i = 1; i < VerticesPerLine - 1; i += 2)
				//	{
				//		int BottomIndex = i + (VerticesPerLine * (VerticesPerLine - 1));
				//		NewVertices[i] = Midpoint(NewVertices[i - 1], NewVertices[i + 1]);
				//		NewVertices[BottomIndex] = Midpoint(NewVertices[BottomIndex - 1], NewVertices[BottomIndex + 1]);

				//		int LeftIndex = VerticesPerLine * i;
				//		int RightIndex = LeftIndex + VerticesPerLine - 1;
				//		NewVertices[LeftIndex] = Midpoint(NewVertices[LeftIndex - VerticesPerLine], NewVertices[LeftIndex + VerticesPerLine]);
				//		NewVertices[RightIndex] = Midpoint(NewVertices[RightIndex - VerticesPerLine], NewVertices[RightIndex + VerticesPerLine]);
				//	}
				//}

				ChunkDataSwapMutex.lock();
				std::swap(Chunk.Vertices, NewVertices);
				std::swap(Chunk.Indices, NewIndices);
				ChunkDataSwapMutex.unlock();

				Chunk.Generated = true;
				break;
			}

			xOffset -= OffsetChange;
		}

		srand((u32)time(NULL));
	}
}

void GenerateFace(int StartingDirection, int NumDirections, int Resolution, int FaceDivision, f32 PlanetRadius, terrain_chunk* ChunkArray, u32 StartingChunkIndex)
{
	v3 Directions[6] = { directions::Up, directions::Down, directions::Left, directions::Right, directions::Forward, directions::Backward };
	for (int i = StartingDirection; i < StartingDirection + NumDirections; i++)
	{
		u32 ChunkIndex = StartingChunkIndex;
		v3 UpDirection = Directions[i];
		f32 InvFace = (1.f / FaceDivision);

		v3 AxisA = v3{ UpDirection.y, UpDirection.z, UpDirection.x };
		v3 AxisB = CrossProduct(UpDirection, AxisA);

		f32 xOffset = (1.f - InvFace);
		f32 yOffset = (1.f - InvFace);
		f32 OffsetChange = (1.f - InvFace) / (FaceDivision - 1) * 2.f;
		for (int d = 0; d < FaceDivision * FaceDivision; d++)
		{
			terrain_chunk Chunk;
			Chunk.FaceDivision = d;
			Chunk.Direction = i;

			if (d % FaceDivision == 0 && d != 0)
			{
				xOffset = (1.f - InvFace);
				yOffset -= OffsetChange;
			}

			v3 AxisAMiddle = AxisA * xOffset;
			v3 AxisBMiddle = AxisB * yOffset;
			Chunk.Midpoint = Normalize(UpDirection + AxisAMiddle + AxisBMiddle) * PlanetRadius;

			xOffset -= OffsetChange;

			ChunkArray[ChunkIndex] = Chunk;
			ChunkIndex++;
		}
	}
}

void planet_terrain_manager::Initialize(f32 _PlanetRadius)
{
	PlanetRadius = _PlanetRadius;

	int NumThreads = std::thread::hardware_concurrency();
	if (NumThreads == 0)
		NumThreads = 1;
	if (NumThreads > 6)
		NumThreads = 6;

	ChunkArray.resize(FaceDivision * FaceDivision * Resolution);

	u32 StartingChunkIndex = 0;
	std::vector<std::thread> threads(NumThreads);
	for (int i = 0; i < NumThreads; i++)
	{
		if (i == NumThreads - 1)
		{
			threads[i] = std::thread(GenerateFace, i, 6 - i, Resolution, FaceDivision, PlanetRadius, ChunkArray.data(), StartingChunkIndex);
		}
		else
		{
			threads[i] = std::thread(GenerateFace, i, 1, Resolution, FaceDivision, PlanetRadius, ChunkArray.data(), StartingChunkIndex);
			StartingChunkIndex += FaceDivision * FaceDivision;
		}
	}

	for (auto& th : threads)
	{
		th.join();
	}

	float Scale = 1.f;
	vertex Vert0 = vertex(-0.5f * Scale, 0.5f * Scale, -0.5f * Scale, 0.f, 0.f);
	vertex Vert1 = vertex(0.5f * Scale, -0.5f * Scale, -0.5f * Scale, 1.f, 0.f);
	vertex Vert2 = vertex(-0.5f * Scale, -0.5f * Scale, -0.5f * Scale, 0.f, 1.f);

	vertex Verts[3] = { Vert0, Vert1, Vert2 };

	binary_tree Tree1 = binary_tree(Verts);
	Trees.push_back(Tree1);
}
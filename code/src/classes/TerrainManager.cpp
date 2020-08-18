#include "pch.h"
#include "MathUtils.h"
#include "TerrainManager.h"

binary_tree::binary_tree(vertex InitialVertices[3], s8 LeftTreeNeighbor, s8 RightTreeNeighbor, s8 BottomTreeNeighbor)
{
	binary_node RootNode;

	v3 Mid = Midpoint(InitialVertices[0], InitialVertices[1], InitialVertices[2]);

	binary_terrain_chunk InitialData = { { InitialVertices[0], InitialVertices[1], InitialVertices[2] }, Mid };

	RootNode.FirstChildIndex = ChunkData.Insert(InitialData);

	if (LeftTreeNeighbor != -1)
	{
		RootNode.LeftNeighbor = 0;
		RootNode.LeftNeighborTree = LeftTreeNeighbor;
	}
	if (RightTreeNeighbor != -1)
	{
		RootNode.RightNeighbor = 0;
		RootNode.RightNeighborTree = RightTreeNeighbor;
	}
	if (BottomTreeNeighbor != -1)
	{
		RootNode.BottomNeighbor = 0;
		RootNode.BottomNeighborTree = BottomTreeNeighbor;
	}

	Nodes.push_back(RootNode);
}

//void binary_tree::SmartSplitNode(int Parent, std::vector<int>& ToProcess)
//{
//	// recursive to make sure lods line up
//	if (Nodes[Parent].IsLeaf && Nodes[Parent].Free == false)
//	{
//		if (Nodes[Parent].BottomNeighbor != -1)
//		{
//			if (Nodes[Nodes[Parent].BottomNeighbor].BottomNeighbor != Parent)
//				SmartSplitNode(Nodes[Parent].BottomNeighbor, ToProcess);
//
//			int ChildIndex = SplitNode(Parent);
//			ToProcess.push_back(ChildIndex);
//			ToProcess.push_back(ChildIndex + 1);
//			if (Nodes[Nodes[Parent].BottomNeighbor].Free == false && Nodes[Nodes[Parent].BottomNeighbor].ForceSplitBy == -1 && Nodes[Nodes[Parent].BottomNeighbor].IsLeaf) // not already split
//			{
//				ChildIndex = SplitNode(Nodes[Parent].BottomNeighbor);
//
//				Nodes[Nodes[Parent].BottomNeighbor].ForceSplitBy = Parent;
//				ToProcess.push_back(ChildIndex);
//				ToProcess.push_back(ChildIndex + 1);
//
//				int RightChild = Nodes[Parent].FirstChildIndex;
//				int LeftChild = Nodes[Parent].FirstChildIndex + 1;
//
//				if (Nodes[Nodes[Parent].BottomNeighbor].IsLeaf == false)
//				{
//					Nodes[LeftChild].RightNeighbor = Nodes[Nodes[Parent].BottomNeighbor].FirstChildIndex;
//					Nodes[RightChild].LeftNeighbor = Nodes[Nodes[Parent].BottomNeighbor].FirstChildIndex + 1;
//				}
//				else
//					Assert(1 == 2); // something is bad
//
//				Nodes[Nodes[Nodes[Parent].BottomNeighbor].FirstChildIndex + 1].RightNeighbor = RightChild;
//				Nodes[Nodes[Nodes[Parent].BottomNeighbor].FirstChildIndex].LeftNeighbor = LeftChild;
//			}
//		}
//		else
//		{
//			// node 1 is the right child
//			int ChildIndex = SplitNode(Parent);
//			Nodes[ChildIndex].LeftNeighbor = -1;
//			Nodes[ChildIndex + 1].RightNeighbor = -1;
//
//			ToProcess.push_back(ChildIndex);
//			ToProcess.push_back(ChildIndex + 1);
//		}
//	}
//}
//
//int binary_tree::SplitNode(int Parent)
//{
//	int ParentNodeDataIndex = Nodes[Parent].FirstChildIndex;
//	const binary_terrain_chunk& Data = ChunkData[ParentNodeDataIndex];
//
//	vertex NewVertex = Normalize(Midpoint(Normalize(Data.Vertices[0]), Normalize(Data.Vertices[1]))); // becomes vertex 2 of the new nodes;
//	NewVertex.Position += NewVertex.Position * planet_terrain_manager::GetTerrainNoise(NewVertex.Position);
//	
//	byte NewDepth = static_cast<byte>(Nodes[Parent].Depth + 1);
//
//	// Node 1 is the right child
//	binary_node NewNode1 = { -1, true, NewDepth, -1, true };
//	binary_node NewNode2 = { -1, true, NewDepth, -1, true };
//
//	if (NewDepth > CurrentDepth)
//		CurrentDepth = NewDepth;
//
//	binary_terrain_chunk NewData1 = { { Data.Vertices[2], Data.Vertices[0], NewVertex }, Midpoint(Data.Vertices[0], Data.Vertices[2], NewVertex) * 998.f };
//	binary_terrain_chunk NewData2 = { { Data.Vertices[1], Data.Vertices[2], NewVertex }, Midpoint(Data.Vertices[1], Data.Vertices[2], NewVertex) * 998.f };
//
//	ChunkData.Erase(ParentNodeDataIndex);
//	NewNode1.FirstChildIndex = ChunkData.Insert(NewData1);
//	NewNode2.FirstChildIndex = ChunkData.Insert(NewData2);
//
//	int NewNodeIndex = -1;
//	if (FirstFreeNode == -1)
//	{
//		NewNodeIndex = static_cast<int>(Nodes.size());
//		Nodes.push_back(NewNode1);
//		Nodes.push_back(NewNode2);
//	}
//	else
//	{
//		NewNodeIndex = FirstFreeNode;
//		FirstFreeNode = Nodes[FirstFreeNode].FirstChildIndex;
//
//		Nodes[NewNodeIndex] = NewNode1;
//		Nodes[NewNodeIndex + 1] = NewNode2;
//	}
//
//	int LeftChild = NewNodeIndex + 1;
//	int RightChild = NewNodeIndex;
//
//	// set neighbors
//	Nodes[RightChild].RightNeighbor = LeftChild;
//	Nodes[LeftChild].LeftNeighbor = RightChild;
//	Nodes[RightChild].BottomNeighbor = Nodes[Parent].RightNeighbor;
//	Nodes[LeftChild].BottomNeighbor = Nodes[Parent].LeftNeighbor;
//
//	if (Nodes[Parent].LeftNeighbor != -1)
//	{
//		if (Nodes[Nodes[Parent].LeftNeighbor].BottomNeighbor == Parent)
//			Nodes[Nodes[Parent].LeftNeighbor].BottomNeighbor = LeftChild;
//		else
//		{
//			if (Nodes[Nodes[Parent].LeftNeighbor].LeftNeighbor == Parent)
//				Nodes[Nodes[Parent].LeftNeighbor].LeftNeighbor = LeftChild;
//			else
//				Nodes[Nodes[Parent].LeftNeighbor].RightNeighbor = LeftChild;
//		}
//	}
//	if (Nodes[Parent].RightNeighbor != -1)
//	{
//		if (Nodes[Nodes[Parent].RightNeighbor].BottomNeighbor == Parent)
//			Nodes[Nodes[Parent].RightNeighbor].BottomNeighbor = RightChild;
//		else
//		{
//			if (Nodes[Nodes[Parent].RightNeighbor].RightNeighbor == Parent)
//				Nodes[Nodes[Parent].RightNeighbor].RightNeighbor = RightChild;
//			else
//				Nodes[Nodes[Parent].RightNeighbor].LeftNeighbor = RightChild;
//		}
//	}
//
//	Nodes[Parent].IsLeaf = false;
//	Nodes[Parent].JustSplit = true;
//	Nodes[Parent].FirstChildIndex = NewNodeIndex;
//
//	return NewNodeIndex;
//}
//
//void binary_tree::CombineNodes(int Parent, bool Force)
//{
//	int LeftChild = Nodes[Parent].FirstChildIndex + 1;
//	int RightChild = Nodes[Parent].FirstChildIndex;
//
//	bool LeftValid = false;
//	bool RightValid = false;
//	bool BottomValid = false;
//
//	if (Nodes[LeftChild].BottomNeighbor == -1)
//		LeftValid = true;
//	else if (Nodes[Nodes[LeftChild].BottomNeighbor].Depth == Nodes[Parent].Depth || abs(Nodes[Nodes[LeftChild].BottomNeighbor].Depth - Nodes[Parent].Depth) == 1)
//		LeftValid = true;
//
//	if (Nodes[RightChild].BottomNeighbor == -1)
//		RightValid = true;
//	else if (Nodes[Nodes[RightChild].BottomNeighbor].Depth == Nodes[Parent].Depth || abs(Nodes[Nodes[RightChild].BottomNeighbor].Depth - Nodes[Parent].Depth) == 1)
//		RightValid = true;
//
//	if (Nodes[Parent].BottomNeighbor == -1)
//		BottomValid = true;
//	else if (Nodes[Nodes[Parent].BottomNeighbor].Depth == Nodes[Parent].Depth || abs(Nodes[Nodes[Parent].BottomNeighbor].Depth - Nodes[Parent].Depth) == 1)
//		BottomValid = true;
//
//	if (Force || (LeftValid && RightValid && BottomValid))
//	{
//		binary_terrain_chunk& NodeData1 = ChunkData[Nodes[RightChild].FirstChildIndex];
//		binary_terrain_chunk& NodeData2 = ChunkData[Nodes[LeftChild].FirstChildIndex];
//		binary_terrain_chunk CombinedData = { { NodeData1.Vertices[1], NodeData2.Vertices[0], NodeData1.Vertices[0] }, Midpoint(NodeData1.Vertices[1], NodeData2.Vertices[0], NodeData1.Vertices[0]) * 998.f };
//
//		ChunkData.Erase(Nodes[RightChild].FirstChildIndex);
//		ChunkData.Erase(Nodes[LeftChild].FirstChildIndex);
//
//		Nodes[RightChild].FirstChildIndex = FirstFreeNode;
//		FirstFreeNode = Nodes[Parent].FirstChildIndex;
//
//		Nodes[Parent].FirstChildIndex = ChunkData.Insert(CombinedData);
//		Nodes[Parent].ForceSplitBy = -1;
//		Nodes[Parent].IsLeaf = true;
//
//		// recalculate neighbors
//		int AllNeighbors[4] = { Nodes[RightChild].LeftNeighbor,
//								Nodes[LeftChild].RightNeighbor,
//								Nodes[RightChild].BottomNeighbor,
//								Nodes[LeftChild].BottomNeighbor };
//
//		Nodes[Parent].LeftNeighbor = Nodes[LeftChild].BottomNeighbor;
//		Nodes[Parent].RightNeighbor = Nodes[RightChild].BottomNeighbor;
//
//		//if (Nodes[RightChild].LeftNeighbor == Nodes[LeftChild].RightNeighbor) // incompatible depths
//		//	Nodes[Parent].BottomNeighbor = -1;
//		//else
//			//Nodes[Parent].BottomNeighbor = Nodes[RightChild].LeftNeighbor;
//
//		for (int i = 0; i < 4; i++)
//		{
//			int Index = AllNeighbors[i];
//			if (Index != -1)
//			{
//				//if (Nodes[Parent].Depth <= Nodes[Index].Depth)
//				//{
//				if (Nodes[Index].LeftNeighbor == LeftChild || Nodes[Index].LeftNeighbor == RightChild)
//					Nodes[Index].LeftNeighbor = Parent;
//				else if (Nodes[Index].RightNeighbor == LeftChild || Nodes[Index].RightNeighbor == RightChild)
//					Nodes[Index].RightNeighbor = Parent;
//				else
//					Nodes[Index].BottomNeighbor = Parent;
//				//}
//				//else
//				//{
//				//	if (Nodes[Index].LeftNeighbor == LeftChild || Nodes[Index].LeftNeighbor == RightChild)
//				//		Nodes[Index].LeftNeighbor = -1;
//				//	else if (Nodes[Index].RightNeighbor == LeftChild || Nodes[Index].RightNeighbor == RightChild)
//				//		Nodes[Index].RightNeighbor = -1;
//				//	else
//				//		Nodes[Index].BottomNeighbor = -1;
//				//}
//			}
//		}
//
//		if (Nodes[Parent].BottomNeighbor != -1)
//			if (Nodes[Nodes[Nodes[Parent].BottomNeighbor].FirstChildIndex].IsLeaf && Nodes[Nodes[Nodes[Parent].BottomNeighbor].FirstChildIndex + 1].IsLeaf)
//				if (Nodes[Nodes[Parent].BottomNeighbor].ForceSplitBy == Parent)
//					CombineNodes(Nodes[Parent].BottomNeighbor, false);
//
//		Nodes[RightChild] = binary_node();
//		Nodes[LeftChild] = binary_node();
//		Nodes[RightChild].Free = true;
//		Nodes[LeftChild].Free = true;
//	}
//}

int binary_tree::RayIntersectsTriangle(v3 RayDirection, v3 RayOrigin)
{
	std::vector<int> ToProcess;
	ToProcess.push_back(0); // root

	while (ToProcess.size() > 0)
	{
		int NodeIndex = ToProcess[ToProcess.size() - 1];
		ToProcess.pop_back();

		if (Nodes[NodeIndex].IsLeaf && !Nodes[NodeIndex].Free)
		{
			binary_terrain_chunk& NodeData = ChunkData[Nodes[NodeIndex].FirstChildIndex];
			v3 EdgeA = NodeData.Vertices[1].Position * 998.f - NodeData.Vertices[0].Position * 998.f;
			v3 EdgeB = NodeData.Vertices[2].Position * 998.f - NodeData.Vertices[0].Position * 998.f;
			v3 TriangleNormal = Normalize(CrossProduct(EdgeA, EdgeB));

			//if (fabs(DotProduct(TriangleNormal, RayDirection)) > 0.01)
			//{
			//	float D = DotProduct(TriangleNormal, NodeData.Vertices[0].Position * 998.f);
			//	float t = (DotProduct(TriangleNormal, RayOrigin) + D) / DotProduct(TriangleNormal, RayDirection);

			//	if (t > 0)
			//	{
			//		v3 IntersectionPoint = RayOrigin + (t * RayDirection);\
			//		v3 EdgeC = NodeData.Vertices[1].Position * 998.f - NodeData.Vertices[0].Position * 998.f;
			//		v3 C0 = IntersectionPoint - EdgeA;
			//		v3 C1 = IntersectionPoint - EdgeB;
			//		v3 C2 = IntersectionPoint - EdgeC;

			//		if (DotProduct(TriangleNormal, CrossProduct(EdgeA, C0)) > 0 &&
			//			DotProduct(TriangleNormal, CrossProduct(EdgeB, C1)) > 0 &&
			//			DotProduct(TriangleNormal, CrossProduct(EdgeC, C2)) > 0)
			//			return NodeIndex; // inside triangle
			//	}
			//}

			v3 pvec = CrossProduct(RayDirection, EdgeB);
			f32 det = DotProduct(EdgeA, pvec);

			if (fabs(det) > 0.01)
			{
				float invDet = 1.f / det;
				v3 tvec = RayOrigin - NodeData.Vertices[0].Position * 998.f;
				f32 u = DotProduct(tvec, pvec) * invDet;

				if (u > 0 && u < 1)
				{
					v3 qvec = CrossProduct(tvec, EdgeA);
					f32 v = DotProduct(RayDirection, qvec) * invDet;
					if (v > 0 && u + v < 1)
					{
						f32 t = DotProduct(EdgeB, qvec) * invDet;
						return NodeIndex;
					}
				}
			}
		}
		else
		{
			ToProcess.push_back(Nodes[NodeIndex].FirstChildIndex);
			ToProcess.push_back(Nodes[NodeIndex].FirstChildIndex + 1);
		}
	}
	return -1;
}

//std::vector<int> binary_tree::Traverse(v3 CameraPosition, f32 LodSwitchIncrement)
//{
//	f32 StartingDistance = LodSwitchIncrement;//* MaxDepth;
//	std::vector<int> ToProcess;
//	std::vector<int> OutIndexes;
//	ToProcess.push_back(0); // root
//	byte DeepestFoundDepth = MinDepth;
//
//	while (ToProcess.size() > 0)
//	{
//		int NodeIndex = ToProcess[ToProcess.size() - 1];
//		ToProcess.pop_back();
//
//		f32 LodDistance = StartingDistance * powf(0.75f, Nodes[NodeIndex].Depth);
//
//		if (Nodes[NodeIndex].IsLeaf)
//		{
//			if (Nodes[NodeIndex].Depth > DeepestFoundDepth)
//				DeepestFoundDepth = Nodes[NodeIndex].Depth;
//			if (Nodes[NodeIndex].JustSplit == false && Nodes[NodeIndex].Depth < MinDepth || (Nodes[NodeIndex].Depth + 1 <= MaxDepth && Length(CameraPosition - ChunkData[Nodes[NodeIndex].FirstChildIndex].Midpoint) < LodDistance))
//			{
//				SmartSplitNode(NodeIndex, ToProcess);
//				//int ChildIndex = SplitNode(NodeIndex);
//				//ToProcess.push_back(ChildIndex);
//				//ToProcess.push_back(ChildIndex + 1);
//			}
//			else
//			{
//				Nodes[NodeIndex].JustSplit = false;
//				OutIndexes.push_back(Nodes[NodeIndex].FirstChildIndex);
//			}
//		}
//		else
//		{
//			f32 CombineDistance = StartingDistance * powf(0.76f, Nodes[NodeIndex].Depth);
//			if (Nodes[NodeIndex].Depth >= CurrentDepth - 1 && Nodes[NodeIndex].JustSplit == false && Nodes[NodeIndex].Depth + 1 > MinDepth&& Nodes[Nodes[NodeIndex].FirstChildIndex].IsLeaf&& Nodes[Nodes[NodeIndex].FirstChildIndex + 1].IsLeaf) // if forcibly split to prevent seams, dont count as out of range
//			{
//				if (Nodes[NodeIndex].ForceSplitBy == -1)
//				{
//					f32 DistanceToLeaf1 = Length(CameraPosition - ChunkData[Nodes[Nodes[NodeIndex].FirstChildIndex].FirstChildIndex].Midpoint);
//					f32 DistanceToLeaf2 = Length(CameraPosition - ChunkData[Nodes[Nodes[NodeIndex].FirstChildIndex + 1].FirstChildIndex].Midpoint);
//
//					if (DistanceToLeaf1 > CombineDistance&& DistanceToLeaf2 > CombineDistance) // both leaves out of LOD range
//					{
//						CombineNodes(NodeIndex, false);
//						if (Nodes[NodeIndex].IsLeaf)
//							ToProcess.push_back(NodeIndex); // process node again to get new vertices
//					}
//					else // otherwise process both nodes as normal
//					{
//						ToProcess.push_back(Nodes[NodeIndex].FirstChildIndex);
//						ToProcess.push_back(Nodes[NodeIndex].FirstChildIndex + 1);
//					}
//				}
//				else
//				{
//					if (Nodes[Nodes[NodeIndex].ForceSplitBy].IsLeaf)
//					{
//						CombineNodes(NodeIndex, false);
//						if (Nodes[NodeIndex].IsLeaf)
//							ToProcess.push_back(NodeIndex); // process node again to get new vertices
//					}
//					else
//					{
//						ToProcess.push_back(Nodes[NodeIndex].FirstChildIndex);
//						ToProcess.push_back(Nodes[NodeIndex].FirstChildIndex + 1);
//					}
//				}
//			}
//			else
//			{
//				Nodes[NodeIndex].JustSplit = false;
//				ToProcess.push_back(Nodes[NodeIndex].FirstChildIndex);
//				ToProcess.push_back(Nodes[NodeIndex].FirstChildIndex + 1);
//			}
//		}
//	}
//
//	if (DeepestFoundDepth < CurrentDepth)
//		CurrentDepth = DeepestFoundDepth;
//
//	return OutIndexes;
//}

f32 planet_terrain_manager::GetTerrainNoise(v3 Location)
{
	srand(MapSeed);
	FastNoise Noise;
	f32 NoiseScale = 0.005f;
	Noise.SetSeed(MapSeed);
	Noise.SetFrequency(60.f);
	Noise.SetFractalOctaves(4);
	Noise.SetNoiseType(FastNoise::PerlinFractal);

	f32 NoiseValue = Noise.GetNoise(Location.x, Location.y, Location.z) * NoiseScale;

	srand((u32)time(NULL));

	return NoiseValue;
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

	//int NumThreads = std::thread::hardware_concurrency();
	//if (NumThreads == 0)
	//	NumThreads = 1;
	//if (NumThreads > 6)
	//	NumThreads = 6;

	//ChunkArray.resize(FaceDivision * FaceDivision * Resolution);

	//u32 StartingChunkIndex = 0;
	//std::vector<std::thread> threads(NumThreads);
	//for (int i = 0; i < NumThreads; i++)
	//{
	//	if (i == NumThreads - 1)
	//	{
	//		threads[i] = std::thread(GenerateFace, i, 6 - i, Resolution, FaceDivision, PlanetRadius, ChunkArray.data(), StartingChunkIndex);
	//	}
	//	else
	//	{
	//		threads[i] = std::thread(GenerateFace, i, 1, Resolution, FaceDivision, PlanetRadius, ChunkArray.data(), StartingChunkIndex);
	//		StartingChunkIndex += FaceDivision * FaceDivision;
	//	}
	//}

	//for (auto& th : threads)
	//{
	//	th.join();
	//}

	float Scale = 2.f;
	//0
	vertex Vert0 = Normalize(vertex(-0.5f * Scale, 0.5f * Scale, -0.5f * Scale, 0.f, 0.f));
	vertex Vert1 = Normalize(vertex(0.5f * Scale, -0.5f * Scale, -0.5f * Scale, 1.f, 1.f));
	vertex Vert2 = Normalize(vertex(-0.5f * Scale, -0.5f * Scale, -0.5f * Scale, 0.f, 1.f));
	//1
	vertex Vert3 = Vert1;
	vertex Vert4 = Vert0;
	vertex Vert5 = Normalize(vertex(0.5f * Scale, 0.5f * Scale, -0.5f * Scale, 1.f, 0.f));
	//2
	vertex Vert6 = Normalize(vertex(-0.5f * Scale, 0.5f * Scale, 0.5f * Scale, 0.f, 0.f));
	vertex Vert7 = Normalize(vertex(0.5f * Scale, 0.5f * Scale, -0.5f * Scale, 1.f, 1.f));
	vertex Vert8 = Normalize(vertex(-0.5f * Scale, 0.5f * Scale, -0.5f * Scale, 0.f, 1.f));
	//3
	vertex Vert9 = Vert7;
	vertex Vert10 = Vert6;
	vertex Vert11 = Normalize(vertex(0.5f * Scale, 0.5f * Scale, 0.5f * Scale, 1.f, 0.f));
	//4
	vertex Vert12 = Normalize(vertex(0.5f * Scale, 0.5f * Scale, -0.5f * Scale, 0.f, 0.f));
	vertex Vert13 = Normalize(vertex(0.5f * Scale, -0.5f * Scale, 0.5f * Scale, 1.f, 1.f));
	vertex Vert14 = Normalize(vertex(0.5f * Scale, -0.5f * Scale, -0.5f * Scale, 0.f, 1.f));
	//5
	vertex Vert15 = Vert13;
	vertex Vert16 = Vert12;
	vertex Vert17 = Vert11;
	//6
	vertex Vert18 = Normalize(vertex(-0.5f * Scale, -0.5f * Scale, -0.5f * Scale, 0.f, 0.f));
	vertex Vert19 = Normalize(vertex(0.5f * Scale, -0.5f * Scale, 0.5f * Scale, 1.f, 1.f));
	vertex Vert20 = Normalize(vertex(-0.5f * Scale, -0.5f * Scale, 0.5f * Scale, 0.f, 1.f));
	//7
	vertex Vert21 = Vert19;
	vertex Vert22 = Vert18;
	vertex Vert23 = Vert14;
	//8
	vertex Vert24 = Normalize(vertex(-0.5f * Scale, 0.5f * Scale, 0.5f * Scale, 0.f, 0.f));
	vertex Vert25 = Normalize(vertex(-0.5f * Scale, -0.5f * Scale, -0.5f * Scale, 1.f, 1.f));
	vertex Vert26 = Normalize(vertex(-0.5f * Scale, -0.5f * Scale, 0.5f * Scale, 0.f, 1.f));
	//9
	vertex Vert27 = Vert25;
	vertex Vert28 = Vert24;
	vertex Vert29 = Normalize(vertex(-0.5f * Scale, 0.5f * Scale, -0.5f * Scale, 1.f, 0.f));
	//10
	vertex Vert30 = Vert11;
	vertex Vert31 = Vert26;
	vertex Vert32 = Vert13;
	//11
	vertex Vert33 = Vert31;
	vertex Vert34 = Vert30;
	vertex Vert35 = Vert6;

	vertex Verts0[3] = { Vert0, Vert1, Vert2 };
	vertex Verts1[3] = { Vert3, Vert4, Vert5 };
	vertex Verts2[3] = { Vert6, Vert7, Vert8 };
	vertex Verts3[3] = { Vert9, Vert10, Vert11 };
	vertex Verts4[3] = { Vert12, Vert13, Vert14 };
	vertex Verts5[3] = { Vert15, Vert16, Vert17 };
	vertex Verts6[3] = { Vert18, Vert19, Vert20 };
	vertex Verts7[3] = { Vert21, Vert22, Vert23 };
	vertex Verts8[3] = { Vert24, Vert25, Vert26 };
	vertex Verts9[3] = { Vert27, Vert28, Vert29 };
	vertex Verts10[3] = { Vert30, Vert31, Vert32 };
	vertex Verts11[3] = { Vert33, Vert34, Vert35 };

	//binary_tree Tree0 = binary_tree(Verts0, -1, -1, -1);

	//binary_tree Tree0 = binary_tree(Verts0, -1, -1, 1);
	//binary_tree Tree1 = binary_tree(Verts1, -1, -1, 0);

	//binary_tree Tree0 = binary_tree(Verts0, -1, -1, 1);
	//binary_tree Tree1 = binary_tree(Verts1, 2, 4, 0);
	//binary_tree Tree2 = binary_tree(Verts2, 1, -1, 3);
	//binary_tree Tree3 = binary_tree(Verts3, -1, 5, 2);
	//binary_tree Tree4 = binary_tree(Verts4, -1, 1, 5);
	//binary_tree Tree5 = binary_tree(Verts5, 3, -1, 4);

	binary_tree Tree0 = binary_tree(Verts0, 7, 9, 1);
	binary_tree Tree1 = binary_tree(Verts1, 2, 4, 0);
	binary_tree Tree2 = binary_tree(Verts2, 1, 9, 3);
	binary_tree Tree3 = binary_tree(Verts3, 11, 5, 2);
	binary_tree Tree4 = binary_tree(Verts4, 7, 1, 5);
	binary_tree Tree5 = binary_tree(Verts5, 3, 10, 4);
	binary_tree Tree6 = binary_tree(Verts6, 10, 8, 7);
	binary_tree Tree7 = binary_tree(Verts7, 0, 4, 6);
	binary_tree Tree8 = binary_tree(Verts8, 6, 11, 9);
	binary_tree Tree9 = binary_tree(Verts9, 2, 0, 8);
	binary_tree Tree10 = binary_tree(Verts10, 6, 5, 11);
	binary_tree Tree11 = binary_tree(Verts11, 3, 8, 10);

	Trees.push_back(Tree0);
	Trees.push_back(Tree1);
	Trees.push_back(Tree2);
	Trees.push_back(Tree3);
	Trees.push_back(Tree4);
	Trees.push_back(Tree5);
	Trees.push_back(Tree6);
	Trees.push_back(Tree7);
	Trees.push_back(Tree8);
	Trees.push_back(Tree9);
	Trees.push_back(Tree10);
	Trees.push_back(Tree11);
}

void planet_terrain_manager::SmartSplitNode(int Parent, s8 TreeIndex, std::vector<std::array<int, 2>>& ToProcess)
{
	// recursive to make sure lods line up
	std::vector<binary_node>& Nodes = Trees[TreeIndex].Nodes;
	if (Nodes[Parent].IsLeaf && Nodes[Parent].Free == false)
	{
		if (Nodes[Parent].BottomNeighbor != -1 /*&& Nodes[Parent].BottomNeighborTree == TreeIndex*/)
		{
			if (!(Trees[Nodes[Parent].BottomNeighborTree].Nodes[Nodes[Parent].BottomNeighbor].BottomNeighbor == Parent && Trees[Nodes[Parent].BottomNeighborTree].Nodes[Nodes[Parent].BottomNeighbor].BottomNeighborTree == TreeIndex))
				SmartSplitNode(Nodes[Parent].BottomNeighbor, Nodes[Parent].BottomNeighborTree, ToProcess);

			int ChildIndex = SplitNode(Parent, TreeIndex);
			//if (TreeIndex == ProcessingTreeIndex)
			//{
				ToProcess.push_back({ ChildIndex, TreeIndex });
				ToProcess.push_back({ ChildIndex + 1, TreeIndex });
			//}

			binary_node& BottomNeighborNode = Trees[Nodes[Parent].BottomNeighborTree].Nodes[Nodes[Parent].BottomNeighbor];
			if (BottomNeighborNode.Free == false && BottomNeighborNode.ForceSplitBy == -1 && BottomNeighborNode.IsLeaf) // not already split
			{
				ChildIndex = SplitNode(Nodes[Parent].BottomNeighbor, Nodes[Parent].BottomNeighborTree);
				binary_node& UpdatedBottomNeighborNode = Trees[Nodes[Parent].BottomNeighborTree].Nodes[Nodes[Parent].BottomNeighbor]; //refresh reference after split

				UpdatedBottomNeighborNode.ForceSplitBy = Parent;
				UpdatedBottomNeighborNode.ForceSplitByTree = TreeIndex;
				//if (Nodes[Parent].BottomNeighborTree == ProcessingTreeIndex)
				//{
					ToProcess.push_back({ ChildIndex, Nodes[Parent].BottomNeighborTree });
					ToProcess.push_back({ ChildIndex + 1, Nodes[Parent].BottomNeighborTree });
				//}

				int RightChild = Nodes[Parent].FirstChildIndex;
				int LeftChild = Nodes[Parent].FirstChildIndex + 1;

				if (UpdatedBottomNeighborNode.IsLeaf == false)
				{
					Nodes[LeftChild].RightNeighbor = UpdatedBottomNeighborNode.FirstChildIndex;
					Nodes[LeftChild].RightNeighborTree = Nodes[Parent].BottomNeighborTree;
					Nodes[RightChild].LeftNeighbor = UpdatedBottomNeighborNode.FirstChildIndex + 1;
					Nodes[RightChild].LeftNeighborTree = Nodes[Parent].BottomNeighborTree;

					Trees[Nodes[Parent].BottomNeighborTree].Nodes[UpdatedBottomNeighborNode.FirstChildIndex + 1].RightNeighbor = RightChild;
					Trees[Nodes[Parent].BottomNeighborTree].Nodes[UpdatedBottomNeighborNode.FirstChildIndex + 1].RightNeighborTree = TreeIndex;
					Trees[Nodes[Parent].BottomNeighborTree].Nodes[UpdatedBottomNeighborNode.FirstChildIndex].LeftNeighbor = LeftChild;
					Trees[Nodes[Parent].BottomNeighborTree].Nodes[UpdatedBottomNeighborNode.FirstChildIndex].LeftNeighborTree = TreeIndex;
				}
				else
					Assert(1 == 2); // something is bad
			}
		}
		else
		{
			// node 1 is the right child
			int ChildIndex = SplitNode(Parent, TreeIndex);
			Nodes[ChildIndex].LeftNeighbor = -1;
			Nodes[ChildIndex].LeftNeighborTree = -1;
			Nodes[ChildIndex + 1].RightNeighbor = -1;
			Nodes[ChildIndex + 1].RightNeighborTree = -1;

			//if (TreeIndex == ProcessingTreeIndex)
			//{
				ToProcess.push_back({ ChildIndex, TreeIndex });
				ToProcess.push_back({ ChildIndex + 1, TreeIndex });
			//}
		}
	}
}

int planet_terrain_manager::SplitNode(int Parent, s8 TreeIndex)
{
	std::vector<binary_node>& Nodes = Trees[TreeIndex].Nodes;
	free_list<binary_terrain_chunk>& ChunkData = Trees[TreeIndex].ChunkData;
	int& FirstFreeNode = Trees[TreeIndex].FirstFreeNode;

	int ParentNodeDataIndex = Nodes[Parent].FirstChildIndex;
	const binary_terrain_chunk& Data = ChunkData[ParentNodeDataIndex];

	vertex NewVertex = Normalize(Midpoint(Normalize(Data.Vertices[0]), Normalize(Data.Vertices[1]))); // becomes vertex 2 of the new nodes;
	NewVertex.Position += NewVertex.Position * planet_terrain_manager::GetTerrainNoise(NewVertex.Position);

	byte NewDepth = static_cast<byte>(Nodes[Parent].Depth + 1);

	// Node 1 is the right child
	binary_node NewNode1 = { -1, true, NewDepth, -1, -1, true };
	binary_node NewNode2 = { -1, true, NewDepth, -1, -1, true };

	//if (NewDepth > TreesCurrentDepth)
	//	TreesCurrentDepth = NewDepth;

	binary_terrain_chunk NewData1 = { { Data.Vertices[2], Data.Vertices[0], NewVertex }, Midpoint(Data.Vertices[0], Data.Vertices[2], NewVertex) * 998.f };
	binary_terrain_chunk NewData2 = { { Data.Vertices[1], Data.Vertices[2], NewVertex }, Midpoint(Data.Vertices[1], Data.Vertices[2], NewVertex) * 998.f };

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

		Nodes[NewNodeIndex] = NewNode1;
		Nodes[NewNodeIndex + 1] = NewNode2;
	}

	int LeftChild = NewNodeIndex + 1;
	int RightChild = NewNodeIndex;

	// set neighbors
	Nodes[RightChild].RightNeighbor = LeftChild;
	Nodes[RightChild].RightNeighborTree = TreeIndex;
	Nodes[LeftChild].LeftNeighbor = RightChild;
	Nodes[LeftChild].LeftNeighborTree = TreeIndex;
	Nodes[RightChild].BottomNeighbor = Nodes[Parent].RightNeighbor;
	Nodes[RightChild].BottomNeighborTree = Nodes[Parent].RightNeighborTree;
	Nodes[LeftChild].BottomNeighbor = Nodes[Parent].LeftNeighbor;
	Nodes[LeftChild].BottomNeighborTree = Nodes[Parent].LeftNeighborTree;

	if (Nodes[Parent].LeftNeighbor != -1)
	{
		binary_node& LeftNeighborNode = Trees[Nodes[Parent].LeftNeighborTree].Nodes[Nodes[Parent].LeftNeighbor];
		if (LeftNeighborNode.BottomNeighbor == Parent && LeftNeighborNode.BottomNeighborTree == TreeIndex)
			LeftNeighborNode.BottomNeighbor = LeftChild;
		else
		{
			if (LeftNeighborNode.LeftNeighbor == Parent && LeftNeighborNode.LeftNeighborTree == TreeIndex)
				LeftNeighborNode.LeftNeighbor = LeftChild;
			else
			{
				LeftNeighborNode.RightNeighbor = LeftChild;
				LeftNeighborNode.RightNeighborTree = TreeIndex;
			}
		}
	}
	if (Nodes[Parent].RightNeighbor != -1)
	{
		binary_node& RightNeighborNode = Trees[Nodes[Parent].RightNeighborTree].Nodes[Nodes[Parent].RightNeighbor];
		if (RightNeighborNode.BottomNeighbor == Parent && RightNeighborNode.BottomNeighborTree == TreeIndex)
			RightNeighborNode.BottomNeighbor = RightChild;
		else
		{
			if (RightNeighborNode.RightNeighbor == Parent && RightNeighborNode.RightNeighborTree == TreeIndex)
				RightNeighborNode.RightNeighbor = RightChild;
			else
			{
				RightNeighborNode.LeftNeighbor = RightChild;
				RightNeighborNode.LeftNeighborTree = TreeIndex;
			}
		}
	}

	Nodes[Parent].IsLeaf = false;
	Nodes[Parent].JustSplit = true;
	Nodes[Parent].FirstChildIndex = NewNodeIndex;

	return NewNodeIndex;
}

void planet_terrain_manager::CombineNodes(int Parent, s8 TreeIndex, bool Force)
{
	std::vector<binary_node>& Nodes = Trees[TreeIndex].Nodes;
	free_list<binary_terrain_chunk>& ChunkData = Trees[TreeIndex].ChunkData;
	int& FirstFreeNode = Trees[TreeIndex].FirstFreeNode;

	int LeftChild = Nodes[Parent].FirstChildIndex + 1;
	int RightChild = Nodes[Parent].FirstChildIndex;

	if (!Nodes[Parent].IsLeaf)
	{
		if (!Nodes[LeftChild].IsLeaf)
			CombineNodes(LeftChild, TreeIndex, false);

		if (!Nodes[RightChild].IsLeaf)
			CombineNodes(RightChild, TreeIndex, false);

		bool LeftValid = false;
		bool RightValid = false;
		bool BottomValid = false;


		if (Nodes[LeftChild].BottomNeighbor == -1)
			LeftValid = true;
		else
		{
			if (Trees[Nodes[LeftChild].BottomNeighborTree].Nodes[Nodes[LeftChild].BottomNeighbor].Depth == Nodes[Parent].Depth || Trees[Nodes[LeftChild].BottomNeighborTree].Nodes[Nodes[LeftChild].BottomNeighbor].Depth - Nodes[Parent].Depth == 1)
				if (Trees[Nodes[LeftChild].BottomNeighborTree].Nodes[Nodes[LeftChild].BottomNeighbor].ForceSplitBy == -1)
					//if (Trees[Nodes[LeftChild].BottomNeighborTree].Nodes[Nodes[LeftChild].BottomNeighbor].IsLeaf)
						LeftValid = true;
		}

		if (Nodes[RightChild].BottomNeighbor == -1)
			RightValid = true;
		else
		{
			if (Trees[Nodes[RightChild].BottomNeighborTree].Nodes[Nodes[RightChild].BottomNeighbor].Depth == Nodes[Parent].Depth || Trees[Nodes[RightChild].BottomNeighborTree].Nodes[Nodes[RightChild].BottomNeighbor].Depth - Nodes[Parent].Depth == 1)
				if (Trees[Nodes[RightChild].BottomNeighborTree].Nodes[Nodes[RightChild].BottomNeighbor].ForceSplitBy == -1)
					//if (Trees[Nodes[RightChild].BottomNeighborTree].Nodes[Nodes[RightChild].BottomNeighbor].IsLeaf)
						RightValid = true;
		}

		if (Nodes[Parent].BottomNeighbor == -1)
			BottomValid = true;
		else
		{
			std::vector<binary_node>& BottomNodes = Trees[Nodes[Parent].BottomNeighborTree].Nodes;
			if (BottomNodes[Nodes[Parent].BottomNeighbor].Depth == Nodes[Parent].Depth || BottomNodes[Nodes[Parent].BottomNeighbor].Depth - Nodes[Parent].Depth == 1)
				if (BottomNodes[Nodes[Parent].BottomNeighbor].IsLeaf || (BottomNodes[BottomNodes[Nodes[Parent].BottomNeighbor].FirstChildIndex].IsLeaf && BottomNodes[BottomNodes[Nodes[Parent].BottomNeighbor].FirstChildIndex + 1].IsLeaf))
					BottomValid = true;
		}

		if (Force || (LeftValid && RightValid && BottomValid))
		{
			binary_terrain_chunk& NodeData1 = ChunkData[Nodes[RightChild].FirstChildIndex];
			binary_terrain_chunk& NodeData2 = ChunkData[Nodes[LeftChild].FirstChildIndex];
			binary_terrain_chunk CombinedData = { { NodeData1.Vertices[1], NodeData2.Vertices[0], NodeData1.Vertices[0] }, Midpoint(NodeData1.Vertices[1], NodeData2.Vertices[0], NodeData1.Vertices[0]) * 998.f };

			ChunkData.Erase(Nodes[RightChild].FirstChildIndex);
			ChunkData.Erase(Nodes[LeftChild].FirstChildIndex);

			Nodes[RightChild].FirstChildIndex = FirstFreeNode;
			FirstFreeNode = Nodes[Parent].FirstChildIndex;

			Nodes[Parent].FirstChildIndex = ChunkData.Insert(CombinedData);
			Nodes[Parent].ForceSplitBy = -1;
			Nodes[Parent].ForceSplitByTree = -1;
			Nodes[Parent].IsLeaf = true;

			// recalculate neighbors
			int AllNeighbors[4] = { Nodes[RightChild].LeftNeighbor,
									Nodes[LeftChild].RightNeighbor,
									Nodes[RightChild].BottomNeighbor,
									Nodes[LeftChild].BottomNeighbor };
			int AllNeighborTrees[4] = { Nodes[RightChild].LeftNeighborTree,
										Nodes[LeftChild].RightNeighborTree,
										Nodes[RightChild].BottomNeighborTree,
										Nodes[LeftChild].BottomNeighborTree };

			Nodes[Parent].LeftNeighbor = Nodes[LeftChild].BottomNeighbor;
			Nodes[Parent].LeftNeighborTree = Nodes[LeftChild].BottomNeighborTree;
			Nodes[Parent].RightNeighbor = Nodes[RightChild].BottomNeighbor;
			Nodes[Parent].RightNeighborTree = Nodes[RightChild].BottomNeighborTree;

			//if (Nodes[RightChild].LeftNeighbor == Nodes[LeftChild].RightNeighbor) // incompatible depths
			//	Nodes[Parent].BottomNeighbor = -1;
			//else
				//Nodes[Parent].BottomNeighbor = Nodes[RightChild].LeftNeighbor;

			for (int i = 0; i < 4; i++)
			{
				int Index = AllNeighbors[i];
				int NTreeIndex = AllNeighborTrees[i];
				if (Index != -1)
				{
					//if (Nodes[Parent].Depth <= Nodes[Index].Depth)
					//{
					if ((Trees[NTreeIndex].Nodes[Index].LeftNeighbor == LeftChild && Trees[NTreeIndex].Nodes[Index].LeftNeighborTree == TreeIndex) || (Trees[NTreeIndex].Nodes[Index].LeftNeighbor == RightChild && Trees[NTreeIndex].Nodes[Index].LeftNeighborTree == TreeIndex))
						Trees[NTreeIndex].Nodes[Index].LeftNeighbor = Parent;
					else if ((Trees[NTreeIndex].Nodes[Index].RightNeighbor == LeftChild && Trees[NTreeIndex].Nodes[Index].RightNeighborTree == TreeIndex) || (Trees[NTreeIndex].Nodes[Index].RightNeighbor == RightChild && Trees[NTreeIndex].Nodes[Index].RightNeighborTree == TreeIndex))
						Trees[NTreeIndex].Nodes[Index].RightNeighbor = Parent;
					else
					{
						Trees[NTreeIndex].Nodes[Index].BottomNeighbor = Parent;
						Trees[NTreeIndex].Nodes[Index].BottomNeighborTree = TreeIndex;
					}
					//}
					//else
					//{
					//	if (Nodes[Index].LeftNeighbor == LeftChild || Nodes[Index].LeftNeighbor == RightChild)
					//		Nodes[Index].LeftNeighbor = -1;
					//	else if (Nodes[Index].RightNeighbor == LeftChild || Nodes[Index].RightNeighbor == RightChild)
					//		Nodes[Index].RightNeighbor = -1;
					//	else
					//		Nodes[Index].BottomNeighbor = -1;
					//}
				}
			}

			if (Nodes[Parent].BottomNeighbor != -1)
			{
				std::vector<binary_node>& BottomNeighborNodes = Trees[Nodes[Parent].BottomNeighborTree].Nodes;
				if (BottomNeighborNodes[BottomNeighborNodes[Nodes[Parent].BottomNeighbor].FirstChildIndex].IsLeaf && BottomNeighborNodes[BottomNeighborNodes[Nodes[Parent].BottomNeighbor].FirstChildIndex + 1].IsLeaf)
					if (BottomNeighborNodes[Nodes[Parent].BottomNeighbor].ForceSplitBy == Parent && BottomNeighborNodes[Nodes[Parent].BottomNeighbor].ForceSplitByTree == TreeIndex && BottomNeighborNodes[Nodes[Parent].BottomNeighbor].Depth)
						CombineNodes(Nodes[Parent].BottomNeighbor, Nodes[Parent].BottomNeighborTree, false);
			}

			Nodes[RightChild] = binary_node();
			Nodes[LeftChild] = binary_node();
			Nodes[RightChild].Free = true;
			Nodes[LeftChild].Free = true;
		}
	}
}

void planet_terrain_manager::Traverse(v3 CameraPosition, s8 TreeIndex, f32 LodSwitchIncrement)
{
	f32 StartingDistance = LodSwitchIncrement;//* MaxDepth;
	std::vector<std::array<int, 2>> ToProcess;
	ToProcess.push_back({ 0, TreeIndex }); // root

	while (ToProcess.size() > 0)
	{
		int NodeIndex = ToProcess[ToProcess.size() - 1][0];
		int ProcessingTree = ToProcess[ToProcess.size() - 1][1];
		ToProcess.pop_back();

		std::vector<binary_node>& Nodes = Trees[ProcessingTree].Nodes;
		free_list<binary_terrain_chunk>& ChunkData = Trees[ProcessingTree].ChunkData;
		std::vector<int>& OutIndexes = Trees[ProcessingTree].NodesToRender;
		byte& MaxDepth = Trees[ProcessingTree].MaxDepth;
		byte& MinDepth = Trees[ProcessingTree].MinDepth;

		f32 LodDistance = StartingDistance * powf(0.75f, Nodes[NodeIndex].Depth);

		if (Nodes[NodeIndex].IsLeaf)
		{
			//if (Nodes[NodeIndex].Depth > DeepestFoundDepth)
			//	DeepestFoundDepth = Nodes[NodeIndex].Depth;
			if ((Nodes[NodeIndex].JustSplit == false && Nodes[NodeIndex].Depth < MinDepth) || (Nodes[NodeIndex].Depth + 1 <= MaxDepth && Length(CameraPosition - ChunkData[Nodes[NodeIndex].FirstChildIndex].Midpoint) < LodDistance))
			{
				SmartSplitNode(NodeIndex, ProcessingTree, ToProcess);
				//int ChildIndex = SplitNode(NodeIndex, TreeIndex);
				//ToProcess.push_back({ ChildIndex, TreeIndex });
				//ToProcess.push_back({ ChildIndex + 1, TreeIndex });
			}
			else
			{
				Nodes[NodeIndex].JustSplit = false;
				OutIndexes.push_back(Nodes[NodeIndex].FirstChildIndex);
			}
		}
		else
		{
			bool AddToProcess = false;
			f32 CombineDistance = StartingDistance * powf(0.76f, Nodes[NodeIndex].Depth);
			if (Nodes[Nodes[NodeIndex].FirstChildIndex].IsLeaf && Nodes[Nodes[NodeIndex].FirstChildIndex + 1].IsLeaf)
			{
				f32 DistanceToLeaf1 = Length(CameraPosition - ChunkData[Nodes[Nodes[NodeIndex].FirstChildIndex].FirstChildIndex].Midpoint);
				f32 DistanceToLeaf2 = Length(CameraPosition - ChunkData[Nodes[Nodes[NodeIndex].FirstChildIndex + 1].FirstChildIndex].Midpoint);

				if (DistanceToLeaf1 > CombineDistance && DistanceToLeaf2 > CombineDistance) // both leaves out of LOD range
				{
					if (Nodes[NodeIndex].JustSplit == false && Nodes[NodeIndex].Depth + 1 > MinDepth)
					{
						if (Nodes[NodeIndex].ForceSplitBy == -1)
						{
							CombineNodes(NodeIndex, ProcessingTree, false);
							if (Nodes[NodeIndex].IsLeaf)
								ToProcess.push_back({ NodeIndex, ProcessingTree }); // process node again to get new vertices
							else
								AddToProcess = true;
						}
						else
						{
							if (Trees[Nodes[NodeIndex].ForceSplitByTree].Nodes[Nodes[NodeIndex].ForceSplitBy].IsLeaf/* || Trees[Nodes[NodeIndex].ForceSplitByTree].Nodes[Nodes[NodeIndex].ForceSplitBy].Depth < Nodes[NodeIndex].Depth*/)
							{
								CombineNodes(NodeIndex, ProcessingTree, false);
								if (Nodes[NodeIndex].IsLeaf)
									ToProcess.push_back({ NodeIndex, ProcessingTree }); // process node again to get new vertices
								else
									AddToProcess = true;
							}
							else
								AddToProcess = true;
						}
					}
					else
						AddToProcess = true;
				}
				else
					AddToProcess = true;
			}
			else
				AddToProcess = true;

			if (AddToProcess)
			{
				Nodes[NodeIndex].JustSplit = false;
				ToProcess.push_back({ Nodes[NodeIndex].FirstChildIndex, ProcessingTree });
				ToProcess.push_back({ Nodes[NodeIndex].FirstChildIndex + 1, ProcessingTree });
			}


		//	if (Nodes[NodeIndex].Depth >= TreesCurrentDepth - 1 && Nodes[NodeIndex].JustSplit == false && Nodes[NodeIndex].Depth + 1 > MinDepth && Nodes[Nodes[NodeIndex].FirstChildIndex].IsLeaf && Nodes[Nodes[NodeIndex].FirstChildIndex + 1].IsLeaf) // if forcibly split to prevent seams, dont count as out of range
		//	{
		//		if (Nodes[NodeIndex].ForceSplitBy == -1)
		//		{
		//			f32 DistanceToLeaf1 = Length(CameraPosition - ChunkData[Nodes[Nodes[NodeIndex].FirstChildIndex].FirstChildIndex].Midpoint);
		//			f32 DistanceToLeaf2 = Length(CameraPosition - ChunkData[Nodes[Nodes[NodeIndex].FirstChildIndex + 1].FirstChildIndex].Midpoint);

		//			if (DistanceToLeaf1 > CombineDistance && DistanceToLeaf2 > CombineDistance) // both leaves out of LOD range
		//			{
		//				CombineNodes(NodeIndex, ProcessingTree, false);
		//				if (Nodes[NodeIndex].IsLeaf)
		//					ToProcess.push_back({ NodeIndex, ProcessingTree }); // process node again to get new vertices
		//			}
		//			else // otherwise process both nodes as normal
		//				AddToProcess = true;
		//		}
		//		else
		//		{
		//			if (Trees[Nodes[NodeIndex].ForceSplitByTree].Nodes[Nodes[NodeIndex].ForceSplitBy].IsLeaf)
		//			{
		//				CombineNodes(NodeIndex, ProcessingTree, false);
		//				if (Nodes[NodeIndex].IsLeaf)
		//					ToProcess.push_back({ NodeIndex, ProcessingTree }); // process node again to get new vertices
		//			}
		//			else
		//				AddToProcess = true;
		//		}
		//	}
		//	else
		//		AddToProcess = true;

		//	if (AddToProcess)
		//	{
		//		Nodes[NodeIndex].JustSplit = false;
		//		ToProcess.push_back({ Nodes[NodeIndex].FirstChildIndex, ProcessingTree });
		//		ToProcess.push_back({ Nodes[NodeIndex].FirstChildIndex + 1, ProcessingTree });
		//	}
		}
	}
}
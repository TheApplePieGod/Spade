#include "pch.h"
#include "MathUtils.h"
#include "TerrainManager.h"

void planet_terrain_manager::GenerateChunkVerticesAndIndices(int LOD, terrain_chunk& Chunk)
{
	if (Chunk.CurrentLOD != LOD)
	{
		Chunk.CurrentLOD = LOD;
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

				for (int i = 1; i < VerticesPerLine - 1; i += 2)
				{
					int BottomIndex = i + (VerticesPerLine * (VerticesPerLine - 1));
					NewVertices[i] = Midpoint(NewVertices[i - 1], NewVertices[i + 1]);
					NewVertices[BottomIndex] = Midpoint(NewVertices[BottomIndex - 1], NewVertices[BottomIndex + 1]);
				}

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
}
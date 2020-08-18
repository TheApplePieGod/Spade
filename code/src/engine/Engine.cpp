#include "pch.h"
#include "Engine.h"
#include "AssetUtils.h"
#include "MathUtils.h"

#if PLATFORM_WINDOWS
#include "../startup/WindowsMain.cpp"
#endif

extern shader_constants_actor ActorConstants;
extern shader_constants_frame FrameConstants;
extern shader_constants_lighting LightingConstants;

void engine::Tick()
{
	ProcessUserInput();

	// Update lighting constants
	f32 Angle = DegreesToRadians(DebugData.SunAngle);
	LightingConstants.SunDirection = v3{ sin(Angle), 0.f, cos(Angle)};
	//DebugData.SunAngle += 0.05f * UserInputs.DeltaTime;
	Renderer.MapConstants(map_operation::Lighting);

	MainCamera.UpdateProjectionType(projection_type::Perspective);
	v2 MouseDelta = v2{ UserInputs.MouseDeltaX * MainCamera.MouseInputScale, UserInputs.MouseDeltaY * MainCamera.MouseInputScale };
	if (DebugData.FreeCam)
		MainCamera.ViewMatrix = renderer::GenerateViewMatrix(true, MainCamera.CameraInfo, MainCamera.LookAtVector, MainCamera.UpVector);
	else
		MainCamera.ViewMatrix = renderer::GeneratePlanetaryViewMatrix(true, MainCamera.CameraInfo, MouseDelta, MainCamera.ForwardVector, MainCamera.LookAtVector, MainCamera.UpVector);
	UserInputs.MousePosWorldSpace = renderer::GetWorldSpaceDirectionFromMouse(v2{ UserInputs.MousePosX, UserInputs.MousePosY }, &MainCamera);

	RenderScene();

	UpdateComponents(); // before scene render?

	RenderDebugWidgets();

	Renderer.FinishFrame();

	if (DebugData.SlowMode)
		Sleep(150);
}

void engine::Initialize(void* Window, int WindowWidth, int WindowHeight)
{
	ScreenSize = { (f32)WindowWidth, (f32)WindowHeight };
	MainLevel = level();
	MainCamera = camera((f32)WindowWidth, (f32)WindowHeight);
	MemoryManager = memory_manager();

	MemoryManager.Initialize();
	Renderer.Initialize(Window, WindowWidth, WindowHeight);
	MainLevel.Initialize();

	InitializeAssetSystem();

	//TerrainManager.Initialize((cMeshAsset*)AssetRegistry[GetAssetIDFromName("sphere.fbx")], 998.f);
	TerrainManager.Initialize(PlanetRadius);

	s32 ids[6];
	s32 id = GetTextureIDFromName("stars.png");
	ids[0] = id;
	ids[1] = id;
	ids[2] = id;
	ids[3] = id;
	ids[4] = id;
	ids[5] = id;

	Renderer.UpdateSkybox(ids);

	pipeline_state State = pipeline_state();
	State.VertexShaderID = GetShaderIDFromName("skyboxvs");
	State.PixelShaderID = GetShaderIDFromName("skyboxps");
	State.RasterizerState = rasterizer_state::DefaultCullFrontface;
	State.UniqueIdentifier = "DefaultPBR";
	PipelineStates.CreateComponent(State);

	State = pipeline_state();
	State.VertexShaderID = GetShaderIDFromName("mainvs");
	State.PixelShaderID = GetShaderIDFromName("mainps");
	State.RasterizerState = (DebugData.EnableWireframe ? rasterizer_state::Wireframe : rasterizer_state::DefaultCullBackface);
	State.UniqueIdentifier = "DefaultPBR";
	PipelineStates.CreateComponent(State);

	// todo: hardcoded default texture and material data
	for (u32 i = 0; i < 2; i++)
	{
		material Mat = material();
		switch (i)
		{
			default:
			{ Mat.DiffuseTextureID = GetTextureIDFromName("DefaultTexture.jpg");
			  Mat.Reflectivity = 1.f;
			  /*Mat.DiffuseColor = colors::Blue;*/ } break;

			//case 1:
			//{ Mat.DiffuseTextureID = GetTextureIDFromName("snowpath_diffuse.png");
			//  Mat.NormalTextureID = GetTextureIDFromName("snowpath_normal.png");
			//  Mat.DiffuseColor = colors::Red;
			//  Mat.Reflectivity = 5.f; } break;

			//case 2:
			//{ Mat.DiffuseTextureID = GetTextureIDFromName("eyes.jpg");
			//  /*Mat.DiffuseColor = colors::Green;*/ } break;

			//case 3:
			//{ Mat.DiffuseTextureID = GetTextureIDFromName("lmao.jpg");
			//  /*Mat.DiffuseColor = colors::Orange;*/ } break;

			//case 4:
			//{ Mat.DiffuseTextureID = GetTextureIDFromName("what.jpg");
			//  /*Mat.DiffuseColor = colors::White;*/ } break;

			case 5:
			{ Mat.DiffuseTextureID = GetTextureIDFromName("snowpath_diffuse.png");
			  Mat.NormalTextureID = GetTextureIDFromName("snowpath_normal.png");
			/*Mat.DiffuseColor = colors::White;*/ } break;
		}
		MaterialRegistry.CreateComponent(Mat);
	}

	actor_component acomp = actor_component(&MainLevel);
	s32 actorid = ActorComponents.CreateComponent(acomp, true);
	u32 ScaleMod = 1;
	u32 RotationMod = 360;
	u32 LocationMod = 3000;
	for (u32 i = 0; i < 1; i++)
	{
		rendering_component rcomp = rendering_component(actorid);
		if (i == 0)
		{
			rcomp.RenderResources.MaterialID = 1;
			rcomp.RenderResources.PipelineStateID = 0;
			rcomp.RenderResources.MeshAssetID = GetAssetIDFromName("cube_t.fbx");
		}
		else
		{
			rcomp.SetLocation(v3{ 0.f, 0.f, -1000.f });
			rcomp.SetScale(v3{ 100.f, 100.f, 100.f });
			rcomp.RenderResources.MaterialID = 0;
			rcomp.RenderResources.PipelineStateID = 1;
			rcomp.RenderResources.MeshAssetID = GetAssetIDFromName("cube_t.fbx");
		}

		rcomp.ActorComponentID = actorid;
		RenderingComponents.CreateComponent(rcomp, true);
	}

	LightingConstants.AmbientColor = v3{ 0.05f, 0.05f, 0.05f };
	LightingConstants.SunColor = colors::White;
	DebugData.SunAngle = 180.f;
}

void engine::Cleanup()
{
	if (ChunkUpdateThread.joinable())
		ChunkUpdateThread.join();
	Renderer.Cleanup();
}

void engine::InitializeAssetSystem()
{
	assetTypes::ImageType.LoadCallback = assetCallbacks::ImageLoadCallback;
	assetTypes::MeshType.LoadCallback = assetCallbacks::MeshLoadCallback;
	assetLoader::AddAssetType(assetTypes::ImageType);
	assetLoader::AddAssetType(assetTypes::MeshType);
	assetLoader::AddAssetType(assetTypes::MaterialType);

#if SPADE_DEBUG
	assetLoader::ScanAssets("assets\\", false, true);
	assetLoader::InitializeAssetsInDirectory("assets\\", true);
#else
	GetAssetSettings().LoadFromPack = true;
	assetLoader::InitializeAssetsFromPack();
#endif

	// initialize materials
	for (u32 i = 0; i < AssetRegistry.size(); i++)
	{
		if (AssetRegistry[i]->Type == MATERIAL_ASSET_ID) // material
			AssetRegistry[i]->LoadAssetData();
	}
}

void engine::ProcessUserInput()
{
	ImGuiIO io = ImGui::GetIO();
	UserInputs.GuiMouseFocus = io.WantCaptureMouse;
	UserInputs.GuiKeyboardFocus = io.WantCaptureKeyboard;

	f32 speed = DebugData.CameraSpeed * UserInputs.DeltaTime;
	v3 OldLocation = MainCamera.CameraInfo.Transform.Location;

	//if (Length(MainCamera.CameraInfo.Transform.Location) < 999.f)
	//	MainCamera.CameraInfo.Transform.Location = OldLocation;

	// move to platform layer / optimize
	if (UserInputs.KeysDown[VK_CONTROL].Pressed)
	{
		while (ShowCursor(true) <= 0);
	}
	else
	{
		MainCamera.UpdateFromInput();
		if (!UserInputs.GuiKeyboardFocus)
		{
			if (UserInputs.KeysDown['A'].Pressed)
			{
				MainCamera.CameraInfo.Transform.Location += -speed * MainCamera.RightVector;
				UserInputs.PlayerMovement = true;
			}
			if (UserInputs.KeysDown['D'].Pressed)
			{
				MainCamera.CameraInfo.Transform.Location += speed * MainCamera.RightVector;
				UserInputs.PlayerMovement = true;
			}
			if (UserInputs.KeysDown['W'].Pressed)
			{
				MainCamera.CameraInfo.Transform.Location += speed * MainCamera.ForwardVector;
				UserInputs.PlayerMovement = true;
			}
			if (UserInputs.KeysDown['S'].Pressed)
			{
				MainCamera.CameraInfo.Transform.Location += -speed * MainCamera.ForwardVector;
				UserInputs.PlayerMovement = true;
			}
		}
		while (ShowCursor(false) >= 0);
	}		
}

void UpdateVisibleChunks(planet_terrain_manager* TerrainManager, camera* MainCamera)
{
	if (!TerrainManager->UpdatingChunkData)
	{
		TerrainManager->UpdatingChunkData = true;

		std::vector<vertex> NewLowLODVertices;
		std::vector<u32> NewLowLODIndices;
		std::vector<u32> NewVisibleChunkIDs;

		u32 CurrentOffset = 0;
		u32 IndicesIndex = 0;

		for (u32 i = 0; i < (u32)TerrainManager->ChunkArray.size(); i++)
		{
			terrain_chunk& Chunk = TerrainManager->ChunkArray[i];
			if (TerrainManager->IsChunkVisible(Chunk, MainCamera->CameraInfo.Transform.Location))
			{
				f32 Distance = Length(MainCamera->CameraInfo.Transform.Location - TerrainManager->ChunkArray[i].Midpoint);
				//if (Distance <= 400.f)
				{
					NewVisibleChunkIDs.push_back(i);


					bool LowLOD = false;
					//if (Distance <= 25.f)
					//	TerrainManager->GenerateChunkVerticesAndIndices(0, Chunk, true);
					//else if (Distance < 50.f)
					//	TerrainManager->GenerateChunkVerticesAndIndices(1, Chunk, true);
					//else if (Distance < 100.f)
					//	TerrainManager->GenerateChunkVerticesAndIndices(2, Chunk, true);
					//else if (Distance < 150.f)
					//	TerrainManager->GenerateChunkVerticesAndIndices(4, Chunk, true);
					//else
					//{
					//	if (Distance < 300)
					//		Chunk.RenderSubChunk = true;
						TerrainManager->GenerateChunkVerticesAndIndices(36, Chunk, true);
					//}

					Chunk.RenderSubChunk = false;

					if (Chunk.RenderSubChunk || Chunk.CurrentLOD == 36)
					{
						terrain_chunk DummyChunk = Chunk;
						terrain_chunk* ChunkToUse = nullptr;

						if (Chunk.RenderSubChunk)
						{
							TerrainManager->GenerateChunkVerticesAndIndices(36, DummyChunk, true);
							ChunkToUse = &DummyChunk;
						}
						else
							ChunkToUse = &Chunk;

						NewLowLODVertices.insert(NewLowLODVertices.end(), ChunkToUse->Vertices.begin(), ChunkToUse->Vertices.end());
						NewLowLODIndices.resize(NewLowLODIndices.size() + ChunkToUse->Indices.size());
						for (u32 n = 0; n < ChunkToUse->Indices.size(); n++)
						{
							NewLowLODIndices[IndicesIndex] = ChunkToUse->Indices[n] + CurrentOffset;
							IndicesIndex++;
						}
						CurrentOffset += (u32)ChunkToUse->Vertices.size();
					}

					//if (LowLOD)
					//{
					//	NewLowLODVertices.insert(NewLowLODVertices.end(), Chunk.Vertices.begin(), Chunk.Vertices.end());
					//	NewLowLODIndices.resize(NewLowLODIndices.size() + Chunk.Indices.size());
					//	for (u32 n = 0; n < Chunk.Indices.size(); n++)
					//	{
					//		NewLowLODIndices[IndicesIndex] = Chunk.Indices[n] + CurrentOffset;
					//		IndicesIndex++;
					//	}
					//	CurrentOffset += (u32)Chunk.Vertices.size();
					//}
				}
			}
		}

		TerrainManager->VisibleChunkSwapMutex.lock();
		std::swap(TerrainManager->VisibleChunkIDs, NewVisibleChunkIDs);
		std::swap(TerrainManager->LowLODVertices, NewLowLODVertices);
		std::swap(TerrainManager->LowLODIndices, NewLowLODIndices);
		TerrainManager->VisibleChunkSwapMutex.unlock();

		TerrainManager->UpdatingChunkData = false;
	}
}

void UpdateVisibleChunksBT(planet_terrain_manager* TerrainManager, camera* MainCamera)
{
	if (!TerrainManager->UpdatingChunkData)
	{
		TerrainManager->UpdatingChunkData = true;
		std::vector<vertex> BTVertices;

		u32 VertexIndex = 0;
		//for (u32 n = 0; n < (u32)TerrainManager->Trees.size(); n++)
		//{
		//	std::vector<int> OutInts = TerrainManager->Trees[n].Traverse(MainCamera->CameraInfo.Transform.Location, 5000.f);
		//	BTVertices.resize(BTVertices.size() + OutInts.size() * 3);
		//	for (u32 i = 0; i < (u32)OutInts.size(); i++)
		//	{
		//		const binary_terrain_chunk& Data = TerrainManager->Trees[n].ChunkData[OutInts[i]];

		//		for (u32 d = 0; d < 3; d++)
		//		{
		//			BTVertices[VertexIndex] = Data.Vertices[d];
		//			VertexIndex++;
		//		}
		//		//BTVertices.insert(BTVertices.end(), Data.Vertices, Data.Vertices + 3);
		//	}
		//	//Renderer.DrawIndexedTerrainChunk(BTVertices.data(), BTIndices.data(), (u32)BTVertices.size(), (u32)BTIndices.size());
		//	//Renderer.Draw(BTVertices.data(), (u32)BTVertices.size(), draw_topology_type::TriangleList);
		//}
		if (Engine->DebugData.VisibleChunkUpdates)
		{
			for (u32 n = 0; n < (u32)TerrainManager->Trees.size(); n++)
			{
				TerrainManager->Trees[n].NodesToRender.clear();
			}
		}

		for (u32 n = 0; n < (u32)TerrainManager->Trees.size(); n++)
		{
			if (Engine->DebugData.VisibleChunkUpdates)
			{
				TerrainManager->Traverse(MainCamera->CameraInfo.Transform.Location, n, 5000.f);
			}
			//Renderer.DrawIndexedTerrainChunk(BTVertices.data(), BTIndices.data(), (u32)BTVertices.size(), (u32)BTIndices.size());
			//Renderer.Draw(BTVertices.data(), (u32)BTVertices.size(), draw_topology_type::TriangleList);
			int IntersectingIndex = TerrainManager->Trees[n].RayIntersectsTriangle(Engine->UserInputs.MousePosWorldSpace, MainCamera->CameraInfo.Transform.Location);
			if (IntersectingIndex != -1)
			{
				Engine->DebugData.IntersectingIndex = IntersectingIndex;
				Engine->DebugData.IntersectingTree = n;
			}
		}

		if (Engine->DebugData.VisibleChunkUpdates)
		{
			for (u32 n = 0; n < (u32)TerrainManager->Trees.size(); n++)
			{
				BTVertices.resize(BTVertices.size() + TerrainManager->Trees[n].NodesToRender.size() * 3);
				for (u32 i = 0; i < (u32)TerrainManager->Trees[n].NodesToRender.size(); i++)
				{
					const binary_terrain_chunk& Data = TerrainManager->Trees[n].ChunkData[TerrainManager->Trees[n].NodesToRender[i]];

					for (u32 d = 0; d < 3; d++)
					{
						BTVertices[VertexIndex] = Data.Vertices[d];
						VertexIndex++;
					}
					//BTVertices.insert(BTVertices.end(), Data.Vertices, Data.Vertices + 3);
				}
			}
		}

		if (Engine->DebugData.VisibleChunkUpdates)
		{
			TerrainManager->VisibleChunkSwapMutex.lock();
			std::swap(TerrainManager->LowLODVertices, BTVertices);
			TerrainManager->VisibleChunkSwapMutex.unlock();
		}

		TerrainManager->UpdatingChunkData = false;
	}
}

//always rendered first & has very specific rendering. move out of here later?
void engine::RenderPlanet()
{
	pipeline_state SkyFromSpace = pipeline_state();
	SkyFromSpace.VertexShaderID = GetShaderIDFromName("mainvs");
	SkyFromSpace.PixelShaderID = GetShaderIDFromName("SkyFromSpacePS");
	SkyFromSpace.RasterizerState = rasterizer_state::DefaultCullFrontface;
	SkyFromSpace.UniqueIdentifier = "DefaultPBR";

	pipeline_state GroundFromSpace = pipeline_state();
	GroundFromSpace.VertexShaderID = GetShaderIDFromName("mainvs");
	GroundFromSpace.PixelShaderID = GetShaderIDFromName("GroundFromSpacePS");
	//GroundFromSpace.DomainShaderID = GetShaderIDFromName("TerrainDomainShader");
	//GroundFromSpace.HullShaderID = GetShaderIDFromName("TerrainHullShader");
	//GroundFromSpace.EnableTesselation = true;
	GroundFromSpace.RasterizerState = (DebugData.EnableWireframe ? rasterizer_state::Wireframe : rasterizer_state::DefaultCullBackface);
	GroundFromSpace.UniqueIdentifier = "DefaultPBR";

	pipeline_state SkyFromAtmoshere = pipeline_state();
	SkyFromAtmoshere.VertexShaderID = GetShaderIDFromName("mainvs");
	SkyFromAtmoshere.PixelShaderID = GetShaderIDFromName("SkyFromAtmospherePS");
	SkyFromAtmoshere.RasterizerState = rasterizer_state::DefaultCullFrontface;
	SkyFromAtmoshere.UniqueIdentifier = "DefaultPBR";

	pipeline_state GroundFromAtmosphere = pipeline_state();
	GroundFromAtmosphere.VertexShaderID = GetShaderIDFromName("mainvs");
	GroundFromAtmosphere.PixelShaderID = GetShaderIDFromName("GroundFromAtmospherePS");
	//GroundFromAtmosphere.DomainShaderID = GetShaderIDFromName("TerrainDomainShader");
	//GroundFromAtmosphere.HullShaderID = GetShaderIDFromName("TerrainHullShader");
	//GroundFromAtmosphere.EnableTesselation = true;
	GroundFromAtmosphere.RasterizerState = (DebugData.EnableWireframe ? rasterizer_state::Wireframe : rasterizer_state::DefaultCullBackface);
	GroundFromAtmosphere.UniqueIdentifier = "DefaultPBR";

	bool InAtmosphere = true;
	if (Length(MainCamera.CameraInfo.Transform.Location) > 1025.f)
		InAtmosphere = false;

	cMeshAsset* PlanetMesh = (cMeshAsset*)AssetRegistry[GetAssetIDFromName("sphere.fbx")];
	v3 PlanetScale = v3{ PlanetRadius, PlanetRadius, PlanetRadius };

	transform AtmosphereTransform = transform();
	AtmosphereTransform.Scale = 1.025f * PlanetScale;
	ActorConstants.Instances[0].WorldMatrix = renderer::GenerateWorldMatrix(AtmosphereTransform); // todo: static
	Renderer.MapConstants(map_operation::Actor);

	if (InAtmosphere)
		Renderer.SetPipelineState(SkyFromAtmoshere);
	else
		Renderer.SetPipelineState(SkyFromSpace);

	//Renderer.DrawIndexedInstanced((vertex*)PlanetMesh->Data, (u32*)((vertex*)PlanetMesh->Data + PlanetMesh->MeshData.NumVertices), PlanetMesh->MeshData.NumVertices, PlanetMesh->MeshData.NumIndices, 0, 1, draw_topology_type::TriangleList);

	transform PlanetTransform = transform();
	PlanetTransform.Scale = PlanetScale;
	PlanetTransform.Rotation = v3{ 0.f, 0.f, 0.f };
	ActorConstants.Instances[0].WorldMatrix = renderer::GenerateWorldMatrix(PlanetTransform);
	Renderer.MapConstants(map_operation::Actor);

	if (InAtmosphere)
		Renderer.SetPipelineState(GroundFromAtmosphere);
	else
		Renderer.SetPipelineState(GroundFromSpace);

	f32 CamX = MainCamera.CameraInfo.Transform.Location.x;
	f32 CamY = MainCamera.CameraInfo.Transform.Location.y;
	f32 CamZ = MainCamera.CameraInfo.Transform.Location.z;
	v3 Direction = Normalize(MainCamera.CameraInfo.Transform.Location);

	//Renderer.DrawIndexedInstanced((vertex*)PlanetMesh->Data, (u32*)((vertex*)PlanetMesh->Data + PlanetMesh->MeshData.NumVertices), PlanetMesh->MeshData.NumVertices, PlanetMesh->MeshData.NumIndices, 0, 1, draw_topology_type::TriangleList);
	Renderer.BindMaterial(MaterialRegistry.GetComponent(0));

	if ((!TerrainManager.UpdatingChunkData) || ChunkUpdateThread.get_id() == std::thread::id())
	{
		if (ChunkUpdateThread.joinable())
			ChunkUpdateThread.join();
		ChunkUpdateThread = std::thread(UpdateVisibleChunksBT, &TerrainManager, &MainCamera);
	}
	
	//if (DebugData.VisibleChunkUpdates)
		//UpdateVisibleChunksBT(&TerrainManager, &MainCamera);

	//binary tree test
	pipeline_state BinaryTreeTest = pipeline_state();
	BinaryTreeTest.VertexShaderID = GetShaderIDFromName("mainvs");
	BinaryTreeTest.PixelShaderID = GetShaderIDFromName("mainps");
	BinaryTreeTest.RasterizerState = (DebugData.EnableWireframe ? rasterizer_state::Wireframe : rasterizer_state::DefaultCullBackface);
	BinaryTreeTest.UniqueIdentifier = "DefaultPBR";
	Renderer.SetPipelineState(BinaryTreeTest);

	TerrainManager.VisibleChunkSwapMutex.lock();
	if (TerrainManager.LowLODVertices.size() < 200000)
		Renderer.Draw(TerrainManager.LowLODVertices.data(), (u32)TerrainManager.LowLODVertices.size(), draw_topology_type::TriangleList);
	DebugData.NumTerrainVertices = (u32)TerrainManager.LowLODVertices.size();
	TerrainManager.VisibleChunkSwapMutex.unlock();

	//------

	//TerrainManager.VisibleChunkSwapMutex.lock();
	//TerrainManager.ChunkDataSwapMutex.lock();
	//for (u32 i = 0; i < (u32)TerrainManager.VisibleChunkIDs.size(); i++)
	//{
	//	terrain_chunk& Chunk = TerrainManager.ChunkArray[TerrainManager.VisibleChunkIDs[i]];

	//	if (Chunk.CurrentLOD <= 4)
	//	{
	//		Renderer.DrawIndexedTerrainChunk(Chunk.Vertices.data(), Chunk.Indices.data(), (u32)Chunk.Vertices.size(), (u32)Chunk.Indices.size());
	//		DebugData.ChunkDrawCalls++;
	//	}

	//	if (Chunk.CurrentLOD == 0 || Chunk.CurrentLOD == 36)
	//		Chunk.RenderSubChunk = false;
	//	else
	//		Chunk.RenderSubChunk = true;
	//}
	//Renderer.DrawIndexedTerrainChunk(TerrainManager.LowLODVertices.data(), TerrainManager.LowLODIndices.data(), (u32)TerrainManager.LowLODVertices.size(), (u32)TerrainManager.LowLODIndices.size());
	//TerrainManager.VisibleChunkSwapMutex.unlock();
	//TerrainManager.ChunkDataSwapMutex.unlock();
}

inline bool CompareRenderComponents(rendering_component* Comp1, rendering_component* Comp2)
{
	//return ((Comp1.RenderResources.MaterialID == Comp2.RenderResources.MaterialID) && (Comp1.RenderResources.MeshAssetID < Comp2.RenderResources.MeshAssetID)) ||
	//	(Comp1.RenderResources.MaterialID < Comp2.RenderResources.MaterialID);

	return ((Comp1->RenderResources.PipelineStateID == Comp2->RenderResources.PipelineStateID && Comp1->RenderResources.MaterialID < Comp2->RenderResources.MaterialID) ||
			(Comp1->RenderResources.PipelineStateID == Comp2->RenderResources.PipelineStateID && Comp1->RenderResources.MaterialID == Comp2->RenderResources.MaterialID && Comp1->RenderResources.MeshAssetID < Comp2->RenderResources.MeshAssetID) ||
			Comp1->RenderResources.PipelineStateID < Comp2->RenderResources.PipelineStateID);
}

// todo: multithreading
void engine::RenderScene()
{
	// Prep frame for render
	transform CamTransform = transform(MainCamera.CameraInfo.Transform.Location);
	FrameConstants.CameraViewProjectionMatrix = MainCamera.ProjectionMatrix * MainCamera.ViewMatrix;
	FrameConstants.CameraWorldMatrix = renderer::GenerateWorldMatrix(CamTransform); // cache camera matrix?
	FrameConstants.CameraWorldViewMatrix = MainCamera.ViewMatrix * FrameConstants.CameraWorldMatrix;
	FrameConstants.CameraPosition = MainCamera.CameraInfo.Transform.Location;
	Renderer.MapConstants(map_operation::Frame);

	std::vector<rendering_component>& RCRegistry = RenderingComponents.GetRegistry();
	std::vector<rendering_component*> SortedRegistry;
	u32 NumRenderComponents = (u32)RCRegistry.size();
	for (u32 i = 0; i < NumRenderComponents; i++)
		SortedRegistry.push_back(&RCRegistry[i]);
	
	if (NumRenderComponents > 0) // first component is skybox
	{
		// Sort by material ID then by meshid (optimize?)
		std::sort(SortedRegistry.begin(), SortedRegistry.end(), CompareRenderComponents); //todo: dont sort every time
		s32 MaterialID = SortedRegistry[0]->RenderResources.MaterialID;
		s32 AssetID = SortedRegistry[0]->RenderResources.MeshAssetID;
		s32 PipelineStateID = SortedRegistry[0]->RenderResources.PipelineStateID;
		cMeshAsset* Asset = (cMeshAsset*)AssetRegistry[AssetID];

		Renderer.SetPipelineState(PipelineStates.GetComponent(PipelineStateID));
		Renderer.BindMaterial(MaterialRegistry.GetComponent(MaterialID));

		// Render loop
		u32 InstanceCount = 0;
		bool Draw = false;
		for (u32 i = 0; i < NumRenderComponents; i++)
		{
			if (i < SortedRegistry.size() - 1)
			{
				if (SortedRegistry[i + 1]->RenderResources.PipelineStateID != PipelineStateID) // pipeline state will change
				{
					Draw = true;
					PipelineStateID = SortedRegistry[i + 1]->RenderResources.PipelineStateID;
				}
				if (SortedRegistry[i + 1]->RenderResources.MaterialID != MaterialID) // material will change
				{
					Draw = true;
					MaterialID = SortedRegistry[i + 1]->RenderResources.MaterialID;
				}
				if (SortedRegistry[i + 1]->RenderResources.MeshAssetID != AssetID) // mesh will change
				{
					Draw = true;
					AssetID = SortedRegistry[i + 1]->RenderResources.MeshAssetID;
				}
			}
			else
				Draw = true;

			if (SortedRegistry[i]->IsActive() && SortedRegistry[i]->RenderResources.MaterialID != -1 && SortedRegistry[i]->ActorComponentID != -1 && SortedRegistry[i]->RenderResources.MeshAssetID != -1 && SortedRegistry[i]->RenderResources.PipelineStateID != -1)
			{
				actor_component& ActorComp = ActorComponents.GetComponent(SortedRegistry[i]->ActorComponentID);
				if (ActorComp.Active)
				{
					if (ActorComp.Flag == actor_flag::PositionUpdated)
					{
						transform FinalRenderTransform = ActorComp.GetTransform() + SortedRegistry[i]->GetTransform();
						FinalRenderTransform.Scale = ActorComp.GetScale() * SortedRegistry[i]->GetScale();
						ActorConstants.Instances[InstanceCount].WorldMatrix = renderer::GenerateWorldMatrix(FinalRenderTransform);
						ActorConstants.Instances[InstanceCount].InverseTransposeWorldMatrix = renderer::InverseMatrix(ActorConstants.Instances[InstanceCount].WorldMatrix, false); // ?
						SortedRegistry[i]->SetWorldMatrix(ActorConstants.Instances[InstanceCount].WorldMatrix);
						SortedRegistry[i]->SetITPWorldMatrix(ActorConstants.Instances[InstanceCount].InverseTransposeWorldMatrix);
					}
					else
					{
						ActorConstants.Instances[InstanceCount].WorldMatrix = SortedRegistry[i]->GetWorldMatrix();
						ActorConstants.Instances[InstanceCount].InverseTransposeWorldMatrix = SortedRegistry[i]->GetInverseWorldMatrix();
					}
					InstanceCount++;
				}

				if (Draw || InstanceCount >= MAX_INSTANCES) // draw all previous instances and update state for new batch
				{
					if (InstanceCount > 0)
					{
						Renderer.MapConstants(map_operation::Actor);
						//Renderer.DrawInstanced((vertex*)Asset->Data, Asset->MeshData.NumVertices, InstanceCount, draw_topology_type::TriangleList);
						Renderer.DrawIndexedInstanced((vertex*)Asset->Data, (u32*)((vertex*)Asset->Data + Asset->MeshData.NumVertices), Asset->MeshData.NumVertices, Asset->MeshData.NumIndices, 0, InstanceCount, draw_topology_type::TriangleList);
						InstanceCount = 0;
					}
				}
			}
			if (Draw) // no need to update state if drawing bc of batching
			{
				Renderer.SetPipelineState(PipelineStates.GetComponent(PipelineStateID));
				Renderer.BindMaterial(MaterialRegistry.GetComponent(MaterialID));
				Asset = (cMeshAsset*)AssetRegistry[AssetID];
			}
			Draw = false;
		}
	}

	RenderPlanet();
}

void engine::UpdateComponents()
{
	std::vector<actor_component>& ARegistry = ActorComponents.GetRegistry();

	// Actor update ?
	u32 NumActorComponents = (u32)ARegistry.size();
	for (u32 i = 0; i < NumActorComponents; i++)
	{
		ARegistry[i].Flag = actor_flag::Idle;
		//ARegistry[i].SetRotation(ARegistry[i].GetRotation() + rotator{ 0.f, 0.01f, 0.f });
	}
}

void engine::RenderDebugWidgets()
{
	//ImGui::ShowDemoWindow();
	if (ImGui::Begin("Debug Screen"))
	{
		ImGui::Text("FPS: %f", 1 / (UserInputs.DeltaTime / 1000));
		ImGui::Text("Delta: %f", UserInputs.DeltaTime);
		//ImGui::Text("Ctrl: %d", UserInputs.KeysDown[VK_CONTROL].Pressed);
	}
	ImGui::End();

	if (ImGui::Begin("Engine State"))
	{
		ImGui::Text("Player Position: (%f, %f, %f)", MainCamera.CameraInfo.Transform.Location.x, MainCamera.CameraInfo.Transform.Location.y, MainCamera.CameraInfo.Transform.Location.z);

		ImGui::Text("MousePos: (%f, %f)", UserInputs.MousePosX, UserInputs.MousePosY);

		if (ImGui::TreeNode("Terrain Info"))
		{
			ImGui::Indent();

			ImGui::AlignTextToFramePadding();
			ImGui::Text("Picking Ray: (%f, %f, %f)", UserInputs.MousePosWorldSpace.x, UserInputs.MousePosWorldSpace.y, UserInputs.MousePosWorldSpace.z);

			ImGui::AlignTextToFramePadding();
			ImGui::Text("IntersectingIndex: %d", DebugData.IntersectingIndex);

			if (DebugData.IntersectingIndex != -1 && DebugData.IntersectingTree != -1)
			{
				ImGui::Indent();

				binary_node& Node = TerrainManager.Trees[DebugData.IntersectingTree].Nodes[DebugData.IntersectingIndex];

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Tree: %d", DebugData.IntersectingTree);

				//ImGui::AlignTextToFramePadding();
				//ImGui::Text("CurrentTreeDepth: %d", TerrainManager.TreesCurrentDepth);

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Depth: %d", Node.Depth);

				ImGui::AlignTextToFramePadding();
				ImGui::Text("FirstChildIndex: %d", Node.FirstChildIndex);

				ImGui::AlignTextToFramePadding();
				ImGui::Text("IsLeaf: %s", (Node.IsLeaf ? "true" : "false"));

				ImGui::AlignTextToFramePadding();
				ImGui::Text("ForceSplitBy: %d", Node.ForceSplitBy);

				ImGui::AlignTextToFramePadding();
				ImGui::Text("ForceSplitByTree: %d", Node.ForceSplitByTree);

				ImGui::AlignTextToFramePadding();
				ImGui::Text("LeftNeighbor: %d", Node.LeftNeighbor);

				ImGui::AlignTextToFramePadding();
				ImGui::Text("RightNeighbor: %d", Node.RightNeighbor);

				ImGui::AlignTextToFramePadding();
				ImGui::Text("BottomNeighbor: %d", Node.BottomNeighbor);

				ImGui::AlignTextToFramePadding();
				ImGui::Text("LeftNeighborTree: %d", Node.LeftNeighborTree);

				ImGui::AlignTextToFramePadding();
				ImGui::Text("RightNeighborTree: %d", Node.RightNeighborTree);

				ImGui::AlignTextToFramePadding();
				ImGui::Text("BottomNeighborTree: %d", Node.BottomNeighborTree);

				ImGui::Unindent();
			}

			ImGui::Unindent();
			ImGui::TreePop();
		}

		ImGui::Text("Num Terrain Verts: %d", DebugData.NumTerrainVertices);

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Sun Angle:");
		ImGui::SameLine();
		f32 Min = 0.f;
		f32 Max = 360.f;
		ImGui::DragScalar("##SunAngle", ImGuiDataType_Float, &DebugData.SunAngle, 0.5f, &Min, &Max, "%f", 1.0f);

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Camera Speed:");
		ImGui::SameLine();
		Min = 0.f;
		Max = 1.f;
		ImGui::DragScalar("##CameraSpeed", ImGuiDataType_Float, &DebugData.CameraSpeed, 0.001f, &Min, &Max, "%f", 1.0f);
		
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Enable Wireframe:");
		ImGui::SameLine();
		if (ImGui::Button((DebugData.EnableWireframe ? "true##0" : "false##0")))
			DebugData.EnableWireframe = !DebugData.EnableWireframe;

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Visible Chunk Updates:");
		ImGui::SameLine();
		if (ImGui::Button((DebugData.VisibleChunkUpdates ? "true##1" : "false##1")))
			DebugData.VisibleChunkUpdates = !DebugData.VisibleChunkUpdates;

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Enable Slow Mode:");
		ImGui::SameLine();
		if (ImGui::Button((DebugData.SlowMode ? "true##2" : "false##2")))
			DebugData.SlowMode = !DebugData.SlowMode;

		ImGui::AlignTextToFramePadding();
		ImGui::Text("Enable Free Cam:");
		ImGui::SameLine();
		if (ImGui::Button((DebugData.FreeCam ? "true##3" : "false##3")))
			DebugData.FreeCam = !DebugData.FreeCam;

		if (ImGui::CollapsingHeader("Rendering Components"))
		{
			std::vector<rendering_component>& RCRegistry = RenderingComponents.GetRegistry();
			for (u32 i = 0; i < (u32)RCRegistry.size(); i++)
			{
				ImGui::PushID(i);

				if (i < 1000)
				{
					if (RCRegistry[i].IsValid())
					{
						u32 NumZeros = (i > 0 ? (i > 9 ? (i > 99 ? 0 : 1) : 2) : 3);

						char Buffer[20];
						_snprintf_s(Buffer, sizeof(Buffer), (NumZeros == 0 ? "%d" : (NumZeros == 1 ? "0%d" : (NumZeros == 2 ? "00%d" : "000"))), i);

						if (ImGui::TreeNode(Buffer))
						{
							ImGui::Indent();

							ImGui::AlignTextToFramePadding();
							ImGui::Text("IsActive:");
							ImGui::SameLine();
							if (ImGui::Button((RCRegistry[i].Active ? "true" : "false")))
								RCRegistry[i].Active = !RCRegistry[i].Active;

							ImGui::AlignTextToFramePadding();
							ImGui::Text("ActorComponentID:");
							ImGui::SameLine();
							_snprintf_s(Buffer, sizeof(Buffer), "%d", RCRegistry[i].ActorComponentID);
							if (ImGui::InputText("##ActorComponentID", Buffer, 10, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_EnterReturnsTrue))
								RCRegistry[i].ActorComponentID = atoi(Buffer);

							ImGui::AlignTextToFramePadding();
							ImGui::Text("MeshAssetID:");
							ImGui::SameLine();
							_snprintf_s(Buffer, sizeof(Buffer), "%d", RCRegistry[i].RenderResources.MeshAssetID);
							if (ImGui::InputText("##MeshAssetID", Buffer, 10, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_EnterReturnsTrue))
								RCRegistry[i].RenderResources.MeshAssetID = atoi(Buffer);

							ImGui::AlignTextToFramePadding();
							ImGui::Text("MaterialID:");
							ImGui::SameLine();
							_snprintf_s(Buffer, sizeof(Buffer), "%d", RCRegistry[i].RenderResources.MaterialID);
							if (ImGui::InputText("##MaterialID", Buffer, 10, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_EnterReturnsTrue))
								RCRegistry[i].RenderResources.MaterialID = atoi(Buffer);

							ImGui::AlignTextToFramePadding();
							ImGui::Text("PipelineStateID:");
							ImGui::SameLine();
							_snprintf_s(Buffer, sizeof(Buffer), "%d", RCRegistry[i].RenderResources.PipelineStateID);
							if (ImGui::InputText("##PipelineStateID", Buffer, 10, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_EnterReturnsTrue))
								RCRegistry[i].RenderResources.PipelineStateID = atoi(Buffer);

							ImGui::AlignTextToFramePadding();
							switch (RCRegistry[i].RenderResources.TopologyType)
							{
							case draw_topology_type::TriangleList:
								ImGui::Text("DrawTopologyType: %s", "TriangleList"); break;
							case draw_topology_type::LineList:
								ImGui::Text("DrawTopologyType: %s", "LineList"); break;
							case draw_topology_type::PointList:
								ImGui::Text("DrawTopologyType: %s", "PointList"); break;
							}

							ImGui::Unindent();
							ImGui::TreePop();
						}
					}
				}
				ImGui::PopID();
			}
		}
	}
	ImGui::End();

	if (ImGui::Begin("Content Browser", NULL, ImGuiWindowFlags_MenuBar))//, &DebugData.Editor_ContentBrowser_Open))
	{
		//if (DebugData.Editor_ContentBrowser_DraggingFiles)
		//{
		//	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceExtern))	// we use an external source (i.e. not ImGui-created)
		//	{
		//		// replace "FILES" with whatever identifier you want - possibly dependant upon what type of files are being dragged
		//		// you can specify a payload here with parameter 2 and the sizeof(parameter) for parameter 3.
		//		// I store the payload within a vector of strings within the application itself so don't need it.
		//		ImGui::SetDragDropPayload("FILES", nullptr, 0);
		//		ImGui::BeginTooltip();
		//		ImGui::Text("Files");
		//		ImGui::EndTooltip();
		//		ImGui::EndDragDropSource();
		//	}
		//}
#if SPADE_DEBUG
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Create Asset"))
			{
				asset_type* AssetTypes = assetLoader::GetAssetTypeArray();
				for (u32 i = 0; i < assetLoader::AssetTypeArraySize(); i++)
				{
					ImGui::PushID(i);
					switch (AssetTypes[i].TypeID)
					{
						default:
						{} break;

						case MATERIAL_ASSET_ID:
						{
							if (ImGui::MenuItem(AssetTypes[i].TypeName))
							{
								material Mat = material();
								char Path[MAX_PATH];
								srand((u32)time(NULL));
								_snprintf_s(Path, sizeof(Path), "assets\\materials\\m_%d.emf", rand()); // temporary
								FILE* file = fopen(Path, "wb"); // kind of hardcoded, but as long as we stick to putting materials in this folder it shouldn't be an issue
								if (file)
								{
									assetTypes::material_data data = Mat.Serialize();
									fwrite((char*)&data, sizeof(assetTypes::material_data), 1, file);
									fclose(file);
								}

								const char* NewPath = assetLoader::PackAsset(Path);
								cAsset* LoadedAsset = assetLoader::InitializeAsset(NewPath);
								if (LoadedAsset != nullptr)
									LoadedAsset->LoadAssetData();

								delete[] NewPath;
							}
						} break;
					}
					ImGui::PopID();
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
#endif

		if (ImGui::BeginChild("cbchild"))
		{
			ImGuiIO& io = ImGui::GetIO();
			ImVec2 ButtonSize(100, 100);
			ImGuiStyle& style = ImGui::GetStyle();
			float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
			for (u32 i = 0; i < (u32)AssetRegistry.size(); i++)
			{
				cAsset* Asset = AssetRegistry[i];

				if (Asset->Active)
				{
					float last_button_x2 = ImGui::GetItemRectMax().x;
					float next_button_x2 = last_button_x2 + style.ItemSpacing.x + ButtonSize.x; // Expected position if next button was on same line
					if (i < AssetRegistry.size() && next_button_x2 < window_visible_x2)
						ImGui::SameLine();
					ImGui::BeginGroup();
					ImGui::PushID(i);
					if (Asset->Type == TEXTURE_ASSET_ID)
						ImGui::ImageButton(((cTextureAsset*)Asset)->ShaderHandle, ImVec2(ButtonSize.x - 6, ButtonSize.y - 6)); // TODO: PLATFORM
					else
						ImGui::Button(Asset->Filename, ButtonSize);

					ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ButtonSize.x);
					ImGui::Text(Asset->Filename);
					ImGui::PopTextWrapPos();
					ImGui::EndGroup();

					if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
						ImGui::OpenPopup("Details");

					if (ImGui::BeginPopupModal("Details", NULL, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_AlwaysAutoResize))
					{
						if (ImGui::BeginMenuBar())
						{
							if (ImGui::MenuItem("Close"))
								ImGui::CloseCurrentPopup();

#if SPADE_DEBUG
							switch (Asset->Type)
							{
								default:
								{} break;

								case MATERIAL_ASSET_ID:
								{
									if (ImGui::MenuItem("Save & Close"))
									{
										//todo: delete .eaf file?
										std::string Filename = Asset->Filename; // kind of hardcoded, but as long as we stick to putting materials in this folder it shouldn't be an issue
										FILE* file = fopen(("assets\\materials\\" + Filename).c_str(), "wb");
										if (file)
										{
											assetTypes::material_data data = MaterialRegistry.GetComponent(((assetTypes::material_data*)Asset->Data)->ComponentID).Serialize();
											fwrite((char*)&data, sizeof(assetTypes::material_data), 1, file);
											fclose(file);
										}
										ImGui::CloseCurrentPopup();
									}
								} break;
							}
#endif

							ImGui::EndMenuBar();
						}

						ImGui::Text("Filename: %s", Asset->Filename);
						ImGui::Text("Path: %s", Asset->Path);
						ImGui::Text("Type: %s", assetLoader::GetAssetTypeFromID(Asset->Type).TypeName);
						ImGui::Text("AssetID: %d", i); // NOT Asset->AssetID

						f32 PreviewScale = 1.f;
						//DebugData.Editor_ContentBrowser_PreviewScale += io.MouseWheel * 0.1f;
						//if (DebugData.Editor_ContentBrowser_PreviewScale < 0)
						//	DebugData.Editor_ContentBrowser_PreviewScale = 0;

						switch (Asset->Type)
						{
							default:
							{} break;

							case TEXTURE_ASSET_ID:
							{
								cTextureAsset* Tex = (cTextureAsset*)Asset;
								ImGui::Text("TextureID: %d", GetTextureIDFromName(Asset->Filename));
								ImGui::Text("Size: %d x %d", Tex->ImageData.Width, Tex->ImageData.Height);
								ImGui::Text("Channels: %d", Tex->ImageData.Channels);
								ImGui::Text("Preview:");
								ImGui::Image(Tex->ShaderHandle, ImVec2(512.f, 512.f)/*ImVec2(Tex->ImageData.Width * PreviewScale, Tex->ImageData.Height * PreviewScale)*/, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
							} break;

							//case FONT_ASSET_ID:
							//{
							//	cFontAsset* Font = (cFontAsset*)Asset;
							//	ImGui::Text("Preview:");
							//	ImGui::Image(Font->AtlasShaderHandle, ImVec2(Font->FontData.AtlasDim * PreviewScale, Font->FontData.AtlasDim * PreviewScale), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
							//} break;

							case MESH_ASSET_ID:
							{
								cMeshAsset* Mesh = (cMeshAsset*)Asset;
								ImGui::Text("Vertex Count: %d", Mesh->MeshData.NumVertices);
								ImGui::Text("Index Count: %d", Mesh->MeshData.NumIndices);
								//ImGui::Text("Preview:");
							} break;

							case MATERIAL_ASSET_ID:
							{
								assetTypes::cMaterialAsset* MatAsset = (assetTypes::cMaterialAsset*)Asset;
								if (MatAsset->Loaded)
								{
									u32 CompID = ((assetTypes::material_data*)MatAsset->Data)->ComponentID;
									material& Mat = Engine->MaterialRegistry.GetComponent(CompID);
									ImGui::Text("MaterialID: %d", CompID);

									ImGui::AlignTextToFramePadding();
									ImGui::Text("Diffuse Color:");
									ImGui::SameLine();
									ImGui::ColorEdit4("##diffuse", Mat.DiffuseColor.E);

									char Buffer[10];
									ImGui::AlignTextToFramePadding();
									ImGui::Text("Reflectivity:");
									ImGui::SameLine();
									_snprintf_s(Buffer, sizeof(Buffer), "%f", Mat.Reflectivity);
									if (ImGui::InputText("##Reflectivity", Buffer, 10, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_EnterReturnsTrue))
										Mat.Reflectivity = (f32)atof(Buffer);

									ImGui::AlignTextToFramePadding();
									ImGui::Text("DiffuseTextureID:");
									ImGui::SameLine();
									_snprintf_s(Buffer, sizeof(Buffer), "%d", Mat.DiffuseTextureID);
									if (ImGui::InputText("##DiffuseTextureID", Buffer, 10, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_EnterReturnsTrue))
										Mat.DiffuseTextureID = atoi(Buffer);

									ImGui::AlignTextToFramePadding();
									ImGui::Text("NormalTextureID:");
									ImGui::SameLine();
									_snprintf_s(Buffer, sizeof(Buffer), "%d", Mat.NormalTextureID);
									if (ImGui::InputText("##NormalTextureID", Buffer, 10, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_EnterReturnsTrue))
										Mat.NormalTextureID = atoi(Buffer);

									ImGui::AlignTextToFramePadding();
									ImGui::Text("ReflectiveTextureID:");
									ImGui::SameLine();
									_snprintf_s(Buffer, sizeof(Buffer), "%d", Mat.ReflectiveTextureID);
									if (ImGui::InputText("##ReflectiveTextureID", Buffer, 10, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_EnterReturnsTrue))
										Mat.ReflectiveTextureID = atoi(Buffer);

									//ImGui::Text("Preview:");
								}
							} break;
						}
						ImGui::EndPopup();
					}

					if (ImGui::BeginPopupContextItem("AssetContext"))
					{
						//ImGui::Text("Select Tile");

						if (ImGui::Button("Delete"))
						{
							remove(AssetRegistry[i]->Path);
							AssetRegistry[i]->UnloadAsset();
							AssetRegistry[i]->Active = false; // dont remove from array as not to disturb index order
							//AssetRegistry.erase(AssetRegistry.begin() + i);
							ImGui::CloseCurrentPopup();
						}

						if (ImGui::Button("Export"))
						{
							//assetLoader::ExportAsset(Asset);
							ImGui::CloseCurrentPopup();
						}

						if (ImGui::Button("Rename"))
						{
							ImGui::OpenPopup("Rename (enter to confirm)");
							//ImGui::CloseCurrentPopup();
						}

						if (ImGui::Button("Close"))
							ImGui::CloseCurrentPopup();

						if (ImGui::BeginPopupModal("Rename (enter to confirm)", NULL, ImGuiWindowFlags_AlwaysAutoResize))
						{
							char Buffer[MAX_PATH];
							_snprintf_s(Buffer, sizeof(Buffer), "%s", Asset->Filename);
							Buffer[strlen(Asset->Filename) - 4] = '\0';
							if (ImGui::InputText("##RenameName", Buffer, 30, ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_EnterReturnsTrue))
							{
								assetLoader::RenameAsset(Asset, Buffer); // todo: check for duplicate filenames
								ImGui::CloseCurrentPopup();
							}

							if (ImGui::Button("Close##2"))
								ImGui::CloseCurrentPopup();

							ImGui::EndPopup();
						}

						ImGui::EndPopup();
					}
					ImGui::PopID();
				}
			}
		}
		ImGui::EndChild();

#if SPADE_DEBUG
		//if (ImGui::BeginDragDropTarget())
		//{
		//	if (ImGui::AcceptDragDropPayload("FILES"))  // or: const ImGuiPayload* payload = ... if you sent a payload in the block above
		//	{
		//		for (std::string szFile : DebugData.Editor_ContentBrowser_DraggedFileNames)
		//		{
		//			s32 NewID = GlobalAssetRegistry.Num();
		//			WIN32_FIND_DATA data;
		//			HANDLE hFind = FindFirstFile(szFile.c_str(), &data);
		//			std::string Filename = data.cFileName;
		//			std::string FullPath = "assets\\" + Filename;

		//			std::ifstream  src(szFile, std::ios::binary);
		//			std::ofstream  dst(FullPath.c_str(), std::ios::binary);

		//			dst << src.rdbuf();
		//			src.close();
		//			dst.close();

		//			switch (assetLoader::GetFileType(data.cFileName))
		//			{
		//			default:
		//			{} break;

		//			case Texture:
		//			{
		//				const char* NewPath = assetLoader::PackImage(FullPath.c_str(), NewID);
		//				assetLoader::LoadImage(NewPath, AssetLoadCallbacks.ImageCallback);
		//				delete[] NewPath;
		//			} break;

		//			case Font:
		//			{
		//				const char* NewPath = assetLoader::PackFont(FullPath.c_str(), NewID);
		//				assetLoader::LoadFont(NewPath, AssetLoadCallbacks.FontCallback);
		//				delete[] NewPath;
		//			} break;
		//			}
		//		}
		//		DebugData.Editor_ContentBrowser_DraggedFileNames.clear();
		//	}
		//	ImGui::EndDragDropTarget();
		//}
#endif
	}
	ImGui::End();
}
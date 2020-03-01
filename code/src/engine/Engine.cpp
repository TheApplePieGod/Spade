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

	MainCamera.UpdateFromInput();

	// Update lighting constants
	f32 Angle = DegreesToRadians(DebugData.SunAngle);
	LightingConstants.SunDirection = v3{ 0.f, cos(Angle), sin(Angle)};
	DebugData.SunAngle += 0.05f * UserInputs.DeltaTime;
	Renderer.MapConstants(map_operation::Lighting);

	MainCamera.UpdateProjectionType(projection_type::Perspective);
	MainCamera.ViewMatrix = renderer::GenerateViewMatrix(true, MainCamera.CameraInfo, MainCamera.LookAtVector, MainCamera.UpVector);
	RenderScene();

	UpdateComponents(); // before scene render?

	RenderDebugWidgets();

	Renderer.FinishFrame();

}

void engine::Initialize(void* Window, int WindowWidth, int WindowHeight)
{
	ScreenSize = { (f32)WindowWidth, (f32)WindowHeight };
	MainLevel = level();
	MainCamera = camera((f32)WindowWidth, (f32)WindowHeight);
	MemoryManager = memory_manager();

	//debug
	vertex* verts = new vertex[36];

	// Front
	verts[0] = { -0.5f, -0.5f, -0.5f, 0.f, 1.f, 0.f, 0.f, -1.f };
	verts[1] = { -0.5f, 0.5f, -0.5f, 0.f, 0.f, 0.f, 0.f, -1.f };
	verts[2] = { 0.5f, 0.5f, -0.5f, 1.f, 0.f, 0.f, 0.f, -1.f };
	verts[3] = verts[0];
	verts[4] = verts[2];
	verts[5] = { 0.5f, -0.5f, -0.5f, 1.f, 1.f, 0.f, 0.f, -1.f };

	// Back
	verts[6] = { 0.5f, -0.5f, 0.5f, 0.f, 1.f, 0.f, 0.f, 1.f };
	verts[7] = { 0.5f, 0.5f, 0.5f, 0.f, 0.f, 0.f, 0.f, 1.f };
	verts[8] = { -0.5f, 0.5f, 0.5f, 1.f, 0.f, 0.f, 0.f, 1.f };
	verts[9] = verts[8];
	verts[10] = { -0.5f, -0.5f, 0.5f, 1.f, 1.f, 0.f, 0.f, 1.f };
	verts[11] = verts[6];

	// Top
	verts[12] = verts[2]; verts[12].v = 1.f; verts[12].ny = 1; verts[12].nz = 0;
	verts[13] = verts[1]; verts[13].v = 1.f; verts[13].ny = 1; verts[13].nz = 0;
	verts[14] = verts[8]; verts[14].u = 0.f; verts[14].ny = 1; verts[14].nz = 0;
	verts[15] = verts[12]; 
	verts[16] = verts[14];
	verts[17] = verts[7]; verts[17].u = 1.f; verts[17].ny = 1; verts[17].nz = 0;

	// Bottom
	verts[18] = verts[10]; verts[18].v = 0.f; verts[18].ny = -1; verts[18].nz = 0;
	verts[19] = verts[0]; verts[19].u = 1.f; verts[19].ny = -1; verts[19].nz = 0;
	verts[20] = verts[5]; verts[20].u = 0.f; verts[20].ny = -1; verts[20].nz = 0;
	verts[21] = verts[6]; verts[21].v = 0.f; verts[21].ny = -1; verts[21].nz = 0;
	verts[22] = verts[18];
	verts[23] = verts[20];

	// Left Side
	verts[24] = verts[8]; verts[24].u = 0.f; verts[24].nx = -1; verts[24].nz = 0;
	verts[25] = verts[1]; verts[25].u = 1.f; verts[25].nx = -1; verts[25].nz = 0;
	verts[26] = verts[0]; verts[26].u = 1.f; verts[26].nx = -1; verts[26].nz = 0;
	verts[27] = verts[10]; verts[27].u = 0.f; verts[27].nx = -1; verts[27].nz = 0;
	verts[28] = verts[24];
	verts[29] = verts[26];

	// Right Side
	verts[30] = verts[5]; verts[30].u = 0.f; verts[30].nx = 1; verts[30].nz = 0;
	verts[31] = verts[2]; verts[31].u = 0.f; verts[31].nx = 1; verts[31].nz = 0;
	verts[32] = verts[7]; verts[32].u = 1.f; verts[32].nx = 1; verts[32].nz = 0;
	verts[33] = verts[30];
	verts[34] = verts[32];
	verts[35] = verts[6]; verts[35].u = 1.f; verts[35].nx = 1; verts[35].nz = 0;

	cMeshAsset* mesh = new cMeshAsset();
	mesh->MeshData.NumVertices = 36;
	mesh->Data = verts;
	mesh->Loaded = true;
	//AssetRegistry.push_back(mesh);

	MemoryManager.Initialize();
	Renderer.Initialize(Window, WindowWidth, WindowHeight);
	MainLevel.Initialize();

	defaultAssetTypes::ImageType.LoadCallback = assetCallbacks::ImageLoadCallback;
	defaultAssetTypes::MeshType.LoadCallback = assetCallbacks::MeshLoadCallback;
	assetLoader::AddAssetType(defaultAssetTypes::ImageType);
	assetLoader::AddAssetType(defaultAssetTypes::MeshType);

#if SPADE_DEBUG
	assetLoader::ScanAssets("assets\\", false, true);
	assetLoader::InitializeAssetsInDirectory("assets\\", true);
#else
	assetLoader::InitializeAssetsFromPack();
#endif

	s32 ids[6];
	switch (4)
	{
		default: // mountains
		{
			ids[0] = GetTextureIDFromName("emap-left.tga");
			ids[1] = GetTextureIDFromName("emap-right.tga");
			ids[2] = GetTextureIDFromName("emap-up.png");
			ids[3] = GetTextureIDFromName("emap-down.png");
			ids[4] = GetTextureIDFromName("emap-front.tga");
			ids[5] = GetTextureIDFromName("emap-back.tga");
		} break;
		case 1: // underwater
		{
			ids[0] = GetTextureIDFromName("underwater-left.jpg");
			ids[1] = GetTextureIDFromName("underwater-right.jpg");
			ids[2] = GetTextureIDFromName("underwater-up.jpg");
			ids[3] = GetTextureIDFromName("underwater-down.jpg");
			ids[4] = GetTextureIDFromName("underwater-front.jpg");
			ids[5] = GetTextureIDFromName("underwater-back.jpg");
		} break;
		case 2: // bloodbeach
		{
			ids[0] = GetTextureIDFromName("bloodbeach-left.tga");
			ids[1] = GetTextureIDFromName("bloodbeach-right.tga");
			ids[2] = GetTextureIDFromName("bloodbeach-up.png");
			ids[3] = GetTextureIDFromName("bloodbeach-down.png");
			ids[4] = GetTextureIDFromName("bloodbeach-front.tga");
			ids[5] = GetTextureIDFromName("bloodbeach-back.tga");
		} break;
		case 3: // city1
		{
			ids[0] = GetTextureIDFromName("city-left.jpg");
			ids[1] = GetTextureIDFromName("city-right.jpg");
			ids[2] = GetTextureIDFromName("city-up.jpg");
			ids[3] = GetTextureIDFromName("city-down.jpg");
			ids[4] = GetTextureIDFromName("city-front.jpg");
			ids[5] = GetTextureIDFromName("city-back.jpg");
		} break;
		case 4: // city2
		{
			u32 id = GetTextureIDFromName("stars.png");
			ids[0] = id;
			ids[1] = id;
			ids[2] = id;
			ids[3] = id;
			ids[4] = id;
			ids[5] = id;
		} break;
	}

	Renderer.UpdateSkybox(ids);

	pipeline_state State = pipeline_state();
	State.VertexShaderID = GetShaderIDFromName("skyboxvs");
	State.PixelShaderID = GetShaderIDFromName("skyboxps");
	State.RasterizerState = rasterizer_state::DefaultCullFrontface;
	State.UniqueIdentifier = "DefaultPBR";
	PipelineStates.CreateComponent(State);

	State = pipeline_state();
	State.VertexShaderID = GetShaderIDFromName("mainvs");
	State.PixelShaderID = GetShaderIDFromName("SkyFromSpacePS");
	State.RasterizerState = rasterizer_state::DefaultCullFrontface;
	State.UniqueIdentifier = "DefaultPBR";
	PipelineStates.CreateComponent(State);

	State = pipeline_state();
	State.VertexShaderID = GetShaderIDFromName("mainvs");
	State.PixelShaderID = GetShaderIDFromName("GroundFromSpacePS");
	State.RasterizerState = rasterizer_state::DefaultCullBackface;
	State.UniqueIdentifier = "DefaultPBR";
	PipelineStates.CreateComponent(State);

	State = pipeline_state();
	State.VertexShaderID = GetShaderIDFromName("mainvs");
	State.PixelShaderID = GetShaderIDFromName("SkyFromAtmospherePS");
	State.RasterizerState = rasterizer_state::DefaultCullFrontface;
	State.UniqueIdentifier = "DefaultPBR";
	PipelineStates.CreateComponent(State);

	State = pipeline_state();
	State.VertexShaderID = GetShaderIDFromName("mainvs");
	State.PixelShaderID = GetShaderIDFromName("GroundFromAtmospherePS");
	State.RasterizerState = rasterizer_state::DefaultCullBackface;
	State.UniqueIdentifier = "DefaultPBR";
	PipelineStates.CreateComponent(State);

	for (u32 i = 0; i < 6; i++)
	{
		material Mat = material();
		switch (i)
		{
			default:
			{ Mat.DiffuseTextureID = GetTextureIDFromName("DefaultTexture.jpg");
			  Mat.Reflectivity = 1.f;
			  /*Mat.DiffuseColor = colors::Blue;*/ } break;

			//case 1:
			//{ Mat.DiffuseTextureID = GetTextureIDFromName("chev.jpg");
			//  /*Mat.DiffuseColor = colors::Red;*/ } break;

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
	for (u32 i = 0; i < 3; i++)
	{
		rendering_component rcomp = rendering_component(actorid);
		if (i == 0)
		{
			rcomp.SetScale(v3{ 1025.f, 1025.f, 1025.f });
			rcomp.RenderResources.MaterialID = 0;
			rcomp.RenderResources.PipelineStateID = 1;
			rcomp.RenderResources.MeshAssetID = GetAssetIDFromName("sphere2.fbx");
		}
		else if (i == 1)
		{
			rcomp.SetScale(v3{ 1000.f, 1000.f, 1000.f });
			rcomp.RenderResources.MaterialID = 0;
			rcomp.RenderResources.PipelineStateID = 2;
			rcomp.RenderResources.MeshAssetID = GetAssetIDFromName("sphere2.fbx");
		}
		else
		{
			rcomp.RenderResources.MaterialID = 0;
			rcomp.RenderResources.PipelineStateID = 0;
			rcomp.RenderResources.MeshAssetID = GetAssetIDFromName("cube_t.fbx");
		}

		rcomp.ActorComponentID = actorid;
		if (i == 0)
			AtmosphereRenderingID = RenderingComponents.CreateComponent(rcomp, true);
		else if (i == 1)
			PlanetRenderingID = RenderingComponents.CreateComponent(rcomp, true);
		else
			RenderingComponents.CreateComponent(rcomp, true);
	}

	LightingConstants.AmbientColor = v3{ 0.05f, 0.05f, 0.05f };
	LightingConstants.SunColor = colors::White;
	DebugData.SunAngle = 0.f;
}

void engine::Cleanup()
{
	Renderer.Cleanup();
}

void engine::ProcessUserInput()
{
	f32 speed = 0.08f * UserInputs.DeltaTime;
	v3 OldLocation = MainCamera.CameraInfo.Transform.Location;
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

	//if (Length(MainCamera.CameraInfo.Transform.Location) < 999.f)
	//	MainCamera.CameraInfo.Transform.Location = OldLocation;

	if (Length(MainCamera.CameraInfo.Transform.Location) > 1025.f)
	{
		RenderingComponents.GetComponent(AtmosphereRenderingID).RenderResources.PipelineStateID = 1;
		RenderingComponents.GetComponent(PlanetRenderingID).RenderResources.PipelineStateID = 2;
	}
	else
	{
		RenderingComponents.GetComponent(AtmosphereRenderingID).RenderResources.PipelineStateID = 3;
		RenderingComponents.GetComponent(PlanetRenderingID).RenderResources.PipelineStateID = 4;
	}

	// move to platform layer / optimize
	if (UserInputs.KeysDown[VK_CONTROL].Pressed)
		while (ShowCursor(true) <= 0);
	else
		while (ShowCursor(false) >= 0);
		
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

		// Prep frame for render
		transform CamTransform = transform(MainCamera.CameraInfo.Transform.Location);
		FrameConstants.CameraViewProjectionMatrix = MainCamera.ProjectionMatrix * MainCamera.ViewMatrix;
		FrameConstants.CameraWorldMatrix = renderer::GenerateWorldMatrix(CamTransform); // cache camera matrix?
		FrameConstants.CameraWorldViewMatrix = MainCamera.ViewMatrix * FrameConstants.CameraWorldMatrix;
		FrameConstants.CameraPosition = MainCamera.CameraInfo.Transform.Location;
		Renderer.MapConstants(map_operation::Frame);
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
						ActorConstants.Instances[InstanceCount].InverseTransposeWorldMatrix = renderer::InverseMatrix(ActorConstants.Instances[InstanceCount].WorldMatrix, false);
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
						Renderer.DrawInstanced((vertex*)Asset->Data, Asset->MeshData.NumVertices, InstanceCount, draw_topology_type::TriangleList);
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
	ImGui::ShowDemoWindow();
	if (ImGui::Begin("Debug Screen"))
	{
		ImGui::Text("FPS: %f", 1 / (UserInputs.DeltaTime / 1000));
		ImGui::Text("Delta: %f", UserInputs.DeltaTime);
		//ImGui::Text("Ctrl: %d", UserInputs.KeysDown[VK_CONTROL].Pressed);
	}
	ImGui::End();

	if (ImGui::Begin("Engine State"))
	{
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

						char Buffer[10];
						_snprintf_s(Buffer, sizeof(Buffer), (NumZeros == 0 ? "%d" : (NumZeros == 1 ? "0%d" : (NumZeros == 2 ? "00%d" : "000"))), i);

						if (ImGui::TreeNode(Buffer))
						{
							ImGui::Indent();

							ImGui::AlignTextToFramePadding();
							ImGui::Text("IsActive:");
							ImGui::SameLine();
							if (ImGui::Button((RCRegistry[i].Active ? "true" : "false")))
								RCRegistry[i].Active = !RCRegistry[i].Active;

							ImGui::Text("ActorComponentID:");
							ImGui::SameLine();
							_snprintf_s(Buffer, sizeof(Buffer), "%d", RCRegistry[i].ActorComponentID);
							if (ImGui::InputText("ActorComponentID", Buffer, 10, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_EnterReturnsTrue))
								RCRegistry[i].ActorComponentID = atoi(Buffer);

							ImGui::Text("MeshAssetID:");
							ImGui::SameLine();
							_snprintf_s(Buffer, sizeof(Buffer), "%d", RCRegistry[i].RenderResources.MeshAssetID);
							if (ImGui::InputText("MeshAssetID", Buffer, 10, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_EnterReturnsTrue))
								RCRegistry[i].RenderResources.MeshAssetID = atoi(Buffer);

							ImGui::Text("MaterialID:");
							ImGui::SameLine();
							_snprintf_s(Buffer, sizeof(Buffer), "%d", RCRegistry[i].RenderResources.MaterialID);
							if (ImGui::InputText("MaterialID", Buffer, 10, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_EnterReturnsTrue))
								RCRegistry[i].RenderResources.MaterialID = atoi(Buffer);

							ImGui::Text("PipelineStateID:");
							ImGui::SameLine();
							_snprintf_s(Buffer, sizeof(Buffer), "%d", RCRegistry[i].RenderResources.PipelineStateID);
							if (ImGui::InputText("PipelineStateID", Buffer, 10, ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_EnterReturnsTrue))
								RCRegistry[i].RenderResources.PipelineStateID = atoi(Buffer);

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
		
		//if (ImGui::CollapsingHeader("Assets"))
		//{
		//	for (u32 i = 0; i < (u32)AssetRegistry.size(); i++)
		//	{
		//		ImGui::PushID(i);



		//		ImGui::PopID;
		//	}
		//}
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
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Create Asset"))
			{
				asset_type* AssetTypes = assetLoader::GetAssetTypeArray();
				for (u32 i = 0; i < assetLoader::AssetTypeArraySize(); i++)
				{
					ImGui::PushID(i);
					if (ImGui::MenuItem(AssetTypes[i].TypeName))
					{

					}
					ImGui::PopID();
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
		if (ImGui::BeginChild("cbchild"))
		{
			ImGuiIO& io = ImGui::GetIO();
			ImVec2 ButtonSize(100, 100);
			ImGuiStyle& style = ImGui::GetStyle();
			float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
			for (u32 i = 0; i < (u32)AssetRegistry.size(); i++)
			{
				cAsset* Asset = AssetRegistry[i];
				ImGui::PushID(i);
				ImGui::Button(Asset->Filename, ButtonSize);
				float last_button_x2 = ImGui::GetItemRectMax().x;
				float next_button_x2 = last_button_x2 + style.ItemSpacing.x + ButtonSize.x; // Expected position if next button was on same line
				if (i + 1 < AssetRegistry.size() && next_button_x2 < window_visible_x2)
					ImGui::SameLine();

				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
					ImGui::OpenPopup("Details");

				if (ImGui::BeginPopupModal("Details", NULL, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_HorizontalScrollbar))
				{
					ImGui::Text("Filename: %s", Asset->Filename);
					ImGui::Text("Path: %s", Asset->Path);
					ImGui::Text("Type: %s", assetLoader::GetAssetTypeFromID(Asset->Type).TypeName);

					f32 PreviewScale = 1.f;
					//DebugData.Editor_ContentBrowser_PreviewScale += io.MouseWheel * 0.1f;
					//if (DebugData.Editor_ContentBrowser_PreviewScale < 0)
					//	DebugData.Editor_ContentBrowser_PreviewScale = 0;

					switch (Asset->Type)
					{
						default:
						{} break;

						case 1: // texture
						{
							cTextureAsset* Tex = (cTextureAsset*)Asset;
							ImGui::Text("Size: %d x %d", Tex->ImageData.Width, Tex->ImageData.Height);
							ImGui::Text("Channels: %d", Tex->ImageData.Channels);
							ImGui::Text("Preview:");
							ImGui::Image(Tex->ShaderHandle, ImVec2(Tex->ImageData.Width * PreviewScale, Tex->ImageData.Height * PreviewScale), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
						} break;

						//case 2: // font
						//{
						//	cFontAsset* Font = (cFontAsset*)Asset;
						//	ImGui::Text("Preview:");
						//	ImGui::Image(Font->AtlasShaderHandle, ImVec2(Font->FontData.AtlasDim * PreviewScale, Font->FontData.AtlasDim * PreviewScale), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
						//} break;

						case 3: // mesh
						{
							cMeshAsset* Mesh = (cMeshAsset*)Asset;
							ImGui::Text("Vertex Count: %d", Mesh->MeshData.NumVertices);
							ImGui::Text("Preview:");
						} break;
					}

					ImGui::NewLine();
					if (ImGui::Button("Close"))
						ImGui::CloseCurrentPopup();
					ImGui::EndPopup();
				}

				if (ImGui::BeginPopupContextItem("AssetContext"))
				{
					//ImGui::Text("Select Tile");

					if (ImGui::Button("Delete Asset"))
					{
						remove(AssetRegistry[i]->Path);
						AssetRegistry.erase(AssetRegistry.begin() + i);
						ImGui::CloseCurrentPopup();
					}

					if (ImGui::Button("Export Asset"))
					{
						//assetLoader::ExportAsset(Asset);
						ImGui::CloseCurrentPopup();
					}

					if (ImGui::Button("Close"))
						ImGui::CloseCurrentPopup();

					ImGui::EndPopup();
				}
				ImGui::PopID();
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
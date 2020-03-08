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
	LightingConstants.SunDirection = v3{ sin(Angle), 0.f, cos(Angle)};
	//DebugData.SunAngle += 0.05f * UserInputs.DeltaTime;
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

	MemoryManager.Initialize();
	Renderer.Initialize(Window, WindowWidth, WindowHeight);
	MainLevel.Initialize();

	InitializeAssetSystem();

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
	State.PixelShaderID = GetShaderIDFromName("SkyFromSpacePS");
	State.RasterizerState = rasterizer_state::DefaultCullFrontface;
	State.UniqueIdentifier = "DefaultPBR";
	PipelineStates.CreateComponent(State);

	State = pipeline_state();
	//State.VertexShaderID = GetShaderIDFromName("mainvs");
	State.VertexShaderID = GetShaderIDFromName("TerrainVS");
	State.PixelShaderID = GetShaderIDFromName("GroundFromSpacePS");
	State.HullShaderID = GetShaderIDFromName("TerrainHullShader");
	State.DomainShaderID = GetShaderIDFromName("TerrainDomainShader");
	State.EnableTesselation = true;
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
	State.VertexShaderID = GetShaderIDFromName("TerrainVS");
	State.PixelShaderID = GetShaderIDFromName("GroundFromAtmospherePS");
	State.HullShaderID = GetShaderIDFromName("TerrainHullShader");
	State.DomainShaderID = GetShaderIDFromName("TerrainDomainShader");
	State.EnableTesselation = true;
	State.RasterizerState = rasterizer_state::DefaultCullBackface;
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
	float PlanetRadius = 1160.f;
	v3 PlanetScale = v3{ PlanetRadius, PlanetRadius, PlanetRadius };
	for (u32 i = 0; i < 3; i++)
	{
		rendering_component rcomp = rendering_component(actorid);
		if (i == 0)
		{
			rcomp.SetScale(1.025f * PlanetScale);
			rcomp.RenderResources.MaterialID = 1;
			rcomp.RenderResources.PipelineStateID = 1;
			rcomp.RenderResources.MeshAssetID = GetAssetIDFromName("sphere3.fbx");
		}
		else if (i == 1)
		{
			rcomp.SetScale(PlanetScale);
			rcomp.RenderResources.MaterialID = 0;
			rcomp.RenderResources.PipelineStateID = 2;
			rcomp.RenderResources.MeshAssetID = GetAssetIDFromName("sphere3.fbx");
		}
		else if (i == 2)
		{
			rcomp.RenderResources.MaterialID = 1;
			rcomp.RenderResources.PipelineStateID = 0;
			rcomp.RenderResources.MeshAssetID = GetAssetIDFromName("cube_t.fbx");
		}
		else
		{
			rcomp.SetLocation(v3{ 0.f, 0.f, -1000.f });
			rcomp.RenderResources.MaterialID = 0;
			rcomp.RenderResources.PipelineStateID = 2;
			rcomp.RenderResources.MeshAssetID = GetAssetIDFromName("parasite_l_starkie.fbx");
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
	DebugData.SunAngle = 180.f;
}

void engine::Cleanup()
{
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

	f32 speed = 0.08f * UserInputs.DeltaTime;
	v3 OldLocation = MainCamera.CameraInfo.Transform.Location;

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
						//Renderer.DrawInstanced((vertex*)Asset->Data, Asset->MeshData.NumVertices, InstanceCount, draw_topology_type::TriangleList);
						Renderer.DrawIndexedInstanced((vertex*)Asset->Data, (u32*)((vertex*)Asset->Data + Asset->MeshData.NumVertices), Asset->MeshData.NumVertices, Asset->MeshData.NumIndices, InstanceCount, draw_topology_type::TriangleList);
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
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Sun Angle:");
		ImGui::SameLine();
		f32 Min = 0.f;
		f32 Max = 360.f;
		ImGui::DragScalar("##SunAngle", ImGuiDataType_Float, &DebugData.SunAngle, 0.5f, &Min, &Max, "%f", 1.0f);

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
#include "pch.h"
#include "Engine.h"
#include "AssetUtils.h"
#include "../classes/Actor.h"
#include "MathUtils.h"

#if PLATFORM_WINDOWS
#include "../startup/WindowsMain.cpp"
#endif

extern shader_constants_actor ActorConstants;
extern shader_constants_frame FrameConstants;

void engine::Tick()
{
	ProcessUserInput();

	MainCamera.UpdateFromInput();

	MainCamera.UpdateProjectionType(projection_type::Perspective);
	MainCamera.ViewMatrix = renderer::GenerateViewMatrix(true, MainCamera.CameraInfo, MainCamera.LookAtVector);
	RenderScene();

	// separate
	//ImGui::ShowDemoWindow();
	if (ImGui::Begin("Debug Screen"))
	{
		ImGui::Text("FPS: %f", 1 / (UserInputs.DeltaTime / 1000));
		ImGui::Text("Delta: %f", UserInputs.DeltaTime);
		//ImGui::Text("Ctrl: %d", UserInputs.KeysDown[VK_CONTROL].Pressed);
	}
	ImGui::End();

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
	verts[0] = { -0.5f, -0.5f, -0.5f, 0.f, 1.f };
	verts[1] = { -0.5f, 0.5f, -0.5f, 0.f, 0.f };
	verts[2] = { 0.5f, 0.5f, -0.5f, 1.f, 0.f };
	verts[3] = verts[0];
	verts[4] = verts[2];
	verts[5] = { 0.5f, -0.5f, -0.5f, 1.f, 1.f };

	// Back
	verts[6] = { 0.5f, -0.5f, 0.5f, 0.f, 1.f };
	verts[7] = { 0.5f, 0.5f, 0.5f, 0.f, 0.f };
	verts[8] = { -0.5f, 0.5f, 0.5f, 1.f, 0.f };
	verts[9] = verts[8];
	verts[10] = { -0.5f, -0.5f, 0.5f, 1.f, 1.f };
	verts[11] = verts[6];

	// Top
	verts[12] = verts[2]; verts[12].v = 1.f;
	verts[13] = verts[1]; verts[13].v = 1.f;
	verts[14] = verts[8]; verts[14].u = 0.f;
	verts[15] = verts[12]; 
	verts[16] = verts[14];
	verts[17] = verts[7]; verts[17].u = 1.f;

	// Bottom
	verts[18] = verts[10]; verts[18].v = 0.f;
	verts[19] = verts[0]; verts[19].u = 1.f;
	verts[20] = verts[5]; verts[20].u = 0.f;
	verts[21] = verts[6]; verts[21].v = 0.f;
	verts[22] = verts[18];
	verts[23] = verts[20];

	// Left Side
	verts[24] = verts[8]; verts[24].u = 0.f;
	verts[25] = verts[1]; verts[25].u = 1.f;
	verts[26] = verts[0]; verts[26].u = 1.f;
	verts[27] = verts[10]; verts[27].u = 0.f;
	verts[28] = verts[24];
	verts[29] = verts[26];

	// Right Side
	verts[30] = verts[5]; verts[30].u = 0.f;
	verts[31] = verts[2]; verts[31].u = 0.f;
	verts[32] = verts[7]; verts[32].u = 1.f;
	verts[33] = verts[30];
	verts[34] = verts[32];
	verts[35] = verts[6]; verts[35].u = 1.f;

	cMeshAsset* mesh = new cMeshAsset();
	mesh->VertexCount = 36;
	mesh->Data = verts;
	mesh->Loaded = true;
	AssetRegistry.push_back(mesh);

	MemoryManager.Initialize();
	Renderer.Initialize(Window, WindowWidth, WindowHeight);
	MainLevel.Initialize();

#if SPADE_DEBUG
	assetLoader::ScanAssets("assets\\", false);
	assetLoader::InitializeAssetsInDirectory("assets\\", &(assetLoader::asset_load_callbacks)AssetLoadCallbacks);
#else
	assetLoader::InitializeAssetsFromPac(&AssetLoadCallbacks);
#endif

	for (u32 i = 0; i < 5; i++)
	{
		s32 matid = CreateMaterial();
		material& Mat = GetMaterial(matid);

		switch (i)
		{
			default:
			{ Mat.DiffuseShaderID = GetShaderIDFromName("Canned-Eggplant.jpg");
			  Mat.DiffuseColor = colors::Blue; } break;

			case 1:
			{ Mat.DiffuseShaderID = GetShaderIDFromName("chev.jpg");
			  Mat.DiffuseColor = colors::Red; } break;

			case 2:
			{ Mat.DiffuseShaderID = GetShaderIDFromName("eyes.jpg");
			  Mat.DiffuseColor = colors::Green; } break;

			case 3:
			{ Mat.DiffuseShaderID = GetShaderIDFromName("lmao.jpg");
			  Mat.DiffuseColor = colors::Orange; } break;

			case 4:
			{ Mat.DiffuseShaderID = GetShaderIDFromName("what.jpg");
			  Mat.DiffuseColor = colors::White; } break;
		}
	}

	for (u32 i = 0; i < 5000; i++)
	{
		s32 actorid = ActorComponents.CreateComponent(actor_component(&MainLevel), true);
		s32 compid = RenderingComponents.CreateComponent(rendering_component(actorid), true);
		rendering_component& rcomp = RenderingComponents.GetComponent(compid);
		actor_component& acomp = ActorComponents.GetComponent(actorid);
		acomp.SetScale(v3{ 50.f, 50.f, 50.f });
		acomp.SetLocation(v3{ (rand() % 5000) - 2500.f, (rand() % 5000) - 2500.f, (rand() % 5000) - 2500.f });

		rcomp.ActorComponentID = actorid;
		rcomp.RenderResources.MaterialID = rand() % 5;
		rcomp.RenderResources.MeshAssetID = 0;

		renderer_actor* Actor = new renderer_actor();
		Actor->ActorComponentID = actorid;
		Actor->RenderingComponentID = compid;
		MainLevel.AddActorToRegistry(Actor);
	}

}

void engine::Cleanup()
{
	Renderer.Cleanup();
}

void engine::ProcessUserInput()
{
	f32 speed = 0.5f * UserInputs.DeltaTime;
	if (UserInputs.KeysDown['A'].Pressed)
	{
		MainCamera.CameraInfo.Position += -speed * MainCamera.RightVector;
		UserInputs.PlayerMovement = true;
	}
	if (UserInputs.KeysDown['D'].Pressed)
	{
		MainCamera.CameraInfo.Position += speed * MainCamera.RightVector;
		UserInputs.PlayerMovement = true;
	}
	if (UserInputs.KeysDown['W'].Pressed)
	{
		MainCamera.CameraInfo.Position += speed * MainCamera.ForwardVector;
		UserInputs.PlayerMovement = true;
	}
	if (UserInputs.KeysDown['S'].Pressed)
	{
		MainCamera.CameraInfo.Position += -speed * MainCamera.ForwardVector;
		UserInputs.PlayerMovement = true;
	}

	// move to platform layer / optimize
	if (UserInputs.KeysDown[VK_CONTROL].Pressed)
		while (ShowCursor(true) <= 0);
	else
		while (ShowCursor(false) >= 0);
		
}

bool CompareRenderComponents(rendering_component& Comp1, rendering_component& Comp2)
{
	return ((Comp1.RenderResources.MaterialID == Comp2.RenderResources.MaterialID) && (Comp1.RenderResources.MeshAssetID < Comp2.RenderResources.MeshAssetID)) || (Comp1.RenderResources.MaterialID < Comp2.RenderResources.MaterialID);
}

// todo: multithreading
void engine::RenderScene()
{
	std::vector<rendering_component>& RCRegistry = RenderingComponents.GetRegistry();

	// Sort by material ID then by meshid (optimize?)
	std::sort(RCRegistry.begin(), RCRegistry.end(), CompareRenderComponents);
	s32 MaterialID = -1;
	s32 AssetID = -1;
	cMeshAsset* Asset = nullptr;

	FrameConstants.ViewProjectionMatrix = MainCamera.ProjectionMatrix * MainCamera.ViewMatrix;
	Renderer.MapConstants(map_operation::Frame);

	u32 InstanceCount = 0;
	for (u32 i = 0; i < RCRegistry.size(); i++)
	{
		if (RCRegistry[i].Active && RCRegistry[i].RenderResources.MaterialID != -1 && RCRegistry[i].ActorComponentID != -1 && RCRegistry[i].RenderResources.MeshAssetID != -1)
		{
			if (RCRegistry[i].RenderResources.MeshAssetID != AssetID || i == RCRegistry.size() - 1 || InstanceCount >= MAX_INSTANCES) // mesh changed, draw previous instances if there are any
			{
				if (InstanceCount > 0)
				{
					Renderer.MapConstants(map_operation::Actor);
					Renderer.DrawInstanced((vertex*)Asset->Data, Asset->VertexCount, InstanceCount, draw_topology_types::TriangleList);
					InstanceCount = 0;
				}
			 
				AssetID = RCRegistry[i].RenderResources.MeshAssetID;
				Asset = (cMeshAsset*)AssetRegistry[AssetID];
			}
			if (RCRegistry[i].RenderResources.MaterialID != MaterialID) // material changed, draw previous instances if there are any
			{
				if (InstanceCount > 0)
				{
					Renderer.MapConstants(map_operation::Actor);
					Renderer.DrawInstanced((vertex*)Asset->Data, Asset->VertexCount, InstanceCount, draw_topology_types::TriangleList);
					InstanceCount = 0;
				}
				MaterialID = RCRegistry[i].RenderResources.MaterialID;
				Renderer.BindMaterial(GetMaterial(MaterialID));
			}

			actor_component& ActorComp = ActorComponents.GetComponent(RCRegistry[i].ActorComponentID);
			if (ActorComp.Active)
			{
				transform FinalRenderTransform = ActorComp.GetTransform() + RCRegistry[i].RenderResources.LocalTransform;
				FinalRenderTransform.Scale = ActorComp.GetScale() * RCRegistry[i].RenderResources.LocalTransform.Scale;
				ActorConstants.Instances[InstanceCount].WorldMatrix = renderer::GenerateWorldMatrix(FinalRenderTransform); // todo: cached world matrix?
				InstanceCount++;
			}
		}
	}

}


// remove
s32 engine::CreateMaterial()
{
	material Mat;
	Mat.MaterialID = MaterialsAdded;
	MaterialRegistry.push_back(Mat);
	return MaterialsAdded++;
}

material& engine::GetMaterial(s32 ID)
{
	if (ID != -1)
	{
		u32 size = (u32)MaterialRegistry.size();
		for (u32 i = 0; i < size; i++)
		{
			if (MaterialRegistry[i].MaterialID == ID)
				return MaterialRegistry[i];
		}
		Assert(1 == 2); // not found
	}
	Assert(1 == 2); // cant get id of -1
	return MaterialRegistry[0];
}
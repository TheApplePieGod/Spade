#pragma once

#define TEXTURE_ASSET_ID 1
#define FONT_ASSET_ID 2
#define MESH_ASSET_ID 3
#define MATERIAL_ASSET_ID 4

/*
* Returns index into array, not (deprecated) assetid
*/
s32 GetAssetIDFromName(const char* Name, bool IsFullPath = false);

/*
* Returns index into array
*/
s32 GetTextureIDFromName(const char* Name, bool IsFullPath = false);

/*
* Returns index into array
*/
s32 GetShaderIDFromName(const char* Name);

namespace assetCallbacks
{
	void ImageLoadCallback(cAsset* Asset);
	void FontLoadCallback(cAsset* Asset);
	void MeshLoadCallback(cAsset* Asset);
	void MaterialLoadCallback(cAsset* Asset);
}

// Extended asset types
namespace assetTypes
{
	// material
	struct material_data
	{
		s32 ComponentID = -1; // internal use w asset

		v4 DiffuseColor = colors::White;
		f32 Reflectivity = 0.f;

		// texture locations
		char DiffuseFileName[MAX_PATH] = "";
		char NormalFileName[MAX_PATH] = "";
		char ReflectiveFileName[MAX_PATH] = "";
	};

	bool Material_GetDataForWriting(char*& Out_ExtraData, char*& Out_RawData, u32& Out_ExtraDataSize, u32& Out_RawDataSize, char* FilePath);
	cAsset* Material_InitializeData(cAsset* AssetDefaults, char* ExtraData, u32 ExtraDataSize);

	static asset_type MaterialType = { MATERIAL_ASSET_ID, "Material", {"emf"}, Material_GetDataForWriting, Material_InitializeData, assetCallbacks::MaterialLoadCallback }; // Set callback separately

	struct cMaterialAsset : public cAsset
	{
		void LoadAssetData() override;
		void UnloadAsset() override;
	};
}
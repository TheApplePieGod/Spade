#include "pch.h"
#include "../engine/Engine.h"
#include "AssetUtils.h"

extern engine* Engine;

void assetCallbacks::ImageLoadCallback(cAsset* Asset)
{
	Asset->LoadAssetData();
	Engine->AssetRegistry.push_back(Asset);
	Engine->TextureRegistry.push_back((cTextureAsset*)Asset);
	Engine->Renderer.RegisterTexture(Asset, true);
}

void assetCallbacks::FontLoadCallback(cAsset* Asset)
{
	Asset->LoadAssetData();
	Engine->AssetRegistry.push_back(Asset);
	Engine->Renderer.RegisterTexture(Asset, false);
}

void assetCallbacks::MeshLoadCallback(cAsset* Asset)
{
	Asset->LoadAssetData();
	Engine->AssetRegistry.push_back(Asset);
}

void assetCallbacks::MaterialLoadCallback(cAsset* Asset)
{
	// Dont load material data until after all assets have been loaded
	Engine->AssetRegistry.push_back(Asset);
}

bool assetTypes::Material_GetDataForWriting(char*& Out_ExtraData, char*& Out_RawData, u32& Out_ExtraDataSize, u32& Out_RawDataSize, char* FilePath)
{
	void* Data = nullptr;
	u32 FileSize = 0;

	if (assetLoader::LoadAllFileData(FilePath, Data, FileSize))
	{
		// Set fields (no extra data)
		Out_RawData = (char*)Data;
		Out_RawDataSize = FileSize;

		return true;
	}
	else
		return false;
}

cAsset* assetTypes::Material_InitializeData(cAsset* AssetDefaults, char* ExtraData, u32 ExtraDataSize)
{
	cMaterialAsset* MatAsset = new cMaterialAsset();
	MatAsset->CopyFields(AssetDefaults);

	return MatAsset;
}

void assetTypes::cMaterialAsset::LoadAssetData()
{
	UnloadAsset();
	cAsset::LoadAssetData();

	material Material;
	material_data* MatData = ((material_data*)Data);
	Material.Deserialize(*MatData);

	MatData->ComponentID = Engine->MaterialRegistry.CreateComponent(Material);
}

void assetTypes::cMaterialAsset::UnloadAsset()
{
	if (Loaded)
	{
		material_data* MaterialData = (material_data*)Data;

		if (MaterialData->ComponentID != -1)
			Engine->MaterialRegistry.DeleteComponent(MaterialData->ComponentID);

		cAsset::UnloadAsset();
	}
}

s32 GetAssetIDFromName(const char* Name, bool IsFullPath)
{
	char Buffer[MAX_PATH];
	_snprintf_s(Buffer, sizeof(Buffer), "%s", Name);

	for (u32 i = 0; i < Engine->AssetRegistry.size(); i++)
	{
		// assumes filename is already formatted
		if (IsFullPath)
		{
			if (strcmp(Buffer, Engine->AssetRegistry[i]->Path) == 0)
				return i;
		}
		else
		{
			if (strcmp(Buffer, Engine->AssetRegistry[i]->Filename) == 0)
				return i;
		}
	}
	//Assert(1 == 2);
	return -1;
}

/*
* Returns index into array
*/
s32 GetTextureIDFromName(const char* Name, bool IsFullPath)
{
	char Buffer[MAX_PATH];
	_snprintf_s(Buffer, sizeof(Buffer), "%s", Name); // remove?

	for (u32 i = 0; i < Engine->TextureRegistry.size(); i++)
	{
		// assumes filename is already formatted
		if (IsFullPath)
		{
			if (strcmp(Buffer, Engine->TextureRegistry[i]->Path) == 0)
				return i;
		}
		else
		{
			if (strcmp(Buffer, Engine->TextureRegistry[i]->Filename) == 0)
				return i;
		}
	}
	//Assert(1 == 2);
	return -1;
}

/*
* Returns index into array
*/
s32 GetShaderIDFromName(const char* Name)
{
	char Buffer[MAX_PATH];
	_snprintf_s(Buffer, sizeof(Buffer), "%s", Name);

	for (u32 i = 0; i < Engine->ShaderRegistry.size(); i++)
	{
		// assumes filename is already formatted
		if (strcmp(Buffer, Engine->ShaderRegistry[i].Name) == 0)
		{
			return i;
		}
	}
	//Assert(1 == 2);
	return -1;
}
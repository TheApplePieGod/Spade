#include "pch.h"
#include "AssetUtils.h"
#include "../engine/Engine.h"

extern engine* Engine;

void assetCallbacks::ImageLoadCallback(cAsset* Asset)
{
	Engine->AssetRegistry.push_back(Asset);
	Engine->TextureRegistry.push_back((cTextureAsset*)Asset);
	Engine->Renderer.RegisterTexture(Asset, true);
}

void assetCallbacks::FontLoadCallback(cAsset* Asset)
{
	Engine->AssetRegistry.push_back(Asset);
	Engine->Renderer.RegisterTexture(Asset, false);
}

void assetCallbacks::MeshLoadCallback(cAsset* Asset)
{
	Engine->AssetRegistry.push_back(Asset);
}

s32 GetAssetIDFromName(const char* Name)
{
	char Buffer[MAX_PATH];
	_snprintf_s(Buffer, sizeof(Buffer), "%s", Name);

	for (u32 i = 0; i < Engine->AssetRegistry.size(); i++)
	{
		// assumes filename is already formatted
		if (strcmp(Buffer, Engine->AssetRegistry[i]->Filename) == 0)
		{
			//return GlobalAssetRegistry[i]->AssetID;
			return i;
		}
	}
	//Assert(1 == 2);
	return -1;
}

/*
* Returns index into array
*/
s32 GetTextureIDFromName(const char* Name)
{
	char Buffer[MAX_PATH];
	_snprintf_s(Buffer, sizeof(Buffer), "%s", Name);

	for (u32 i = 0; i < Engine->TextureRegistry.size(); i++)
	{
		// assumes filename is already formatted
		if (strcmp(Buffer, Engine->TextureRegistry[i]->Filename) == 0)
		{
			return i;
		}
	}
	Assert(1 == 2);
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
	Assert(1 == 2);
	return -1;
}
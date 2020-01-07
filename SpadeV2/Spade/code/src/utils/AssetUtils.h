#pragma once

/*
* Returns index into array, not (deprecated) assetid
*/
//static s32 GetAssetIDFromName(const char* Name)
//{
//	char Buffer[MAX_PATH];
//	_snprintf_s(Buffer, sizeof(Buffer), "%s", Name);
//
//	for (u32 i = 0; i < GlobalAssetRegistry.Num(); i++)
//	{
//		// assumes filename is already formatted
//		if (strcmp(Buffer, GlobalAssetRegistry[i]->Filename) == 0)
//		{
//			//return GlobalAssetRegistry[i]->AssetID;
//			return i;
//		}
//	}
//	Assert(1 == 2);
//	return -1;
//}
//
///*
//* Returns index into array
//*/
//static s32 GetShaderIDFromName(const char* Name)
//{
//	char Buffer[MAX_PATH];
//	_snprintf_s(Buffer, sizeof(Buffer), "%s", Name);
//
//	for (u32 i = 0; i < GlobalTextureRegistry.Num(); i++)
//	{
//		// assumes filename is already formatted
//		if (strcmp(Buffer, GlobalTextureRegistry[i]->Filename) == 0)
//		{
//			return i;
//		}
//	}
//	Assert(1 == 2);
//	return 0;
//}

namespace assetCallbacks
{
	void ImageLoadCallback(cTextureAsset* Asset);
	void FontLoadCallback(cFontAsset* Asset);
}

const assetLoader::asset_load_callbacks AssetLoadCallbacks = { assetCallbacks::ImageLoadCallback, assetCallbacks::FontLoadCallback };
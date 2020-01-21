#pragma once

/*
* Returns index into array, not (deprecated) assetid
*/
s32 GetAssetIDFromName(const char* Name);

/*
* Returns index into array
*/
s32 GetTextureIDFromName(const char* Name);

/*
* Returns index into array
*/
s32 GetShaderIDFromName(const char* Name);

namespace assetCallbacks
{
	void ImageLoadCallback(cTextureAsset* Asset);
	void FontLoadCallback(cFontAsset* Asset);
}

const assetLoader::asset_load_callbacks AssetLoadCallbacks = { assetCallbacks::ImageLoadCallback, assetCallbacks::FontLoadCallback };
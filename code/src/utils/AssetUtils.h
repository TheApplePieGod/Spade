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
	void ImageLoadCallback(cAsset* Asset);
	void FontLoadCallback(cAsset* Asset);
	void MeshLoadCallback(cAsset* Asset);
}
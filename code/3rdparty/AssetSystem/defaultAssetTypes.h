#pragma once

namespace defaultAssetTypes
{
	// image
	struct image_data
	{
		s32 Width;
		s32 Height;
		s32 Channels;
	};

	bool Image_GetDataForWriting(char*& Out_ExtraData, char*& Out_RawData, u32& Out_ExtraDataSize, u32& Out_RawDataSize, char* FilePath);
	cAsset* Image_InitializeData(cAsset* AssetDefaults, char* Data, u32 DataSize);

	static asset_type ImageType = { 1, "Image", {"png", "jpg", "tga"}, Image_GetDataForWriting, Image_InitializeData }; // Set callback separately


	// font
	struct font_data // assumes one channel texture
	{
		s32 Ascent;
		s32 Descent;
		s32 LineGap;
		f32 ScaleMultiplier;
		s32 CharsPerRow;
		s32 BoxHeight;
		s32 BoxWidth;
		s32 AtlasDim;
		s32 AtlasDataSize;
		u32 NumChars;
	};

	struct kern_entry
	{
		u32 AsciiVal1;
		u32 AsciiVal2;
		float Spacing;
	};

	struct char_entry // assumes one channel texture
	{
		u32 AsciiValue;
		u32 Width;
		u32 Height;
		f32 OffsetX;
		f32 OffsetY;
		f32 AdvanceWidth;
		f32 LeftSideBearing;
		u32 GlyphDataLength;
		s32 TopLeftOffsetX;
		s32 TopLeftOffsetY;
	};

	bool Font_GetDataForWriting(char*& Out_ExtraData, char*& Out_RawData, u32& Out_ExtraDataSize, u32& Out_RawDataSize, char* FilePath);
	cAsset* Font_InitializeData(cAsset* AssetDefaults, char* Data, u32 DataSize);

	static asset_type FontType = { 2, "Font", {"ttf"}, Font_GetDataForWriting, Font_InitializeData }; // Set callback separately


	// mesh
	// Structure used when storing vertices from imported mesh
	struct vertex
	{
		// Position in x/y plane
		f32 x, y, z;

		// UV coordinates
		f32 u, v;

		// Normals
		f32 nx, ny, nz;

		// Tangents
		f32 tx, ty, tz;
	};

	struct mesh_data
	{
		u32 NumVertices;
	};

	bool Mesh_GetDataForWriting(char*& Out_ExtraData, char*& Out_RawData, u32& Out_ExtraDataSize, u32& Out_RawDataSize, char* FilePath);
	cAsset* Mesh_InitializeData(cAsset* AssetDefaults, char* Data, u32 DataSize);

	static asset_type MeshType = { 3, "Mesh", {"fbx"}, Mesh_GetDataForWriting, Mesh_InitializeData }; // Set callback separately
};

struct cTextureAsset : public cAsset
{
	defaultAssetTypes::image_data ImageData;

#ifdef ASSET_DIRECTX11
	ID3D11Texture2D* TextureHandle = nullptr;
	ID3D11ShaderResourceView* ShaderHandle = nullptr;
#endif

	// Register texture with rendering sdk by calling assetLoader::Register(your_sdk)Texture
	void UnloadAsset() override;
};

struct cFontAsset : public cAsset
{
	defaultAssetTypes::font_data FontData;

#ifdef ASSET_DIRECTX11
	ID3D11ShaderResourceView* AtlasShaderHandle = nullptr;
#endif

	u32 NumChars;
	u32 NumKernVals;
	defaultAssetTypes::char_entry* Characters;
	defaultAssetTypes::kern_entry* KernValues;

	// Register atlas tex with rendering sdk by calling assetLoader::Register(your_sdk)Texture
	void UnloadAsset() override;

	inline defaultAssetTypes::char_entry FindCharEntryByAscii(u32 AsciiVal)
	{
		for (u32 i = 0; i < NumChars; i++)
		{
			if (Characters[i].AsciiValue == AsciiVal)
				return Characters[i];
		}
		return defaultAssetTypes::char_entry();
	}
};

struct cMeshAsset : public cAsset
{
	defaultAssetTypes::mesh_data MeshData;
};
#if !defined(ASSET_H)

enum asset_type
{
    Invalid,
    Texture,
    FBX,
    Font,
};

struct cAsset
{
    s32 AssetID;
    b32 Active;
    bool Loaded;
    asset_type Type;
    void* Data = nullptr;
    char Filename[MAX_PATH];

    virtual void LoadAssetData() = 0;
    virtual void UnloadAsset();
};

struct cTextureAsset : public cAsset
{
    s32 Width;
    s32 Height;
    s32 Channels;

    void LoadAssetData() override;
    void UnloadAsset() override;
};

struct cMeshAsset : public cAsset
{
    u32 VertexCount;

    void LoadAssetData() override;
};

struct cFontAsset : public cAsset
{
    font_pack FontData;
    ID3D11ShaderResourceView* AtlasShaderHandle;
    cArray<char_entry> Characters;
    cArray<kern_entry> KernValues;

    void LoadAssetData() override;
    char_entry FindCharEntryByAscii(u32 AsciiVal);
};

struct texture_entry
{
    bool Success = false;
    cTextureAsset* Asset;
    ID3D11Texture2D* TextureHandle;
    ID3D11ShaderResourceView* AssociatedShaderHandle;
};

#define ASSET_H
#endif

#include "asset.h"


void cAsset::UnloadAsset()
{

}

void cTextureAsset::LoadAssetData() //todo: update method of loading / unloading (remove new)
{
    FILE* pak = fopen("Test.pac", "rb");
    asset_header SearchingHeader;
    long pos;
    if (FindAssetByID(AssetID, SearchingHeader, pos))
    {
        char* Pixels = new char [SearchingHeader.DataSize];
        fseek(pak, pos + sizeof( png_pack ), SEEK_CUR);
        fread(Pixels, 1, SearchingHeader.DataSize, pak);

        Loaded = true;
        Data = Pixels;
    }
    else
    {
        Assert(1==2); // for now
        Loaded = false;
        Data = nullptr;
    }
    fclose(pak);
}

void cTextureAsset::UnloadAsset()
{

}

void cMeshAsset::LoadAssetData()
{
    FILE* pak = fopen("Test.pac", "rb");
    asset_header SearchingHeader;
    long pos;
    if (FindAssetByID(AssetID, SearchingHeader, pos))
    {
        vertex* Vertices = new vertex [SearchingHeader.DataSize];
        fseek(pak, pos, SEEK_CUR);
        fread(Vertices, 1, SearchingHeader.DataSize, pak);

        Loaded = true;
        Data = Vertices;
    }
    else
    {
        Assert(1==2); // for now
        Loaded = false;
        Data = nullptr;
    }
    fclose(pak);
}

void cFontAsset::LoadAssetData()
{
    FILE* pak = fopen("Test.pac", "rb");
    asset_header SearchingHeader;
    long pos;
    if (FindAssetByID(AssetID, SearchingHeader, pos))
    {
        char* Pixels = new char [SearchingHeader.DataSize];
        fseek(pak, pos + sizeof(font_pack) + SearchingHeader.ExtraSize, SEEK_CUR);
        fread(Pixels, 1, SearchingHeader.DataSize, pak);

        Loaded = true;
        Data = Pixels;
    }
    else
    {
        Assert(1==2); // for now
        Loaded = false;
        Data = nullptr;
    }
    fclose(pak);
}

char_entry cFontAsset::FindCharEntryByAscii(u32 AsciiVal)
{
    for (u32 i = 0; i < Characters.Num(); i++)
    {
        if (Characters[i].AsciiValue == AsciiVal)
            return Characters[i];
    }
    return char_entry();
}
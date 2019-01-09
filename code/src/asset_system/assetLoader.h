#if !defined(ASSETLOADER_H)

#include "../../lib/meow/meow_intrinsics.h"
#include "../../lib/meow/meow_hash.h"

struct saved_hash
{
    meow_hash Hash;
    s32 AssociatedID = 0;
};

struct png_asset_data // remove
{
    s32 Width;
    s32 Height;
    s32 Channels;
    u32 DataLength;
    unsigned char* PixelData;
};

struct png_pack // change to texture_pack
{
    s32 Width;
    s32 Height;
    s32 Channels;
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

struct font_pack // assumes one channel texture
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

struct asset_header
{
    s32 ID;
    s32 Type; // 0 = texture, 1 = fbx, 2 = font(ttf)
    u32 NextItem; // tells where the next asset starts
    u32 ExtraSize; // extra value for extra data sizes
    u32 DataSize; // tells the size of raw asset data
    char Filename[MAX_PATH];

    inline bool operator==(asset_header A)
    {
        if (A.ID == ID &&
            A.Type == Type &&
            A.NextItem == NextItem &&
            A.ExtraSize == ExtraSize &&
            A.DataSize == DataSize)
            return true;
        else
            return false;
    }
};

template <typename T>
struct asset_read
{
    asset_header Header;
    T ReadingType; // ex: png_pack
};

static bool FindAssetByID(s32 ID, asset_header& OutHeader, long& PosInPak) // bad
{
    FILE* pak = fopen("Test.pac", "rb");
    asset_header PrevHeader;
    while (true) // iterate through assets (optimize?)
    {
        size_t d = fread((char*)&OutHeader, 1, sizeof(OutHeader), pak);
        if (PrevHeader == OutHeader) // when all assets have been read
        {
            fclose(pak);
            return false;
        }
        else if (OutHeader.ID == ID)
        {
            PosInPak = ftell(pak);
            fclose(pak);
            return true;
        }
        else
        {
            PrevHeader = OutHeader;
            fseek(pak, OutHeader.NextItem, SEEK_CUR);
        }
    };
    fclose(pak);
    return false;
}

#define ASSETLOADER_H
#endif
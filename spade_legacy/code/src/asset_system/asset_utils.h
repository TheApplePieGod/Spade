#if !defined(ASSET_UTILS_H)

static s32 GetAssetIDFromName(const char* Name)
{
    char Buffer[MAX_PATH];
    _snprintf_s(Buffer, sizeof(Buffer), "%s", Name);

    for (u32 i = 0; i < GlobalAssetRegistry.Num(); i++)
    {
        // assumes filename is already formatted
        if (strcmp(Buffer, GlobalAssetRegistry[i]->Filename) == 0)
        {
            return i;
        }
    }
    Assert(1==2);
    return -1;
}

static s32 GetShaderIDFromName(const char* Name)
{
    char Buffer[MAX_PATH];
    _snprintf_s(Buffer, sizeof(Buffer), "%s", Name);

    for (u32 i = 0; i < GlobalTextureRegistry.Num(); i++)
    {
        // assumes filename is already formatted
        if (strcmp(Buffer, GlobalTextureRegistry[i].Asset->Filename) == 0)
        {
            return i;
        }
    }
    Assert(1==2);
    return 0;
}

#define ASSET_UTILS_H
#endif
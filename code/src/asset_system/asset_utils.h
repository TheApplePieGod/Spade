#if !defined(ASSET_UTILS_H)

static s32 GetAssetIDFromName(const char* Name)
{
    char Buffer[MAX_PATH];
    _snprintf_s(Buffer, sizeof(Buffer), "%s", Name);

    for (u32 i = 0; i < GlobalAssetRegistry.Num(); i++)
    {
        char Buffer2[MAX_PATH];
         _snprintf_s(Buffer2, sizeof(Buffer2), "%s", GlobalAssetRegistry[i]->Filename);
        if (strcmp(Buffer, Buffer2) == 0)
        {
            return i;
        }
    }
    Assert(1==2);
    return -1;
}

s32 SAFE_GetAssetIDFromName(char* Name)
{
    char Buffer[MAX_PATH];
    _snprintf_s(Buffer, sizeof(Buffer), "%s", Name);

    for (u32 i = 0; i < GlobalAssetRegistry.Num(); i++)
    {
        char Buffer2[MAX_PATH];
         _snprintf_s(Buffer2, sizeof(Buffer2), "%s", GlobalAssetRegistry[i]->Filename);
        if (strcmp(Buffer, Buffer2) == 0)
        {
            return i;
        }
    }
    return -1;
}

static s32 GetShaderIDFromName(const char* Name)
{
    char Buffer[MAX_PATH];
    _snprintf_s(Buffer, sizeof(Buffer), "%s", Name);

    for (u32 i = 0; i < GlobalTextureRegistry.Num(); i++)
    {
        char Buffer2[MAX_PATH];
         _snprintf_s(Buffer2, sizeof(Buffer2), "%s", GlobalTextureRegistry[i].Asset->Filename);
        if (strcmp(Buffer, Buffer2) == 0)
        {
            return i;
        }
    }
    Assert(1==2);
    return 0;
}

#define ASSET_UTILS_H
#endif
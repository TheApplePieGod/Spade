#if !defined(TILEMAP_H)

struct tileset
{
    tileset(texture_entry MapTex, s32 ShaderId, u32 SizeX, u32 SizeY)
    {
        TextureData = MapTex;
        ShaderID = ShaderId;
        TileSizeX = SizeX;
        TileSizeY = SizeY;
        if (SizeX != 0) // checks that there is a valid initilization
            NumTiles = (MapTex.Asset->Width / SizeX) * (MapTex.Asset->Height / SizeY);
    }

    // pixels
    u32 TileSizeX = 32;
    u32 TileSizeY = 32;
    
    s32 NumTiles = 0;

    s32 ShaderID = 0;
    texture_entry TextureData;
};

class tilemap : public renderingComponent
{
public:
    // tileset data is static during runtime
    tilemap(tileset Set)
    {
        SpecialType = rendering_specials::Tilemap;
        TileSet = Set;
        // populate tile array with default tile id (-1, blank)
        for (u32 i = 0; i < MapSize.x * MapSize.y; i++)
        {
            TileArray.Add((rand() % 1023) - 0, true);
        }
    }

    cArray<s32> TileArray;
    tileset TileSet = tileset(texture_entry(), 0, 0, 0); // uninitialized
    v2 MapSize = V2(400, 50);

    v2 TileSizeWorldSpace = V2(1.f, 1.f);

    /*
    * vertices are outputted in local component space
    */
    void InitializeMap()
    {
        v2 AddedPosition = V2(0.f, 0.f);
        RenderResources.Vertices.Clear();

        u32 TexWidth = TileSet.TextureData.Asset->Width;
        u32 TexHeight = TileSet.TextureData.Asset->Height;
        v2 PixelSize = V2((f32)TileSet.TileSizeX, (f32)TileSet.TileSizeY);
        v2 Size = TileSizeWorldSpace;
        for (u32 i = 0; i < TileArray.Num(); i++)
        {
            if (TileArray[i] > TileSet.NumTiles - 1) // check to make sure index isn't over set bounds
                TileArray[i] = TileSet.NumTiles - 1;
            if (i % (u32)MapSize.x == 0 && i != 0)
                AddedPosition = V2(0, AddedPosition.y - Size.y);
            if (TileArray[i] != -1)
            {
                u32 RowInTex = (TileArray[i] * TileSet.TileSizeX) / TexWidth;
                v2 BottomLeft = V2(LocalLocation.x, LocalLocation.y) + AddedPosition;
                v2 TopLeft = v2{BottomLeft.x, BottomLeft.y + Size.y};
                v2 BottomRight = v2{BottomLeft.x + Size.x, BottomLeft.y};
                v2 TopRight = v2{BottomLeft.x + Size.x, BottomLeft.y + Size.y};
                v2 TopLeftUV = V2((float)(TileArray[i] * PixelSize.x) / (float)TexWidth, (float)(RowInTex * PixelSize.y) / (float)TexHeight);
                v2 BottomRightUV = V2((float)(TileArray[i] * PixelSize.x + PixelSize.x) / (float)TexWidth, (float)(RowInTex * PixelSize.y + PixelSize.y) / (float)TexHeight);

                //RenderResources.Vertices.Add(TEXVERTEX(BottomLeft.x, BottomLeft.y, 1, TopLeftUV.u, BottomRightUV.v), true); // bottom left
                RenderResources.Vertices.Add(TEXVERTEX(TopLeft.x, TopLeft.y, 1, TopLeftUV.u, TopLeftUV.v), true); // top left
                RenderResources.Vertices.Add(TEXVERTEX(BottomRight.x, BottomRight.y, 1, BottomRightUV.u, BottomRightUV.v), true); // bottom right
                //RenderResources.Vertices.Add(TEXVERTEX(TopLeft.x, TopLeft.y, 1, TopLeftUV.u, TopLeftUV.v), true); // top left
                //RenderResources.Vertices.Add(TEXVERTEX(TopRight.x, TopRight.y, 1, BottomRightUV.u, TopLeftUV.v), true); // top right
                //RenderResources.Vertices.Add(TEXVERTEX(BottomRight.x, BottomRight.y, 1, BottomRightUV.u, BottomRightUV.v), true); // bottom right
            }
            AddedPosition += V2(TileSizeWorldSpace.x, 0);
        }
        RenderResources.ShaderID = TileSet.ShaderID;
    }

    ~tilemap()
    {
        TileArray.ManualFree();
    }
};

#define TILEMAP_H
#endif
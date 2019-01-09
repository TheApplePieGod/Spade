#define STB_IMAGE_IMPLEMENTATION
#include "../../lib/stb/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "../../lib/stb/stb_truetype.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../lib/stb/stb_image_write.h"

#include <fstream>
#include <vector>

#define FBXSDK_SHARED
#include <fbxsdk.h>

#define FONT_PIXEL_HEIGHT 40; 

static void ScanAndGeneratePac(bool Rebuild)
{
    FbxManager* FbxSdkManager = nullptr;
    std::ofstream ofs;
    WIN32_FIND_DATA data;
    HANDLE hFind = FindFirstFile("M:\\code\\src\\asset_system\\assets\\*", &data); // asset directory
    int ID = 0;
    remove("..\\code\\src\\asset_system\\assets.generated.h");
    FILE* generated = fopen("..\\code\\src\\asset_system\\assets.generated.h", "w+");

	char* HeaderComments = 
		"/* ========================================================================\n"
		"   GENERATED FILE: DO NOT EDIT\n"
		"   ======================================================================== */\n\n";

	fprintf (generated, HeaderComments);	

    if ( hFind != INVALID_HANDLE_VALUE ) 
    {
        cArray<saved_hash> ReadHashes;
        cArray<saved_hash> NewHashes;
        FILE* pak = fopen("Test.pac", "rb");
        WIN32_FIND_DATA pacdata;
        HANDLE fpac = FindFirstFile("Test.pac", &pacdata); // pac directory
        u32 HashArraySize = 0;
        fseek(pak, -(s32)sizeof(HashArraySize), SEEK_END);
        fread((char*)&HashArraySize, 1, sizeof(HashArraySize), pak);
        Assert(HashArraySize != 0);

        saved_hash* HashBuffer = new saved_hash[HashArraySize / sizeof(saved_hash)];
        fseek(pak, -(s32)(HashArraySize + sizeof(HashArraySize)), SEEK_END);
        fread(HashBuffer, 1, HashArraySize, pak);
        ReadHashes.Append(HashBuffer, HashArraySize / sizeof(saved_hash), true);

        delete[] HashBuffer;

        //meow_hash NewHash = MeowHash_Accelerated(0, size, PacBuffer);
        // meow_hash ReadHash;
        // fseek(pak, 0, SEEK_SET);
        // fread((char*)&ReadHash, 1, sizeof(meow_hash), pak);

        // fclose(pak);
        // delete[] PacBuffer;

        // if (MeowHashesAreEqual(NewHash, ReadHash))
        // {
        //     // file is the same so dont change anything
        //     return;
        // }

        ofs.open ("Temp.pac", std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
        do 
        {
            int FilenameSize = (int)strlen(data.cFileName);
            if (FilenameSize > 3) // make sure it isnt a random file
            {
                ID++;
                asset_header Header[1];
                const char Checking[3] = { data.cFileName[FilenameSize - 3], data.cFileName[FilenameSize - 2], data.cFileName[FilenameSize - 1] }; // assumes 3 char filetypes
                char* FullPath = new char[FilenameSize + 35];
                *FullPath = '\0';
                strcat(FullPath, "../code/src/asset_system/assets/");
                strcat(FullPath, data.cFileName);

                char* FilenameShort = new char[FilenameSize - 3];
                int fs = 0;
                for (fs = 0; fs < FilenameSize; fs++)
                {
                    Header[0].Filename[fs] = data.cFileName[fs];
                    if (fs < FilenameSize - 4)
                        FilenameShort[fs] = data.cFileName[fs];
                }
                FilenameShort[fs - 4] = '\0';
                Header[0].Filename[fs] = '\0';

                // change checking method
                if ((Checking[0] == 'p' && Checking[1] == 'n' && Checking[2] == 'g') ||
                    (Checking[0] == 'P' && Checking[1] == 'N' && Checking[2] == 'G') ||
                    (Checking[0] == 'j' && Checking[1] == 'p' && Checking[2] == 'g') ||
                    (Checking[0] == 'J' && Checking[1] == 'P' && Checking[2] == 'G')) // image type (hash checking is relatively negligible)
                {
                    png_asset_data PNGData;
                    png_pack PNGPack[1];
                    PNGData.PixelData = stbi_load(FullPath, &PNGData.Width, &PNGData.Height, &PNGData.Channels, 4); // load the png
                    PNGData.Channels = 4;  // makes every loaded texture have 4 channels
                    PNGData.DataLength = PNGData.Width * PNGData.Height * PNGData.Channels;
                    Header[0].ID = ID;
                    Header[0].Type = 0; // png type
                    Header[0].DataSize = PNGData.DataLength;
                    Header[0].NextItem = sizeof(png_pack) + PNGData.DataLength;
                    PNGPack[0].Width = PNGData.Width;
                    PNGPack[0].Height = PNGData.Height;
                    PNGPack[0].Channels = PNGData.Channels;
                    ofs.write((char*)Header, sizeof(asset_header));
                    ofs.write((char*)PNGPack, sizeof(png_pack));
                    ofs.write((char*)PNGData.PixelData, PNGData.DataLength);
                }
                else if ((Checking[0] == 'f' && Checking[1] == 'b' && Checking[2] == 'x') ||
                        (Checking[0] == 'F' && Checking[1] == 'B' && Checking[2] == 'X')) // fbx type
                {
                    // compare file hashes
                    u32 filesize = (data.nFileSizeHigh * ((long)MAXDWORD + 1)) + data.nFileSizeLow;
                    std::ifstream ifs (FullPath, std::ifstream::binary);
                    char* FileBuffer = new char[filesize];
                    ifs.rdbuf()->sgetn(FileBuffer, filesize);

                    saved_hash NewHash = saved_hash{MeowHash_Accelerated(0, filesize, FileBuffer), ID};
                    ifs.close();
                    delete[] FileBuffer;

                    bool found = false;
                    s32 FindID = -1;
                    for (u32 i = 0; i < ReadHashes.Num(); i++)
                    {
                        if (MeowHashesAreEqual(ReadHashes[i].Hash, NewHash.Hash))
                        {
                            found = true;
                            FindID = ReadHashes[i].AssociatedID;
                            break;
                        }
                    }

                    NewHashes.Add(NewHash, true);
                    if (Rebuild == false && found == true && FindID != -1)
                    {
                        asset_header SearchingHeader[1];
                        long pos;
                        if (FindAssetByID(FindID, SearchingHeader[0], pos))
                        {
                            fseek(pak, pos, SEEK_SET);
                            char* Data = new char[SearchingHeader[0].DataSize];
                            fread(Data, 1, SearchingHeader[0].DataSize, pak);
                            SearchingHeader[0].ID = ID;
                            ofs.write((char*)SearchingHeader, sizeof(asset_header));
                            ofs.write(Data, SearchingHeader[0].DataSize);
                            delete[] Data;
                        }
                        else
                            Assert(1==2);
                    }
                    else
                    {
                        cArray<vertex> VertexArray;
                        // initialize and load scene
                        FbxSdkManager = FbxManager::Create();

                        FbxIOSettings* pIOsettings = FbxIOSettings::Create(FbxSdkManager, IOSROOT );
                        FbxSdkManager->SetIOSettings(pIOsettings);


                        FbxImporter* pImporter = FbxImporter::Create(FbxSdkManager,"");
                        FbxScene* pFbxScene = FbxScene::Create(FbxSdkManager,"");

                        bool Success = pImporter->Initialize(FullPath, -1, FbxSdkManager->GetIOSettings() );
                        if(!Success)
                        {
                            OutputDebugMessage("Failed to initialize mesh, skipping");
                            continue;
                        }
                        Success = pImporter->Import(pFbxScene);
                        if(!Success)
                        {
                            OutputDebugMessage("Failed to initialize mesh, skipping");
                            continue;
                        }

                        FbxGeometryConverter FbxConverter(FbxSdkManager);
                        FbxAxisSystem AxisSystem = pFbxScene->GetGlobalSettings().GetAxisSystem();

                        //FbxAxisSystem::DirectX.ConvertScene(pFbxScene);
                        FbxConverter.Triangulate(pFbxScene, true);

                        pImporter->Destroy();
                        FbxNode* pFbxRootNode = pFbxScene->GetRootNode();

                        if(pFbxRootNode)
                        {
                            u32 ArrayIndex = 0;
                            for(int i = 0; i < pFbxRootNode->GetChildCount(); i++)
                            {
                                FbxNode* pFbxChildNode = pFbxRootNode->GetChild(i);

                                if(pFbxChildNode->GetNodeAttribute() == NULL)
                                    continue;

                                FbxNodeAttribute::EType AttributeType = pFbxChildNode->GetNodeAttribute()->GetAttributeType();

                                if(AttributeType != FbxNodeAttribute::eMesh) // only compatible with mesh types
                                    continue;

                                FbxMesh* pMesh = (FbxMesh*) pFbxChildNode->GetNodeAttribute();

                                FbxVector4* lControlPoints = pMesh->GetControlPoints();

                                u32 countd = pMesh->GetPolygonCount();
                                for (s32 j = 0; j < pMesh->GetPolygonCount(); j++)
                                {
                                    u32 NumVertices = pMesh->GetPolygonSize(j);
                                    //Assert( NumVertices == 3 ); // if it fails, the mesh isn't trianglulated
                                    if (NumVertices != 3)
                                    {   
                                        char Buffer[256];
                                        _snprintf_s(Buffer, sizeof(Buffer), "Mesh at ID %d isn't triangulated, skipping\n", ID);
                                        OutputDebugStringA(Buffer);
                                        goto NEXT; // continue to next item in directory
                                    }
                                    //for (u32 k = 0; k < NumVertices; k++)
                                    for (s32 k = NumVertices - 1; k >= 0; k--)
                                    {   
                                        VertexArray.Add(vertex());
                                        //vertices data
                                        u32 ControlPointIndex = pMesh->GetPolygonVertex(j, k);
                                        FbxVector4 Vertex = lControlPoints[ControlPointIndex];

                                        VertexArray[ArrayIndex].x = (float)Vertex[0];
                                        VertexArray[ArrayIndex].y = (float)Vertex[1];
                                        VertexArray[ArrayIndex].z = (float)Vertex[2];
                                        
                                        //UV data
                                        u32 ElementUVCount = pMesh->GetElementUVCount();
                                        for (u32 l = 0; l < ElementUVCount; ++l)
                                        {
                                            FbxGeometryElementUV* leUV = pMesh->GetElementUV(l);
                                            FbxVector2 UV;
                                            switch (leUV->GetMappingMode())
                                            {
                                                default:
                                                    break;
                                                case FbxGeometryElement::eByControlPoint:
                                                    switch (leUV->GetReferenceMode())
                                                    {
                                                        case FbxGeometryElement::eDirect:
                                                        {
                                                            UV = leUV->GetDirectArray().GetAt(ControlPointIndex);
                                                            break;
                                                        }
                                                        case FbxGeometryElement::eIndexToDirect:
                                                        {
                                                            int id = leUV->GetIndexArray().GetAt(ControlPointIndex);
                                                            UV = leUV->GetDirectArray().GetAt(id);
                                                            break;
                                                        }
                                                        default:
                                                            break; // other reference modes not shown here!
                                                    }
                                                    break;

                                                case FbxGeometryElement::eByPolygonVertex:
                                                    {
                                                        int lTextureUVIndex = pMesh->GetTextureUVIndex(j, k);
                                                        switch (leUV->GetReferenceMode())
                                                        {
                                                            case FbxGeometryElement::eDirect:
                                                            case FbxGeometryElement::eIndexToDirect:
                                                                {
                                                                    UV = leUV->GetDirectArray().GetAt(lTextureUVIndex);
                                                                    break;
                                                                }
                                                            default:
                                                                break; // other reference modes not shown here!
                                                        }
                                                    }
                                                    break;

                                                case FbxGeometryElement::eByPolygon: // doesn't make much sense for UVs
                                                case FbxGeometryElement::eAllSame:   // doesn't make much sense for UVs
                                                case FbxGeometryElement::eNone:       // doesn't make much sense for UVs
                                                    break;
                                            }

                                            VertexArray[ArrayIndex].u = (float)UV[0];
                                            VertexArray[ArrayIndex].v = (float)UV[1]; // Invert V due to difference in coord systems between Maya and D3D
                                        }

                                        // normal data
                                        u32 ElementNormalCount = pMesh->GetElementNormalCount();
                                        for (u32 l = 0; l < ElementNormalCount; l++)
                                        {
                                            FbxVector4 Normal;
                                            FbxGeometryElementNormal* lNormalElement = pMesh->GetElementNormal(l);
                                            if(lNormalElement)
                                            {
                                                //mapping mode is by control points. The mesh should be smooth and soft.
                                                //we can get normals by retrieving each control point
                                                if( lNormalElement->GetMappingMode() == FbxGeometryElement::eByControlPoint )
                                                {
                                                    if( lNormalElement->GetReferenceMode() == FbxGeometryElement::eDirect )
                                                    {
                                                        Normal = lNormalElement->GetDirectArray().GetAt(ControlPointIndex);
                                                    }

                                                    //reference mode is index-to-direct, get normals by the index-to-direct
                                                    if(lNormalElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
                                                    {
                                                        int id = lNormalElement->GetIndexArray().GetAt(ControlPointIndex);
                                                        Normal = lNormalElement->GetDirectArray().GetAt(id);
                                                    }
                                                }
                                                else if(lNormalElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
                                                {
                                                    switch (lNormalElement->GetReferenceMode())
                                                    {
                                                        case FbxGeometryElement::eDirect:
                                                            {
                                                                Normal = lNormalElement->GetDirectArray().GetAt(ControlPointIndex); //?
                                                                break;
                                                            }
                                                        case FbxGeometryElement::eIndexToDirect:
                                                            {
                                                                Normal = lNormalElement->GetDirectArray().GetAt(lNormalElement->GetIndexArray().GetAt(ControlPointIndex));
                                                                break;
                                                            }
                                                        default:
                                                            break;
                                                    }
                                                }
                                            }

                                            // storing normals as float3s
                                            VertexArray[ArrayIndex].nx = (float)Normal[0]; 
                                            VertexArray[ArrayIndex].ny = (float)Normal[1];
                                            VertexArray[ArrayIndex].nz = (float)Normal[2];
                                        }

                                        ArrayIndex++;
                                    }
                                }
                            }
                        }
                        FbxSdkManager->Destroy();
                        Header[0].ID = ID;
                        Header[0].Type = 1; // fbx type
                        Header[0].DataSize = VertexArray.Num() * sizeof(vertex);
                        Header[0].NextItem = Header[0].DataSize; // no fbx header
                        ofs.write((char*)Header, sizeof(asset_header));
                        ofs.write((char*)VertexArray.GetRaw(), VertexArray.Num() * sizeof(vertex));
                        VertexArray.ManualFree();
                    }
                }
                else if ((Checking[0] == 't' && Checking[1] == 't' && Checking[2] == 'f') ||
                    (Checking[0] == 'T' && Checking[1] == 'T' && Checking[2] == 'F')) // font type
                {
                    // compare file hashes
                    u32 filesize = (data.nFileSizeHigh * ((long)MAXDWORD + 1)) + data.nFileSizeLow;
                    std::ifstream ifs (FullPath, std::ifstream::binary);
                    char* FileBuffer = new char[filesize];
                    ifs.rdbuf()->sgetn(FileBuffer, filesize);

                    saved_hash NewHash = saved_hash{MeowHash_Accelerated(0, filesize, FileBuffer), ID};
                    ifs.close();

                    bool found = false;
                    s32 FindID = -1;
                    for (u32 i = 0; i < ReadHashes.Num(); i++)
                    {
                        if (MeowHashesAreEqual(ReadHashes[i].Hash, NewHash.Hash))
                        {
                            found = true;
                            FindID = ReadHashes[i].AssociatedID;
                            break;
                        }
                    }

                    NewHashes.Add(NewHash, true);
                    if (Rebuild == false && found == true && FindID != -1)
                    {
                        delete[] FileBuffer;
                        asset_header SearchingHeader[1];
                        long pos;
                        if (FindAssetByID(FindID, SearchingHeader[0], pos))
                        {
                            fseek(pak, pos, SEEK_SET);
                            char* Data = new char[sizeof(font_pack) + SearchingHeader[0].ExtraSize + SearchingHeader[0].DataSize];
                            size_t g = fread(Data, 1, sizeof(font_pack) + SearchingHeader[0].ExtraSize + SearchingHeader[0].DataSize, pak);
                            SearchingHeader[0].ID = ID;
                            ofs.write((char*)SearchingHeader, sizeof(asset_header));
                            ofs.write(Data, sizeof(font_pack) + SearchingHeader[0].ExtraSize + SearchingHeader[0].DataSize);
                            delete[] Data;
                        }
                        else
                            Assert(1==2);
                    }
                    else
                    {
                        cArray<char_entry> Characters;
                        cArray<kern_entry> KernValues;
                        font_pack FontPackData[1];
                        s32 Width, Height, Ascent, Descent, LineGap, AdvanceWidth, LeftSideBearing;
                        stbtt_fontinfo FontInfo;

                        stbtt_InitFont(&FontInfo, (unsigned char*)FileBuffer, stbtt_GetFontOffsetForIndex((unsigned char*)FileBuffer, 0));
                        stbtt_GetFontVMetrics(&FontInfo, &Ascent, &Descent, &LineGap);

                        ifs.close();

                        f32 PH = FONT_PIXEL_HEIGHT;
                        f32 ScaleMultiplier = stbtt_ScaleForPixelHeight(&FontInfo, PH);
                        Ascent *= (s32)ScaleMultiplier;
                        Descent *= (s32)ScaleMultiplier;
                        LineGap *= (s32)ScaleMultiplier;

                        s32 TextureDim = 512 * (s32)(PH / 40);
                        s32 AtlasDataSize = TextureDim * TextureDim * 1;
                        u8* AtlasBuffer = (u8*)malloc(AtlasDataSize);
                        memset(AtlasBuffer, 0, AtlasDataSize);

                        s32 CharCounter = 1;
                        s32 BoxHeight = FONT_PIXEL_HEIGHT;
                        s32 BoxWidth = FONT_PIXEL_HEIGHT;
                        s32 CurrentRow = 1;
                        s32 CharsPerRow = TextureDim / BoxWidth;

                        FontPackData[0].Ascent = Ascent;
                        FontPackData[0].Descent = Descent;
                        FontPackData[0].LineGap = LineGap;
                        FontPackData[0].ScaleMultiplier = ScaleMultiplier;
                        FontPackData[0].CharsPerRow = CharsPerRow;
                        FontPackData[0].BoxHeight = BoxHeight;
                        FontPackData[0].BoxWidth = BoxWidth;
                        FontPackData[0].AtlasDim = TextureDim;
                        FontPackData[0].AtlasDataSize = AtlasDataSize;

                        s32 CharIndex = -1;
                        for (s32 AsciiVal = 32; AsciiVal <= 126; AsciiVal++) // ascii values 32-126
                        {
                            CharIndex++;
                            Characters.Add(char_entry(), true);
                            s32 xOffset = 0;
                            s32 yOffset = 0;

                            u8* CharacterData = stbtt_GetCodepointBitmap(&FontInfo, 0.0f, ScaleMultiplier, AsciiVal, &Width, &Height, &xOffset, &yOffset);
                            stbtt_GetCodepointHMetrics(&FontInfo, AsciiVal, &AdvanceWidth, &LeftSideBearing);

                            Characters[CharIndex].AsciiValue = AsciiVal;
                            Characters[CharIndex].Width = Width;
                            Characters[CharIndex].Height = Height;
                            Characters[CharIndex].OffsetX = (xOffset * ScaleMultiplier);
                            Characters[CharIndex].OffsetY = (f32)yOffset;
                            Characters[CharIndex].AdvanceWidth = (AdvanceWidth * ScaleMultiplier);
                            Characters[CharIndex].LeftSideBearing = (LeftSideBearing * ScaleMultiplier);
                            Characters[CharIndex].GlyphDataLength = Width * Height;
                            
                            s32 TLX = CharCounter * BoxWidth;
                            s32 TLY = TextureDim * (BoxHeight * CurrentRow);

                            Characters[CharIndex].TopLeftOffsetX = TLX;
                            Characters[CharIndex].TopLeftOffsetY = TLY / TextureDim;

                            for (s32 y = 0; y < Height; y++)
                            {
                                s32 Position = TLY + (TextureDim * y) + TLX;
                                for (s32 x = 0; x < Width; x++)
                                {
                                    AtlasBuffer[Position++] = *CharacterData++;
                                }
                            }

                            CharCounter++;
                            if (CharCounter % CharsPerRow == 0)
                            {
                                CurrentRow++;
                                CharCounter = 1;
                            }

                            for (s32 KernVal = 32; KernVal <= 126; KernVal++)
                            {
                                float Spacing = (f32)stbtt_GetCodepointKernAdvance(&FontInfo, AsciiVal, KernVal) * ScaleMultiplier;

                                if (Spacing != 0)
                                {
                                    kern_entry entry;

                                    entry.AsciiVal1 = AsciiVal;
                                    entry.AsciiVal2 = KernVal;
                                    entry.Spacing = Spacing;

                                    KernValues.Add(entry, true);
                                }
                            }
                        }

                        FontPackData[0].NumChars = Characters.Num();
                        Header[0].ID = ID;
                        Header[0].Type = 2; // font type
                        Header[0].DataSize = FontPackData[0].AtlasDataSize;
                        Header[0].ExtraSize = (sizeof(char_entry) * Characters.Num()) + (sizeof(kern_entry) * KernValues.Num());
                        Header[0].NextItem = sizeof(FontPackData[0]) + Header[0].ExtraSize + FontPackData[0].AtlasDataSize;

                        ofs.write((char*)Header, sizeof(asset_header));
                        ofs.write((char*)FontPackData, sizeof(FontPackData));
                        ofs.write((char*)Characters.GetRaw(), sizeof(char_entry) * Characters.Num());
                        ofs.write((char*)KernValues.GetRaw(), sizeof(kern_entry) * KernValues.Num());
                        ofs.write((char*)AtlasBuffer, FontPackData[0].AtlasDataSize);

                        char Buffer[256];
                        _snprintf_s(Buffer, sizeof(Buffer), "../code/src/asset_system/fonts/%s.png", FilenameShort);
                        stbi_write_png(Buffer, TextureDim, TextureDim, 1, AtlasBuffer, TextureDim);

                        delete[] FileBuffer;
                        free(AtlasBuffer);
                        Characters.ManualFree();
                        KernValues.ManualFree();
                    }
                }
                else
                {
                    NEXT: continue;
                }

                data.cFileName[FilenameSize - 4] = '_'; // change dot in filetype to underscore for #define
                fprintf(generated, "#define %s %d\n", data.cFileName, ID);
                delete[] FullPath;
                delete[] FilenameShort;
            }
        } 
        while (FindNextFile(hFind, &data));
        {
            FindClose(hFind);
        }

        u32 ArraySize[1] = {NewHashes.Num() * sizeof(saved_hash)};
        ofs.write((char*)NewHashes.GetRaw(), ArraySize[0]);
        ofs.write((char*)ArraySize, sizeof(u32));
        ofs.close();
        fclose(pak);
        fclose(generated);

        ofs.open ("Test.pac", std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
        WIN32_FIND_DATA tempdata;
        std::ifstream ifs ("Temp.pac", std::ifstream::binary);
        FindFirstFile("Temp.pac", &tempdata);
        u32 size = (tempdata.nFileSizeHigh * ((long)MAXDWORD + 1)) + tempdata.nFileSizeLow;
        char* FinalBuffer = new char[size];
        ifs.rdbuf()->sgetn(FinalBuffer, size);

        ofs.write(FinalBuffer, size);

        ofs.close();
        ifs.close();
        remove("Temp.pac");
        ReadHashes.ManualFree();
        NewHashes.ManualFree();
    }
}

static void InitializeAssets() // assets should never change, so new is OK (later add int array of ids to load)
{
    FILE* pak = fopen("Test.pac", "rb");
    s32 PrevIndex = -1;
    asset_header SearchingHeader;
    while (true) // iterate through assets (optimize?)
    {
        fread((char*)&SearchingHeader, 1, sizeof(SearchingHeader), pak);
        if (PrevIndex == SearchingHeader.ID) // when all assets have been read
        {
            fclose(pak);
            return;
        }
        else
        {
            PrevIndex = SearchingHeader.ID; // TODO: stop using new
            if (SearchingHeader.Type == 0) // texture asset found
            {
                png_pack reading;
                fread((char*)&reading, 1, sizeof(reading), pak);
                cTextureAsset* TexAsset = new cTextureAsset;
                TexAsset->AssetID = SearchingHeader.ID;
                TexAsset->Type = asset_type::Texture; // texture
                strncpy(TexAsset->Filename, SearchingHeader.Filename, MAX_PATH);
                TexAsset->Width = reading.Width;
                TexAsset->Height = reading.Height;
                TexAsset->Channels = reading.Channels;
                GlobalAssetRegistry.Add(TexAsset);

                TexAsset->LoadAssetData();
                LoadAndRegisterTexture(TexAsset);

                fseek(pak, SearchingHeader.DataSize, SEEK_CUR); // set pos to next asset
            }
            else if (SearchingHeader.Type == 1) // fbx asset found
            {
                cMeshAsset* MeshAsset = new cMeshAsset;
                MeshAsset->AssetID = SearchingHeader.ID;
                MeshAsset->Type = asset_type::FBX; // fbx
                strncpy(MeshAsset->Filename, SearchingHeader.Filename, MAX_PATH);
                MeshAsset->VertexCount = SearchingHeader.DataSize / sizeof(vertex);
                GlobalAssetRegistry.Add(MeshAsset);

                MeshAsset->LoadAssetData();

                fseek(pak, SearchingHeader.DataSize, SEEK_CUR);
            }
            else if (SearchingHeader.Type == 2) // font asset found
            {
                cFontAsset* FontAsset = new cFontAsset;
                fread((char*)&FontAsset->FontData, 1, sizeof(font_pack), pak);
                char_entry* CharArray = new char_entry[FontAsset->FontData.NumChars * sizeof(char_entry)];
                kern_entry* KernArray = new kern_entry[SearchingHeader.ExtraSize - (FontAsset->FontData.NumChars * sizeof(char_entry))];
                fread((char*)CharArray, 1, FontAsset->FontData.NumChars * sizeof(char_entry), pak);
                fread((char*)KernArray, 1, SearchingHeader.ExtraSize - (FontAsset->FontData.NumChars * sizeof(char_entry)), pak);
                FontAsset->Characters.Append(CharArray, FontAsset->FontData.NumChars);
                FontAsset->KernValues.Append(KernArray, (SearchingHeader.ExtraSize - (FontAsset->FontData.NumChars * sizeof(char_entry))) / sizeof(kern_entry));
                FontAsset->AssetID = SearchingHeader.ID;
                FontAsset->Type = asset_type::Font; // font
                strncpy(FontAsset->Filename, SearchingHeader.Filename, MAX_PATH);
                GlobalAssetRegistry.Add(FontAsset);

                FontAsset->LoadAssetData();

                FontAsset->AtlasShaderHandle = LoadFontAtlas(FontAsset);

                fseek(pak, SearchingHeader.DataSize, SEEK_CUR);
            }
            else
            {
                fseek(pak, SearchingHeader.NextItem, SEEK_CUR);
            }
        }
    }
}
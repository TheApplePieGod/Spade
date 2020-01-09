#include <DXGI1_6.h>
#include <D3DCommon.h>
#include <d3d11.h>
#include <DirectXMath.h>   
#include <D3Dcompiler.h>
#include <Box2D.h>
#include "../../lib/misc/fast_dynamic_cast.h"

#include "../utils/main.h"
#include "../utils/cmath.h"
#include "../engine/memory.cpp"

static memory_manager* MainManager = new memory_manager(true);

#include "../utils/cArray.h"
#include "../utils/dx.h"

#include "../asset_system/assetLoader.h" // dependency reasons
#include "../asset_system/asset.cpp"
#include "../engine/material.cpp"

cArray<texture_entry> GlobalTextureRegistry = cArray<texture_entry>(MainManager, memory_lifetime::Permanent); // stores all loaded textures
cArray<material> GlobalMaterialRegistry = cArray<material>(MainManager, memory_lifetime::Permanent); // stores all initialized materials
cArray<cAsset*> GlobalAssetRegistry = cArray<cAsset*>(MainManager, memory_lifetime::Permanent); // stores all loaded assets

#include "../asset_system/asset_utils.h"
#include "../engine/renderer.cpp"
#include "../asset_system/assetLoader.cpp"
#include "../engine/camera.cpp"
#include "../classes/widget/widget.cpp"
#include "../classes/components/renderingComponent.cpp"
#include "../engine/actor.h"
#include "../engine/lighting.cpp"
#include "../classes/level.cpp"

#include "../classes/tilemap.cpp"
#include "../engine/actor.cpp"
#include "../../game/globalInclude.h"

#include "../engine/editor.cpp"

// add all subcatagories into one .h

camera* MainCamera = nullptr;
b32 IsRunning = true;
user_inputs* UserInputs = new user_inputs;
float LightAngle = 270;
float ClearColor[4] = {1.f, 1.f, 1.f, 1.f};
int WidgetCount = 0;
level* MainLevel = new level_Level2D; // no multiple level per world support

cArray<user_widget*> WidgetRegistry = cArray<user_widget*>(MainManager, memory_lifetime::Permanent);

HWND Window;

static void ProcessUserInputs()
{
    POINT MousePos;
    GetCursorPos(&MousePos);
    ScreenToClient(Window, &MousePos);
    UserInputs->MousePosX = (((2 * MousePos.x) / (f32)UserScreenSizeX) - 1) / MainCamera->GetProjectionMatrix().m11;
    UserInputs->MousePosY = -(((2 * MousePos.y) / (f32)UserScreenSizeY) - 1) / MainCamera->GetProjectionMatrix().m22;

    #if SPADE_3D
    if (UserInputs->forward.Pressed)
    {
        MainCamera->Position = (MainCamera->SpeedForward * UserInputs->DeltaTime * MainCamera->ForwardVector) + MainCamera->Position;
    }
    if (UserInputs->backward.Pressed)
    {
        MainCamera->Position = (-MainCamera->SpeedForward * UserInputs->DeltaTime * MainCamera->ForwardVector) + MainCamera->Position;
    }
    #endif
    if (UserInputs->left.Pressed)
    {
        MainCamera->Position = (-MainCamera->SpeedSide * UserInputs->DeltaTime * MainCamera->RightVector) + MainCamera->Position;
    }
    if (UserInputs->right.Pressed)
    {
        MainCamera->Position = (MainCamera->SpeedSide * UserInputs->DeltaTime * MainCamera->RightVector) + MainCamera->Position;
    }
}

static void ProcessKeyboardMessage(u32 Key, keyboard_button* InputRef)
{
    b32 IsDown = (((unsigned short) GetKeyState(Key)) >> 15);
   
    if (IsDown)
    {
        InputRef->WasPressed = false;
        InputRef->TransitionCount++;
    }
    else
    {
        if (InputRef->TransitionCount >= 1 && !InputRef->WasPressed)
        {
            InputRef->WasPressed = true;
            InputRef->TransitionCount++;
        }
    }

    InputRef->Pressed = IsDown;
}

static void ProcessPendingMessages()
{
    int MouseDeltaBufferX = 0;
    int MouseDeltaBufferY = 0;
    MSG Message;
    while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
    {
        switch (Message.message)
        {
            case WM_INPUT:
            {
                UINT dwSize;

                GetRawInputData((HRAWINPUT)Message.lParam, RID_INPUT, NULL, &dwSize, 
                                sizeof(RAWINPUTHEADER));
                LPBYTE lpb = new BYTE[dwSize];
                if (lpb == NULL) 
                {
                   break;
                }

                if (GetRawInputData((HRAWINPUT)Message.lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize )
                    OutputDebugMessage("GetRawInputData does not return correct size !"); 

                RAWINPUT* raw = (RAWINPUT*)lpb;
            
                if (raw->header.dwType == RIM_TYPEMOUSE) 
                {
                    int xPosRelative = raw->data.mouse.lLastX;
                    int yPosRelative = raw->data.mouse.lLastY;

                    MouseDeltaBufferX += xPosRelative;
                    MouseDeltaBufferY += yPosRelative;

                    //OutputDebugFloat((f32)xPosRelative);

                    // Need to re-generalize this so that we do not need to repeat this
                    // for each button type. See Win32ProcessKeyboardMessage above.

                    int LButton = raw->data.mouse.usButtonFlags;
                    if (LButton == RI_MOUSE_LEFT_BUTTON_DOWN)
                    {
                        UserInputs->lmb.WasPressed = false;
                        UserInputs->lmb.TransitionCount++;
                        
                        UserInputs->lmb.Pressed = true;
                    }
                    if (LButton == RI_MOUSE_LEFT_BUTTON_UP)
                    {
                        if (UserInputs->lmb.TransitionCount >= 1 && !UserInputs->lmb.WasPressed)
                        {
                            UserInputs->lmb.WasPressed = true;
                            UserInputs->lmb.TransitionCount++;
                        }

                        if (GetFocus() != Window) // reset window focus if clicked & out of focus
                        {
                            SetFocus(Window);
                        }
                        UserInputs->lmb.Pressed = false;
                    }
                    UserInputs->MouseMovement = true;
                } 
                break;
            }
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                u32 KeyCode = (u32)Message.wParam;
                b32 WasDown = ((Message.lParam & (1 << 30)) != 0);
                b32 IsDown = ((Message.lParam & (1 << 31)) == 0);

                if(true)//if(WasDown != IsDown)
                {
                    if (KeyCode == 'W')
                    {
                        ProcessKeyboardMessage(KeyCode, &UserInputs->forward);
                    }
                    if (KeyCode == 'A')
                    {
                        ProcessKeyboardMessage(KeyCode, &UserInputs->left);
                    }
                    if (KeyCode == 'S')
                    {
                        ProcessKeyboardMessage(KeyCode, &UserInputs->backward);
                    }
                    if (KeyCode == 'D')
                    {
                        ProcessKeyboardMessage(KeyCode, &UserInputs->right);
                    }
                    if(IsDown)
                    {
                        if(KeyCode == VK_ESCAPE)
                        {
                            IsRunning = false;
                        }
                    }
                }
                break;
            }
            default:
            {
                TranslateMessage(&Message);
                DispatchMessage(&Message);
                break;
            }
        }
    }
    UserInputs->MousePosX += MouseDeltaBufferX;
    UserInputs->MousePosY += MouseDeltaBufferY;
    MainCamera->Update(V2((float)MouseDeltaBufferX, (float)MouseDeltaBufferY), UserInputs->MouseMovement);
    UserInputs->MouseMovement = false;
}

// static void LightActorIteration(bool Paraboloid = false) // update to support instancing
// {
//     DirectX::XMFLOAT4X4 vtemp;
//     vtemp._11 = constants.ViewProj.m11;
//     vtemp._12 = constants.ViewProj.m12;
//     vtemp._13 = constants.ViewProj.m13;
//     vtemp._14 = constants.ViewProj.m14;
//     vtemp._21 = constants.ViewProj.m21;
//     vtemp._22 = constants.ViewProj.m22;
//     vtemp._23 = constants.ViewProj.m23;
//     vtemp._24 = constants.ViewProj.m24;
//     vtemp._31 = constants.ViewProj.m31;
//     vtemp._32 = constants.ViewProj.m32;
//     vtemp._33 = constants.ViewProj.m33;
//     vtemp._34 = constants.ViewProj.m34;
//     vtemp._41 = constants.ViewProj.m41;
//     vtemp._42 = constants.ViewProj.m42;
//     vtemp._43 = constants.ViewProj.m43;
//     vtemp._44 = constants.ViewProj.m44;

//     DirectX::XMMATRIX ViewProj = XMMatrixTranspose(DirectX::XMLoadFloat4x4(&vtemp));

//     for (u32 i = 0; i < ActorRegistry.Num(); i++)
//     {
//         if (ActorRegistry[i].Flag == actor_flag::PendingDestroy)
//         {
//             // destroy actor
//             continue;
//         }
//         //ActorRegistry[i].Tick();
//         if (ActorRegistry[i].Visible)
//         {   
//             //ActorRegistry[i].Scale = ActorRegistry[i].Scale - V3(0.05, 0, 0.05);
//             for (u32 z = 0; z < ActorRegistry[i].RenderingComponents.Num(); z++) // OPTIMIZE: Dont draw if off screen
//             {
//                 if (ActorRegistry[i].RenderingComponents[z].Visible && ActorRegistry[i].RenderingComponents[z].CastShadow)
//                 {
//                     v3 FinalLocation = ActorRegistry[i].WorldLocation + ActorRegistry[i].RenderingComponents[z].LocalLocation;
//                     if (false) // fix
//                     {
//                         ActorRegistry[i].RenderingComponents[z].UpdateBoundingBox(ActorRegistry[i].Scale * ActorRegistry[i].RenderingComponents[z].LocalScale); // remove
//                         mesh_render_resources Resources = ActorRegistry[i].RenderingComponents[z].RenderResources;
//                         //cArray<v3> BoxPoints = cArray<v3>(MainManager, memory_lifetime::Frame);
//                         v3 BoxPoints[8];

//                         BoxPoints[0] = (Resources.BoundingBoxMin);
//                         BoxPoints[1] = (Resources.BoundingBoxMax);
//                         BoxPoints[2] = (V3(Resources.BoundingBoxMax.x, Resources.BoundingBoxMin.y, Resources.BoundingBoxMin.z));
//                         BoxPoints[3] = (V3(Resources.BoundingBoxMin.x, Resources.BoundingBoxMax.y, Resources.BoundingBoxMin.z));
//                         BoxPoints[4] = (V3(Resources.BoundingBoxMin.x, Resources.BoundingBoxMin.y, Resources.BoundingBoxMax.z));
//                         BoxPoints[5] = (V3(Resources.BoundingBoxMin.x, Resources.BoundingBoxMax.y, Resources.BoundingBoxMax.z));
//                         BoxPoints[6] = (V3(Resources.BoundingBoxMax.x, Resources.BoundingBoxMin.y, Resources.BoundingBoxMax.z));
//                         BoxPoints[7] = (V3(Resources.BoundingBoxMax.x, Resources.BoundingBoxMax.y, Resources.BoundingBoxMin.z));

//                         for (u32 k = 0; k < 8; k++) // asuming box points never changes
//                         {
//                             DirectX::XMVECTOR pos = DirectX::XMVectorSet(BoxPoints[k].x + FinalLocation.x,
//                                                                         BoxPoints[k].y + FinalLocation.y,
//                                                                         BoxPoints[k].z + FinalLocation.z, 1);
                            
//                             pos = DirectX::XMVector3Transform(pos, ViewProj);

//                             DirectX::XMFLOAT4 v4d;
//                             DirectX::XMStoreFloat4(&v4d, pos);
//                             v3 clip = V3(v4d.x, v4d.y, v4d.z);
//                             clip = clip / v4d.w;

//                             f32 ClipDistance = 1;
//                             if (clip.x <= ClipDistance && clip.x >= -ClipDistance && clip.y <= ClipDistance && clip.y >= -ClipDistance && clip.z > 0)
//                             {
//                                 //OutputDebugMessage("Drawing");
//                                 //MaterialIndexes[ActorRegistry[i].RenderingComponents[z].RenderResources.ShaderID].Add(INTV2(i, z), true);
//                                 break;
//                             }
//                         }
//                     }
//                     else
//                     {
//                         v3 FinalRenderLocation = ActorRegistry[i].WorldLocation + ActorRegistry[i].RenderingComponents[z].LocalLocation;
//                         rotator FinalRotation = ActorRegistry[i].Rotation + ActorRegistry[i].RenderingComponents[z].LocalRotation;
//                         v3 FinalScale = ActorRegistry[i].Scale * ActorRegistry[i].RenderingComponents[z].LocalScale;

//                         D3D11_MAPPED_SUBRESOURCE Mapped;
//                         if (Paraboloid)
//                         {
//                             ParaboloidCs.RenderLocation = FinalRenderLocation;
//                             ParaboloidCs.Scale = ActorRegistry[i].Scale * ActorRegistry[i].RenderingComponents[z].LocalScale;

//                             DeviceContext->Map(ParaboloidConstants, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &Mapped);
//                             paraboloid_constants* dataPtr = (paraboloid_constants*)Mapped.pData;
//                             *dataPtr = ParaboloidCs;
//                             DeviceContext->Unmap(ParaboloidConstants, NULL);
//                         }
//                         else
//                         {
//                             //map per rendering comp constants and draw
//                             DirectX::XMMATRIX translation, rotationx, rotationy, rotationz, scale;
//                             translation = DirectX::XMMatrixTranslation(FinalRenderLocation.x, FinalRenderLocation.y, FinalRenderLocation.z);
//                             rotationx = DirectX::XMMatrixRotationX(FinalRotation.x * (Pi32 / 180.0f)); // convert degrees to radians
//                             rotationy = DirectX::XMMatrixRotationY(FinalRotation.y * (Pi32 / 180.0f));
//                             rotationz = DirectX::XMMatrixRotationZ(FinalRotation.z * (Pi32 / 180.0f));
//                             scale = DirectX::XMMatrixScaling(FinalScale.x, FinalScale.y, FinalScale.z);

//                             constants.World = scale * rotationx * rotationy * rotationz * translation; // transform order: scale, rotate (degrees), translate

//                             DeviceContext->Map(ConstantBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &Mapped);
//                             shader_constants* dataPtr = (shader_constants*)Mapped.pData;
//                             *dataPtr = constants;
//                             DeviceContext->Unmap(ConstantBuffer, NULL);
//                         }
//                         Draw(MainBuffer, ActorRegistry[i].RenderingComponents[z].RenderResources.Vertices);
//                     }
//                 }
//             }
//         }
//     }
// }

static void ActorIteration(bool ShadowPass = false, bool Paraboloid = false)
{
    DirectX::XMFLOAT4X4 vtemp;
    vtemp._11 = constants.ViewProj.m11;
    vtemp._12 = constants.ViewProj.m12;
    vtemp._13 = constants.ViewProj.m13;
    vtemp._14 = constants.ViewProj.m14;
    vtemp._21 = constants.ViewProj.m21;
    vtemp._22 = constants.ViewProj.m22;
    vtemp._23 = constants.ViewProj.m23;
    vtemp._24 = constants.ViewProj.m24;
    vtemp._31 = constants.ViewProj.m31;
    vtemp._32 = constants.ViewProj.m32;
    vtemp._33 = constants.ViewProj.m33;
    vtemp._34 = constants.ViewProj.m34;
    vtemp._41 = constants.ViewProj.m41;
    vtemp._42 = constants.ViewProj.m42;
    vtemp._43 = constants.ViewProj.m43;
    vtemp._44 = constants.ViewProj.m44;

    DirectX::XMMATRIX ViewProj = XMMatrixTranspose(DirectX::XMLoadFloat4x4(&vtemp));

    cArray<cArray<intV2>> MaterialIndexes = cArray<cArray<intV2>>(MainManager, memory_lifetime::Frame);
    for (u32 i = 0; i < GlobalMaterialRegistry.Num(); i++)
    {
        MaterialIndexes.Add(cArray<intV2>(MainManager, memory_lifetime::Frame));
    }

    for (u32 i = 0; i < MainLevel->ActorRegistry.Num(); i++)
    {
        if (MainLevel->ActorRegistry[i].Flag == actor_flag::PendingDestroy)
        {
            // destroy actor
            continue;
        }
        //ActorRegistry[i].Tick();
        if (MainLevel->ActorRegistry[i].Visible)
        {   
            if (MainLevel->ActorRegistry[i].PhysicsProperties.EnablePhysics)
            {
                b2Vec2 Pos = MainLevel->ActorRegistry[i].B2DPhysicsBody->GetPosition();
                MainLevel->ActorRegistry[i].WorldLocation = V3(Pos.x, Pos.y, MainLevel->ActorRegistry[i].WorldLocation.z);
            }
            //ActorRegistry[i].Scale = ActorRegistry[i].Scale - V3(0.05, 0, 0.05);
            for (u32 z = 0; z < MainLevel->ActorRegistry[i].RenderingComponents.Num(); z++) // OPTIMIZE: Dont draw if off screen
            {
                if (MainLevel->ActorRegistry[i].RenderingComponents[z].Visible)
                {
                    v3 FinalLocation = MainLevel->ActorRegistry[i].WorldLocation + MainLevel->ActorRegistry[i].RenderingComponents[z].LocalLocation;
                    if (false) // fix
                    {
                        MainLevel->ActorRegistry[i].RenderingComponents[z].UpdateBoundingBox(MainLevel->ActorRegistry[i].Scale * MainLevel->ActorRegistry[i].RenderingComponents[z].LocalScale); // remove
                        mesh_render_resources Resources = MainLevel->ActorRegistry[i].RenderingComponents[z].RenderResources;
                        //cArray<v3> BoxPoints = cArray<v3>(MainManager, memory_lifetime::Frame);
                        v3 BoxPoints[8];

                        BoxPoints[0] = (Resources.BoundingBoxMin);
                        BoxPoints[1] = (Resources.BoundingBoxMax);
                        BoxPoints[2] = (V3(Resources.BoundingBoxMax.x, Resources.BoundingBoxMin.y, Resources.BoundingBoxMin.z));
                        BoxPoints[3] = (V3(Resources.BoundingBoxMin.x, Resources.BoundingBoxMax.y, Resources.BoundingBoxMin.z));
                        BoxPoints[4] = (V3(Resources.BoundingBoxMin.x, Resources.BoundingBoxMin.y, Resources.BoundingBoxMax.z));
                        BoxPoints[5] = (V3(Resources.BoundingBoxMin.x, Resources.BoundingBoxMax.y, Resources.BoundingBoxMax.z));
                        BoxPoints[6] = (V3(Resources.BoundingBoxMax.x, Resources.BoundingBoxMin.y, Resources.BoundingBoxMax.z));
                        BoxPoints[7] = (V3(Resources.BoundingBoxMax.x, Resources.BoundingBoxMax.y, Resources.BoundingBoxMin.z));

                        for (u32 k = 0; k < 8; k++) // asuming box points never changes
                        {
                            DirectX::XMVECTOR pos = DirectX::XMVectorSet(BoxPoints[k].x + FinalLocation.x,
                                                                        BoxPoints[k].y + FinalLocation.y,
                                                                        BoxPoints[k].z + FinalLocation.z, 1);
                            
                            pos = DirectX::XMVector3Transform(pos, ViewProj);

                            DirectX::XMFLOAT4 v4d;
                            DirectX::XMStoreFloat4(&v4d, pos);
                            v3 clip = V3(v4d.x, v4d.y, v4d.z);
                            clip = clip / v4d.w;

                            f32 ClipDistance = 1;
                            if (clip.x <= ClipDistance && clip.x >= -ClipDistance && clip.y <= ClipDistance && clip.y >= -ClipDistance && clip.z > 0)
                            {
                                //OutputDebugMessage("Drawing");
                                MaterialIndexes[MainLevel->ActorRegistry[i].RenderingComponents[z].RenderResources.ShaderID].Add(INTV2(i, z), true);
                                break;
                            }
                        }
                    }
                    else
                    {
                        MaterialIndexes[MainLevel->ActorRegistry[i].RenderingComponents[z].RenderResources.ShaderID].Add(INTV2(i, z), true);
                    }
                }
            }
        }
    }

    u32 CurrentShader = 0;
    for (u32 i = 0; i < MaterialIndexes.Num(); i++)
    {
        if (MaterialIndexes[i].Num() == 0)
            continue;
        if (GlobalMaterialRegistry[i].bUsesTextureDiffuse) // increase shader resources for normal/diffuse/etc textures
            DeviceContext->PSSetShaderResources(0, 1, &GlobalMaterialRegistry[i].DiffuseTexture->AssociatedShaderHandle);

        MaterialConstants.DiffuseColor = GlobalMaterialRegistry[i].DiffuseColor;
        MaterialConstants.SpecularPower = GlobalMaterialRegistry[i].SpecularPower;
        MaterialConstants.SamplerID = GlobalMaterialRegistry[i].SampleType;

        D3D11_MAPPED_SUBRESOURCE Mapped;
        DeviceContext->Map(MaterialBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &Mapped);
        material_shader_constants* dataPtr2 = (material_shader_constants*)Mapped.pData;
        *dataPtr2 = MaterialConstants;
        DeviceContext->Unmap(MaterialBuffer, NULL);

        for (u32 z = 0; z < MaterialIndexes[i].Num(); z++)
        {
            intV2 Accessing = MaterialIndexes[i][z];
            v3 FinalRenderLocation = MainLevel->ActorRegistry[Accessing.x].WorldLocation + MainLevel->ActorRegistry[Accessing.x].RenderingComponents[Accessing.y].LocalLocation;
            rotator FinalRotation = MainLevel->ActorRegistry[Accessing.x].Rotation + MainLevel->ActorRegistry[Accessing.x].RenderingComponents[Accessing.y].LocalRotation;
            v3 FinalScale = MainLevel->ActorRegistry[Accessing.x].Scale * MainLevel->ActorRegistry[Accessing.x].RenderingComponents[Accessing.y].LocalScale;

            if (MainLevel->ActorRegistry[Accessing.x].RenderingComponents[Accessing.y].SpecialType == rendering_specials::Tilemap)
            {
                if (CurrentShader != 1)
                {
                    DeviceContext->VSSetShader(QuadVertexShader, NULL, 0);
                    DeviceContext->PSSetShader(TwoDFontPixShader, NULL, 0);
                    DeviceContext->GSSetShader(QuadGeometryShader, NULL, 0);
                    CurrentShader = 1;
                }
            }
            else
            {
                if (CurrentShader != 0)
                {
                    if (ShadowPass)
                    {
                        DeviceContext->VSSetShader(MainVertexShader, NULL, 0);
                        DeviceContext->PSSetShader(MainPixelShader, NULL, 0);
                    }
                    else
                    {
                        DeviceContext->VSSetShader(TwoDFontVertShader, NULL, 0);
                        DeviceContext->PSSetShader(TwoDFontPixShader, NULL, 0);
                    }
                    DeviceContext->GSSetShader(NULL, NULL, 0);
                    CurrentShader = 0;
                }
            }

            if (Paraboloid)
            {
                ParaboloidCs.RenderLocation = FinalRenderLocation;
                ParaboloidCs.Scale = MainLevel->ActorRegistry[Accessing.x].Scale * MainLevel->ActorRegistry[Accessing.x].RenderingComponents[Accessing.y].LocalScale;

                DeviceContext->Map(ParaboloidConstants, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &Mapped);
                paraboloid_constants* dataPtr = (paraboloid_constants*)Mapped.pData;
                *dataPtr = ParaboloidCs;
                DeviceContext->Unmap(ParaboloidConstants, NULL);
            }
            else
            {
                //map per rendering comp constants and draw
                DirectX::XMMATRIX translation, rotationx, rotationy, rotationz, scale;
                translation = DirectX::XMMatrixTranslation(FinalRenderLocation.x, FinalRenderLocation.y, FinalRenderLocation.z);
                rotationx = DirectX::XMMatrixRotationX(FinalRotation.x * (Pi32 / 180.0f)); // convert degrees to radians
                rotationy = DirectX::XMMatrixRotationY(FinalRotation.y * (Pi32 / 180.0f));
                rotationz = DirectX::XMMatrixRotationZ(FinalRotation.z * (Pi32 / 180.0f));
                scale = DirectX::XMMatrixScaling(FinalScale.x, FinalScale.y, FinalScale.z);

                constants.World = scale * rotationx * rotationy * rotationz * translation; // transform order: scale, rotate (degrees), translate
                constants.color = GlobalMaterialRegistry[i].DiffuseColor;
                constants.has_texture = GlobalMaterialRegistry[i].bUsesTextureDiffuse;
                constants.padding = false; // IsFont; assumes always false when talking about an actor
                //constants.test = DirectX::XMMatrixTranspose( DirectX::XMMatrixInverse(nullptr, scale * rotationx * rotationy * rotationz * translation) );

                DeviceContext->Map(ConstantBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &Mapped);
                shader_constants* dataPtr = (shader_constants*)Mapped.pData;
                *dataPtr = constants;
                DeviceContext->Unmap(ConstantBuffer, NULL);
            }

            Draw(MainBuffer, MainLevel->ActorRegistry[Accessing.x].RenderingComponents[Accessing.y].RenderResources.Vertices, !MainLevel->ActorRegistry[Accessing.x].RenderingComponents[Accessing.y].SpecialType == rendering_specials::Tilemap);
        }
    }
    //reset shader to make sure it is the same as when it started
    if (CurrentShader != 0)
    {
        if (ShadowPass)
        {
            DeviceContext->VSSetShader(MainVertexShader, NULL, 0);
            DeviceContext->PSSetShader(MainPixelShader, NULL, 0);
        }
        else
        {
            DeviceContext->VSSetShader(TwoDFontVertShader, NULL, 0);
            DeviceContext->PSSetShader(TwoDFontPixShader, NULL, 0);
        }
        DeviceContext->GSSetShader(NULL, NULL, 0);
    }
}

static void LightIteration(bool UseDirectionalLight, int NumCascades = 3, float TexSize = 2048) // optimize projections to only be set when light is created not every frame
{
    light_shader_input DirLight;
    int StartingLightIndex = 0;
    if (UseDirectionalLight)
    {
        StartingLightIndex = 1;
        float Radians = LightAngle * 0.0174532925f;
        DirLight.Direction = V3(sinf(Radians), cosf(Radians), 0);
        DirLight.Color = V4(1.f, 1.f, 0.9f, 1.f);
        DirLight.Intensity = 7.f;
        DirLight.LightType = 0;
        DirLight.Enabled = true;
        DirLight.CastShadow = true;
        v3 d = -20000 * DirLight.Direction + (30 * MainCamera->ForwardVector); // edit max depth to fix
        DirLight.Position = V3(d.x, d.y, d.z);
        DirLight.ViewMatrix = GetViewMatrix(projection_type::ORTHOGRAPHIC, V3(d.x + MainCamera->Position.x, d.y + MainCamera->Position.y, d.z + MainCamera->Position.z), MainCamera->Position + (30 * MainCamera->ForwardVector));
        LightConstants.Lights[0] = DirLight;
        LightConstants.UsesDirectional = true;
    }
    else
        LightConstants.UsesDirectional = false;

    light_shader_input PointLight;
    PointLight.Color = V4(1.f, 1.f, 0.9f, 1.f);
    PointLight.Position = MainCamera->Position + V3(0, 0, -0);
    PointLight.ConstantAttenuation = 1.f;
    PointLight.LinearAttenuation = 5.08f;
    PointLight.QuadraticAttenuation = 5.0f;
    PointLight.Intensity = 5000.f;
    PointLight.ViewMatrix = GetViewMatrix(projection_type::PERSPECTIVE, PointLight.Position, V3(0, 0, 1));
    PointLight.LightType = 1;
    PointLight.Enabled = true;
    PointLight.CastShadow = true;

    //map the lighting buffer
    D3D11_MAPPED_SUBRESOURCE Mapped;
    LightConstants.EyePosition = V4(MainCamera->Position.x, MainCamera->Position.y, MainCamera->Position.z, 1);
    LightConstants.Lights[0] = DirLight;
    //LightConstants.Lights[1] = PointLight;
    LightConstants.GlobalAmbient = V4(0.1, 0.1, 0.1, 1);

    DeviceContext->Map(LightingBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &Mapped);
    light_shader_constants* dataPtr = (light_shader_constants*)Mapped.pData;
    *dataPtr = LightConstants;
    DeviceContext->Unmap(LightingBuffer, NULL);

    //DeviceContext->VSSetShader(TwoDFontVertShader, NULL, 0);
    //DeviceContext->PSSetShader(TwoDFontPixShader, NULL, 0);

    if(UseDirectionalLight && DirLight.Enabled && DirLight.CastShadow && NumCascades > 0) // still only support for one directional light
    {
        DXSetViewport(TexSize, TexSize);

        for (int i = 0; i < NumCascades; i++)
        {
            DeviceContext->OMSetRenderTargets(0, NULL, ShadowStencilView[i]);
            DeviceContext->ClearDepthStencilView(ShadowStencilView[i],  D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0 );

            // f32 clipSpaceNear = 0.f;
            // f32 clipSpaceFar = 1.f;
            // v3 min = V3(0.f, 0.f, 0.f);
            // v3 max = V3(0.f, 0.f, 0.f);
            // v3 clipCorners[8] = 
            // {
            //     V3( -1,  1, clipSpaceNear ),
            //     V3(  1,  1, clipSpaceNear ), 
            //     V3(  1, -1, clipSpaceNear ),
            //     V3( -1, -1, clipSpaceNear ), 
            //     V3( -1,  1, clipSpaceFar  ),
            //     V3(  1,  1, clipSpaceFar  ),
            //     V3(  1, -1, clipSpaceFar  ),
            //     V3( -1, -1, clipSpaceFar  )
            // };
            // DirectX::XMMATRIX Inverse = DirectX::XMMatrixInverse(NULL, MainCamera->GenerateWorldMatrix() * M4ToDXM(MainCamera->GenerateViewMatrix(projection_type::PERSPECTIVE, false)) * M4ToDXM(MainCamera->GetPerspectiveProjectionLH(false)));
            // //DirectX::XMMATRIX Inverse = DirectX::XMMatrixInverse(NULL, M4ToDXM(MainCamera->GenerateViewMatrix(projection_type::PERSPECTIVE, false)) * M4ToDXM(MainCamera->GetPerspectiveProjectionLH(false)));
            // //DirectX::XMMATRIX Inverse = DirectX::XMMatrixInverse(NULL, M4ToDXM(MainCamera->GetPerspectiveProjectionLH(false)) * M4ToDXM(MainCamera->GenerateViewMatrix(projection_type::PERSPECTIVE, false)) * MainCamera->GenerateWorldMatrix());
            // //DirectX::XMMATRIX Inverse = DirectX::XMMatrixInverse(NULL, M4ToDXM(MainCamera->GetPerspectiveProjectionLH(false)) * M4ToDXM(MainCamera->GenerateViewMatrix(projection_type::PERSPECTIVE, false)));
            // for (int k = 0; k < 8; k++)
            // {
            //     DirectX::XMVECTOR V = DirectX::XMVectorSet(clipCorners[k].x, clipCorners[k].y, clipCorners[k].z, 1);
            //     V = DirectX::XMVector3Transform(V, Inverse);
            //     DirectX::XMFLOAT4 F;
            //     DirectX::XMStoreFloat4(&F, V);
            //     F.x = F.x / F.w;
            //     F.y = F.y / F.w;
            //     F.z = F.z / F.w;
            //     clipCorners[k] = V3(F.x, F.y, F.z);
            //     if ( F.x < min.x ) min.x = F.x;
            //     if ( F.y < min.y ) min.y = F.y;
            //     if ( F.z < min.z ) min.z = F.z;
            //     if ( F.x > max.x ) max.x = F.x;
            //     if ( F.y > max.y ) max.y = F.y;
            //     if ( F.z > max.z ) max.z = F.z;
            // }
            // DirectX::XMMATRIX translation, rotationx, rotationy, rotationz;
            // translation = DirectX::XMMatrixTranslation(MainCamera->Position.x, MainCamera->Position.y, MainCamera->Position.z);
            // rotationx = DirectX::XMMatrixRotationX(DirLight.Direction.x); // convert degrees to radians
            // rotationy = DirectX::XMMatrixRotationY(DirLight.Direction.y);
            // rotationz = DirectX::XMMatrixRotationZ(DirLight.Direction.z);

            // DirectX::XMMATRIX InverseLight = DirectX::XMMatrixInverse(NULL, rotationx * rotationy * rotationz * translation);

            // DirectX::XMVECTOR Mi = DirectX::XMVector3Transform(DirectX::XMVectorSet(min.x, min.y, min.z, 1), InverseLight);
            // DirectX::XMVECTOR Ma = DirectX::XMVector3Transform(DirectX::XMVectorSet(max.x, max.y, max.z, 1), InverseLight);
            // DirectX::XMFLOAT3 MMi;
            // DirectX::XMFLOAT3 MMa;
            // DirectX::XMStoreFloat3(&MMi, Mi);
            // DirectX::XMStoreFloat3(&MMa, Ma);
            // min = V3(MMi.x, MMi.y, MMi.z);
            // max = V3(MMa.x, MMa.y, MMa.z);
            // MainLevel->ActorRegistry[6].WorldLocation = clipCorners[4];
            // MainLevel->ActorRegistry[7].WorldLocation = clipCorners[5];
            // MainLevel->ActorRegistry[8].WorldLocation = clipCorners[6];
            // MainLevel->ActorRegistry[9].WorldLocation = clipCorners[7];
            //DirLight.ProjectionMatrix = GetOrthographicProjection(490 - (200 * i), 490 - (200 * i), 0.5, 25000); // each cascade is 200 wider
            DirLight.ProjectionMatrix = GetOrthographicProjection(100, 100, 0.5, 25000); // no cascades
            //DirLight.ProjectionMatrix = GetOrthographicProjectionBounded(min.x, max.x, min.y, max.y, 0, 25000); // each cascade is 200 wider
            constants.ViewProj = DirLight.ProjectionMatrix * DirLight.ViewMatrix;

            //constants.ViewProj = constants.ViewProj * CropMatrix;

            LightConstants.CascadeMatrices[i] = DirLight.ProjectionMatrix;

            ActorIteration(true, false);
        }
    }

    // DeviceContext->VSSetShader(ParaboloidVertShader, NULL, 0);
    // DeviceContext->PSSetShader(ParaboloidPixShader, NULL, 0);
    // DeviceContext->VSSetConstantBuffers(0, 1, &ParaboloidConstants);
    // DXSetViewport(1024, 1024);

    // for (int i = 0; i < 2; i++)
    // {
    //     DeviceContext->OMSetRenderTargets(0, NULL, ParaboloidStencilViews[i]);
    //     DeviceContext->ClearDepthStencilView(ParaboloidStencilViews[i],  D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0 );

    //     D3D11_MAPPED_SUBRESOURCE Mapped;
    //     ParaboloidCs.NearPlane = 0.5;
    //     ParaboloidCs.FarPlane = 6000;
    //     if (i == 0)
    //         ParaboloidCs.LightViewMatrix = GetViewMatrix(projection_type::PERSPECTIVE, PointLight.Position, V3(0, 0, 1));
    //     else
    //         ParaboloidCs.LightViewMatrix = GetViewMatrix(projection_type::PERSPECTIVE, PointLight.Position, V3(0, 0, -1));

    //     DeviceContext->Map(ParaboloidConstants, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &Mapped);
    //     paraboloid_constants* dataPtr = (paraboloid_constants*)Mapped.pData;
    //     *dataPtr = ParaboloidCs;
    //     DeviceContext->Unmap(ParaboloidConstants, NULL);

    //     ActorIteration(true, true);
    // }

    // remap lighting constants after changes
    LightConstants.Lights[0] = DirLight;

    DeviceContext->Map(LightingBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &Mapped);
    dataPtr = (light_shader_constants*)Mapped.pData;
    *dataPtr = LightConstants;
    DeviceContext->Unmap(LightingBuffer, NULL);
}

void WidgetComponentArrayIteration(cArray<widget_component*> Array, u32 CurrentWidgetIndex, int* CurrentShader);
void RenderWidgetComponent(widget_component* Component, v2 ParentPosition, u32 CurrentWidgetIndex, int* ShaderVar)
{
    if (Component->ParentComponent != nullptr)
        ParentPosition += Component->ParentComponent->Position;
    switch (Component->GetType())
    {
        case None:
        case VerticalBox: // no support for nested boxes yet
        case HorizontalBox:
            break;
        case Text: // text widget extent is updated every frame (in case font/text changes)
        {
            if (ShaderVar != 0)
            {
                DeviceContext->VSSetShader(TwoDFontVertShader, NULL, 0);
                DeviceContext->PSSetShader(TwoDFontPixShader, NULL, 0);
                *ShaderVar = 0;
            }
            u32 FontID = GetAssetIDFromName("Candara.ttf");
            DeviceContext->PSSetShaderResources(0, 1, &((cFontAsset*)GlobalAssetRegistry[FontID])->AtlasShaderHandle);
            //widget_component_text* Text = fast_dynamic_cast<widget_component_text*>(Component);
            widget_component_text* Text = (widget_component_text*)(Component);
            Component->Extent = RenderTextAtPosition(Text->Text.c_str(), ParentPosition, Text->Scale, Text->Rotation, Text->Color, (cFontAsset*)GlobalAssetRegistry[FontID]);
            break;
        }
        case Border:
        {
            if (ShaderVar != 0)
            {
                DeviceContext->VSSetShader(TwoDFontVertShader, NULL, 0);
                DeviceContext->PSSetShader(TwoDFontPixShader, NULL, 0);
                *ShaderVar = 0;
            }
            widget_component_border Border = *(widget_component_border*)Component;
            if (Border.UseImageBackground)
            {
                u32 ImageID = GetShaderIDFromName(Border.ImageName.c_str());
                DeviceContext->PSSetShaderResources(0, 1, &GlobalTextureRegistry[ImageID].AssociatedShaderHandle);
            }
            RenderQuad(Border.UseImageBackground, ParentPosition, Border.Scale, Border.Extent, Border.Tint);
            break;
        }
    }
    if (Component->Children.Num() > 0)
    {
        WidgetComponentArrayIteration(Component->Children, CurrentWidgetIndex, ShaderVar);
    }
}

void WidgetComponentArrayIteration(cArray<widget_component*> Array, u32 CurrentWidgetIndex, int* CurrentShader)
{
    for (u32 x = 0; x < Array.Num(); x++)
    {
        WidgetCount++;
        switch (Array[x]->GetType())
        {
            case None:
            {
                WidgetCount++;
                if (Array[x]->Children.Num() > 0)
                {
                    WidgetComponentArrayIteration(Array[x]->Children, CurrentWidgetIndex, CurrentShader);
                }
                break;
            }
            case Text:
            case Border:
            {
                WidgetCount++;
                RenderWidgetComponent(Array[x], WidgetRegistry[CurrentWidgetIndex]->Position + Array[x]->Position, CurrentWidgetIndex, CurrentShader);
                break;
            }
            case HorizontalBox:
            {
                WidgetCount++;
                widget_component_horizontalbox* Horiz = (widget_component_horizontalbox*)Array[x];
                f32 Padding = Horiz->Padding;
                bool FillRight = Horiz->FillRight;
                f32 NextPosition = 0.0f;
                for (u32 z = 0; z < Horiz->Children.Num(); z++)
                {
                    WidgetCount++;
                    RenderWidgetComponent(Horiz->Children[z], WidgetRegistry[CurrentWidgetIndex]->Position + Horiz->Position + V2(NextPosition, 0.0f), CurrentWidgetIndex, CurrentShader);
                    if (FillRight)
                        NextPosition += Horiz->Children[z]->Extent.x + Padding;
                    else
                        NextPosition -= Horiz->Children[z]->Extent.x + Padding;
                }
                break;
            }
            case VerticalBox:
            {
                WidgetCount++;
                widget_component_verticalbox* Vert = (widget_component_verticalbox*)Array[x];
                f32 Padding = Vert->Padding;
                bool FillDown = Vert->FillDownward;
                f32 NextPosition = 0.0f;
                for (u32 z = 0; z < Vert->Children.Num(); z++)
                {
                    WidgetCount++;
                    RenderWidgetComponent(Vert->Children[z], WidgetRegistry[CurrentWidgetIndex]->Position + Vert->Position + V2(0.f, NextPosition), CurrentWidgetIndex, CurrentShader);
                    if (FillDown)
                        NextPosition -= Vert->Children[z]->Extent.y + Padding;
                    else
                        NextPosition += Vert->Children[z]->Extent.y + Padding;
                }
                break;
            }
            case WrapBox:
            {

                break;
            }
        }
    }
}

void WidgetCollectionAndRender()
{
    int CurrentShader = -1;
    WidgetCount = 0;

    for (u32 i = 0; i < WidgetRegistry.Num(); i++)
    {
        WidgetComponentArrayIteration(WidgetRegistry[i]->RenderingComponents, i, &CurrentShader);
    }
}

// void PickTrace()
// { 
//     DirectX::XMMATRIX Inv = DirectX::XMMatrixInverse(NULL, M4ToDXM(MainCamera->ViewMatrix));
//     DirectX::XMVECTOR ViewSpaceMouse = DirectX::XMVectorSet(UserInputs->MousePosX, UserInputs->MousePosY, 1.f, 0.f);
//     DirectX::XMVECTOR StartPos = DirectX::XMVectorSet(0.f, 0.f, 0.f, 0.f);

//     DirectX::XMVECTOR RayPos = DirectX::XMVector3TransformCoord(StartPos, Inv);
//     DirectX::XMVECTOR RayDir = DirectX::XMVector3TransformNormal(ViewSpaceMouse, Inv);

//     for (u32 i = 0; i < MainLevel->ActorRegistry.Num(); i++)
//     {
//         if (MainLevel->ActorRegistry[i].Visible && MainLevel->ActorRegistry[i].Flag != actor_flag::PendingDestroy)
//         {
//             for (u32 z = 0; z < MainLevel->ActorRegistry[i].RenderingComponents.Num(); z++)
//             {
//                 if (MainLevel->ActorRegistry[i].RenderingComponents[z].Visible && MainLevel->ActorRegistry[i].RenderingComponents[z].RenderResources.VertexStep == 2)
//                 {
//                     for (u32 x = 0; x < MainLevel->ActorRegistry[i].RenderingComponents[z].RenderResources.Vertices.Num() - 2; i += MainLevel->ActorRegistry[i].RenderingComponents[z].RenderResources.VertexStep)
//                     {
//                         vertex V1 = MainLevel->ActorRegistry[i].RenderingComponents[z].RenderResources.Vertices[i];
//                         vertex V2 = MainLevel->ActorRegistry[i].RenderingComponents[z].RenderResources.Vertices[i + 1];
//                         DirectX::XMVECTOR P1 = DirectX::XMVectorSet(V1.x, V1.y, V1.z, 1);
//                         DirectX::XMVECTOR P2 = DirectX::XMVectorSet(V2.x, V2.y, V2.z, 1);
//                         if (RayIntersectsPlane(P1, P2, RayPos, RayDir))
//                         {
//                             int zz = 0;
//                         }
//                     }
//                 }
//             }
//         }
//     }

//     // v3 FinalPos = V3(UserInputs->MousePosX * m._11 + UserInputs->MousePosY * m._21 + 1 * m._31,
//     //                  UserInputs->MousePosY * m._12 + UserInputs->MousePosY * m._22 + 1 * m._32,
//     //                  UserInputs->MousePosX * m._13 + UserInputs->MousePosY * m._23 + 1 * m._33);
//     // MainLevel->ActorRegistry[3].WorldLocation = FinalPos;
// }

LRESULT CALLBACK WindowProc(HWND Window, UINT Message, WPARAM wParam, LPARAM lParam)
{
    LRESULT Result = NULL;
    switch (Message)
    {
        case WM_DESTROY:
        {
            IsRunning = false;
        }
        case WM_CLOSE:
        {
            IsRunning = false;
        }
        case WM_SETCURSOR:
        {
            HCURSOR hCursor = LoadCursor(NULL, IDC_ARROW);
            SetCursor(hCursor);
            break;
        }
        default:
        {
            Result = DefWindowProcA(Window, Message, wParam, lParam);
            break;
        }
    }

    return Result;
}

int WINAPI wWinMain(HINSTANCE Instance, HINSTANCE PrevInstance, PWSTR CommandLine, int ShowFlag) // entrypoint
{
    // TODO: Need to allow the user to choose their resolution.
    int BufferResolutionWidth = 1920;
    int BufferResolutionHeight = 1080;
    UserScreenSizeX = BufferResolutionWidth;
    UserScreenSizeY = BufferResolutionHeight;

    // Calculate the required size of the window rectangle based on the desired client-rectangle size
    RECT rc = {0, 0, BufferResolutionWidth, BufferResolutionHeight};
    AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
    u32 WindowHeight = rc.bottom - rc.top;
    u32 WindowWidth = rc.right - rc.left;

    WNDCLASSA WindowClass = {0};
    WindowClass.style = CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = WindowProc;
    WindowClass.hInstance = Instance;
    WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
    WindowClass.lpszClassName = "SpadeWindowClass";

    ScanAndGeneratePac(false);

	if(RegisterClassA(&WindowClass))
    {
		Window =
            CreateWindowExA(
                0,
                WindowClass.lpszClassName,
                "Spade",
                (WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME) | WS_VISIBLE,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                WindowWidth,  // TODO: Need to factor in title bar offset
                WindowHeight, 
                0,
                0,
                Instance,
                0);

        if (Window == NULL)
        {
            return 1;
        }

        //
        // Initialization
        //
        //MainManager->Initialize();
        InitDX(Window, BufferResolutionHeight, BufferResolutionWidth);
        InitializeAssets();

        //
        // Enable High-Definition Mouse Movement
        //
        #ifndef HID_USAGE_PAGE_GENERIC
        #define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
        #endif
        #ifndef HID_USAGE_GENERIC_MOUSE
        #define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
        #endif

        RAWINPUTDEVICE Rid[1];
        Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC; 
        Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE; 
        Rid[0].dwFlags = RIDEV_NOLEGACY; // RIDEV_INPUTSINK  
        Rid[0].hwndTarget = Window;
        if (RegisterRawInputDevices(Rid, 1, sizeof(Rid[0])) == FALSE)
            Assert(1==2);
        
        SetCapture(Window);
        HCURSOR hCursor = LoadCursor(NULL, IDC_ARROW);
        SetCursor(hCursor);

        MainCamera = new camera(UserScreenSizeX, UserScreenSizeY);
        #if SPADE_3D
        MainCamera->ViewMode = projection_type::PERSPECTIVE;
        #else
        MainCamera->ViewMode = projection_type::PERSPECTIVE;
        MainCamera->AllowMouseMovement = false;
        #endif

        for (u32 i = 0; i < GlobalTextureRegistry.Num(); i++)
        {
            material Mat;
            Mat.bUsesTextureDiffuse = true;
            Mat.DiffuseTexture = &GlobalTextureRegistry[i];
            GlobalMaterialRegistry.Add(Mat);
        }

        for (u32 i = 0; i < 1; i++)
        {
            WidgetRegistry.Add(new user_widget());
            s32 Index = 0;
            //WidgetRegistry[0]->Position = V2(0, 0);
            Index = WidgetRegistry[0]->AddChildComponent(widget_type::Text);
            ((widget_component_text*)WidgetRegistry[0]->RenderingComponents[Index])->Color = colors::Red;
            WidgetRegistry[0]->RenderingComponents[Index]->Position = V2(20, -50);
            WidgetRegistry[0]->RenderingComponents[Index]->Tag = "Text0";

            Index = WidgetRegistry[0]->AddChildComponent(widget_type::Text);
            WidgetRegistry[0]->RenderingComponents[Index]->Position = V2(20, -100);
            WidgetRegistry[0]->RenderingComponents[Index]->Tag = "Text1";
            
            Index = WidgetRegistry[0]->AddChildComponent(widget_type::Text);
            WidgetRegistry[0]->RenderingComponents[Index]->Position = V2(20, -150);
            WidgetRegistry[0]->RenderingComponents[Index]->Tag = "Text2";

            Index = WidgetRegistry[0]->AddChildComponent(widget_type::Text);
            WidgetRegistry[0]->RenderingComponents[Index]->Position = V2(20, -200);
            WidgetRegistry[0]->RenderingComponents[Index]->Tag = "Text3";

            Index = WidgetRegistry[0]->AddChildComponent(widget_type::Text);
            WidgetRegistry[0]->RenderingComponents[Index]->Position = V2(20, -250);
            WidgetRegistry[0]->RenderingComponents[Index]->Tag = "Text4";

            Index = WidgetRegistry[0]->AddChildComponent(widget_type::Text);
            ((widget_component_text*)WidgetRegistry[0]->RenderingComponents[Index])->Color = colors::Blue;
            WidgetRegistry[0]->RenderingComponents[Index]->Position = V2(20, -300);
            WidgetRegistry[0]->RenderingComponents[Index]->Tag = "Text5";

            Index = WidgetRegistry[0]->AddChildComponent(widget_type::Text);
            ((widget_component_text*)WidgetRegistry[0]->RenderingComponents[Index])->Color = colors::Blue;
            WidgetRegistry[0]->RenderingComponents[Index]->Position = V2(20, -350);
            WidgetRegistry[0]->RenderingComponents[Index]->Tag = "Text6";

            Index = WidgetRegistry[0]->AddChildComponent(widget_type::Border);
            WidgetRegistry[0]->RenderingComponents[Index]->Position = V2(15, -360);
            WidgetRegistry[0]->RenderingComponents[Index]->Extent = V2(500, 345);
            ((widget_component_border*)WidgetRegistry[0]->RenderingComponents[Index])->UpdateBorderImage("bkg.jpg");
            ((widget_component_border*)WidgetRegistry[0]->RenderingComponents[Index])->UseImageBackground = true;
            WidgetRegistry[0]->RenderingComponents[Index]->Tag = "Border0";
            WidgetRegistry[0]->RenderingComponents[Index]->UpdateZOrder(-10);

            //WidgetRegistry[0]->AddChildComponent(widget_type::HorizontalBox);
            // Index = WidgetRegistry[0]->AddChildComponent(WidgetRegistry[0]);
            // WidgetRegistry[0]->RenderingComponents[Index]->Position = V2(200, 0);
            // WidgetRegistry[0]->AddChildComponent(WidgetRegistry[0]);
            // WidgetRegistry[0]->AddChildComponent(WidgetRegistry[0]);
            // WidgetRegistry[0]->AddChildComponent(WidgetRegistry[0]);
            // WidgetRegistry[0]->AddChildComponent(WidgetRegistry[0]);
            // WidgetRegistry[0]->AddChildComponent(WidgetRegistry[0]);
            // for (u32 i = 0; i < 280; i++)
            // {
            //     WidgetRegistry[0]->CanvasWidgets[8]->AddChildComponent(WidgetRegistry[0]->CanvasWidgets[6]);
            //     WidgetRegistry[0]->CanvasWidgets[8]->AddChildComponent(WidgetRegistry[0]->CanvasWidgets[6]);
            //     WidgetRegistry[0]->CanvasWidgets[8]->AddChildComponent(WidgetRegistry[0]->CanvasWidgets[6]);
            // }
            
        }

        MainLevel->Initialize();

        LARGE_INTEGER frequency;
        LARGE_INTEGER t1,t2,t3;
        double elapsedTime = 0;
        QueryPerformanceFrequency(&frequency);
        u32 frame = 0;

        while (IsRunning) // game loop
        {
            frame++;
            QueryPerformanceCounter(&t1);

            ProcessPendingMessages();

            ProcessUserInputs();

            MainCamera->SetViewMatrix(projection_type::PERSPECTIVE); // works in both types of viewing modes

            //PickTrace();

            QueryPerformanceCounter(&t3);
            MainLevel->TickPhysics(UserInputs->DeltaTime);
            QueryPerformanceCounter(&t2);
            char String7[50];
            _snprintf_s(String7, sizeof(String7), "Physics pass: %f ms", ((double)(t2.QuadPart-t3.QuadPart)/frequency.QuadPart) * 1000);
            ((widget_component_text*)WidgetRegistry[0]->GetComponentByTag("Text0"))->Text = String7;

            #if SPADE_3D
            LightAngle -= 0.01f * UserInputs->DeltaTime;
            if (LightAngle < 90.f)
                LightAngle = 270;

            QueryPerformanceCounter(&t3);
            LightIteration(true, 1, 2048); // add dynamic cascade number (locked at 3)
            QueryPerformanceCounter(&t2);
            char String[50];
            _snprintf_s(String, sizeof(String), "Lighting pass: %.3g ms", ((double)(t2.QuadPart-t3.QuadPart)/frequency.QuadPart) * 1000);
            ((widget_component_text*)WidgetRegistry[0]->GetComponentByTag("Text1"))->Text = String;
            #else
            ((widget_component_text*)WidgetRegistry[0]->GetComponentByTag("Text1"))->Text = "Lighting pass: N/A";
            #endif

            RECT rc;
            GetClientRect( Window, &rc );
            DXSetViewport((FLOAT)(rc.right - rc.left), (FLOAT)(rc.bottom - rc.top));

            QueryPerformanceCounter(&t3);
            // reset pipeline for main rendering pass (move into SPADE3D?)
            DeviceContext->OMSetRenderTargets( 1, &RenderTargetView, DepthStencilView );
            DeviceContext->ClearRenderTargetView(RenderTargetView, ClearColor);
            DeviceContext->ClearDepthStencilView(DepthStencilView,  D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0 );
            DeviceContext->PSSetShaderResources(1, 1, &ShadowTex);
            DeviceContext->PSSetShaderResources(2, 1, &ParaboloidTex);
            DeviceContext->VSSetShader(MainVertexShader, NULL, 0);
            DeviceContext->PSSetShader(MainPixelShader, NULL, 0);
            DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer);

            constants.ViewProj = MainCamera->GetProjectionMatrix() * MainCamera->ViewMatrix; // initialize VP before world is initialized per actor

            // Input events in actors?
            ActorIteration();
            QueryPerformanceCounter(&t2);
            char String3[50];
            _snprintf_s(String3, sizeof(String3), "Main pass: %.3g ms", ((double)(t2.QuadPart-t3.QuadPart)/frequency.QuadPart) * 1000);
            ((widget_component_text*)WidgetRegistry[0]->GetComponentByTag("Text2"))->Text = String3;

            // set pipeline for orthographic widget rendering
            MainCamera->SetViewMatrix(projection_type::ORTHOGRAPHIC);
            DeviceContext->OMSetDepthStencilState(DepthStencilDisabled, 1);
            constants.ViewProj = MainCamera->OrthographicProjection * MainCamera->ViewMatrix; // initialize VP before world is initialized per actor

            char String2[50];
            _snprintf_s(String2, sizeof(String2), "FPS: %.3f (%f ms)", (1/(UserInputs->DeltaTime / 1000)), UserInputs->DeltaTime);
            ((widget_component_text*)WidgetRegistry[0]->GetComponentByTag("Text3"))->Text = String2;

            char String5[50];
            _snprintf_s(String5, sizeof(String5), "Actor count: %d", MainLevel->ActorRegistry.Num());
            ((widget_component_text*)WidgetRegistry[0]->GetComponentByTag("Text4"))->Text = String5;

            char String6[50];
            _snprintf_s(String6, sizeof(String6), "Widget comp count: %d", WidgetCount);
            ((widget_component_text*)WidgetRegistry[0]->GetComponentByTag("Text5"))->Text = String6;

            QueryPerformanceCounter(&t3);
            WidgetCollectionAndRender();
            QueryPerformanceCounter(&t2);
            char String4[50];
            _snprintf_s(String4, sizeof(String4), "Widget pass: %.3g ms", ((double)(t2.QuadPart-t3.QuadPart)/frequency.QuadPart) * 1000);
            ((widget_component_text*)WidgetRegistry[0]->GetComponentByTag("Text6"))->Text = String4;

            #if SPADE_DEBUG == 1
            RenderEditor();
            #endif

            Chain->Present(0, 0); // vsync = 0

            QueryPerformanceCounter(&t2);
            elapsedTime = (double)(t2.QuadPart-t1.QuadPart)/frequency.QuadPart;
            UserInputs->DeltaTime = (f32)elapsedTime * 1000; // convert elapsed time to ms
            //OutputDebugDouble(1/(elapsedTime)); // fps

            MainManager->ResetBlock(memory_lifetime::Frame); // reset per frame memory and unbind shaders
            ID3D11ShaderResourceView *const pSRV[4] = { NULL, NULL, NULL, NULL };
            DeviceContext->PSSetShaderResources(1, 4, pSRV);
            DeviceContext->OMSetDepthStencilState(DepthStencilEnabled, 1);
            // shader is 2dfontshader at end of frame
        }
    }
    //ShowWindow(hwnd, nCmdShow); // call after stuff is ready

    DXCleanup();

    OutputDebugMessage("Program end");
    return 0;
}

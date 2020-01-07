#pragma once

#define SPADE_DEBUG 1
#define PLATFORM_WINDOWS 1
#define SDK_DIRECTX11 1

#include <string.h>
#include <random>
#include <fstream>
#include <oleidl.h>
#include <new>
#include <stdio.h>
#include <array>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "../3rdparty/stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../3rdparty/stb/stb_image_write.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "../3rdparty/stb/stb_truetype.h"

#include "../3rdparty/imgui/imgui.h"
#include "../3rdparty/imgui/imgui_internal.h"

#if PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <SDKDDKVer.h>
#include <windows.h>
#include "../3rdparty/imgui/imgui_impl_win32.h"
#endif

#if SDK_DIRECTX11
#pragma comment(lib, "d3d11.lib") 
#include <DXGI1_6.h>
#include <D3DCommon.h>
#include <d3d11.h>
#include <DirectXMath.h>   
#include <D3Dcompiler.h>
#include "../3rdparty/imgui/imgui_impl_dx11.h"
#define ASSET_DIRECTX11
#endif

#include "../3rdparty/AssetSystem/assetLoader.h"


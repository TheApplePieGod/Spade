#include "Platform.h"
#include "pch.h"

// Platform api
#if PLATFORM_WINDOWS
#include "../windows/WindowsMain.cpp"
#endif

// Rendering
#if SDK_DIRECTX11
#include "rendering/DX11Renderer.cpp"
#endif
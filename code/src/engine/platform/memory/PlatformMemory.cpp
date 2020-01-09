#include "pch.h"
#include "Basic.h"
#include "PlatformMemory.h"

// Platform api
#if PLATFORM_WINDOWS
#include "WindowsMemory.cpp"
#endif

u8* memory::Allocate(u64 Size)
{
	return platform_memory::Allocate(Size);
}
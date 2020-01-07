#include "pch.h"
#include "Basic.h"
#include "PlatformMemory.h"

// Platform api
#if PLATFORM_WINDOWS
#include "WindowsMemory.cpp"
#endif

platform_memory* PlatformMemory;

u8* memory::Allocate(u64 Size)
{
	return PlatformMemory->Allocate(Size);
}
#include "WindowsMemory.h"

u8* windows_memory::Allocate(u64 Size)
{
	return (u8*)VirtualAlloc(NULL, Size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}
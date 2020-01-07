#pragma once

class windows_memory
{
public:
	static u8* Allocate(u64 Size);
};

typedef windows_memory platform_memory;
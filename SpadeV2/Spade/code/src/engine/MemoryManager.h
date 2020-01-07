#pragma once
#include "platform/memory/PlatformMemory.h"

struct memory_block_data
{
    size_t AllocatedSize;
    size_t AmountUsed = 0;
};

struct memory_node
{
    u8* PrevFreeNode = nullptr;
    u8* NextFreeNode = nullptr;
    void* NodeData; // allocated size - 2 byte
};

struct memory_block
{
    bool UseList;
    u8* BaseNodeAddress;
    memory_block_data BlockData;
    size_t MemoryAlignment = 4;

    void* Allocate(size_t Amount);
    void Initialize(bool UseListMethod, size_t BlockSize, size_t NodeSize = 0);
};

enum memory_lifetime
{
    Frame,
    Permanent
};

class memory_manager
{

public:

    memory_manager(bool AutoInitialize = false);

    void Initialize();

    memory_block_data GetBlockData(memory_lifetime Lifetime);

    void* Allocate(size_t Amount, memory_lifetime Lifetime);

    void ResetBlock(memory_lifetime Lifetime);

private:

    memory_block PermanentMemoryBlock;
    memory_block FrameMemoryBlock;
    memory_block DynamicMemoryBlock;

};

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
	u32 DataSize = 0;
    void* NodeData = nullptr; // allocated size - 6 byte
};

struct memory_block
{
    bool UseList;
    u8* BaseNodeAddress;
    memory_block_data BlockData;
    size_t MemoryAlignment = 4;

    void* Allocate(size_t Amount);
    void Initialize(bool DynamicList, size_t BlockSize);

	//template <typename T>
	//std::shared_ptr<T> AllocateInPool(size_t Amount);
};

enum class memory_block_type
{
    Frame,
    Permanent,
	Actors,
	Components,
	// Dynamic
};

class memory_manager
{

public:

    memory_manager(bool AutoInitialize = false);

    void Initialize();

    memory_block_data GetBlockData(memory_block_type Lifetime);

    void* Allocate(size_t Amount, memory_block_type Lifetime);

    void ResetBlock(memory_block_type Lifetime);

private:

    memory_block PermanentMemoryBlock;
    memory_block FrameMemoryBlock;
	memory_block ActorMemoryBlock;
	memory_block ComponentMemoryBlock;
    //memory_block DynamicMemoryBlock;

};

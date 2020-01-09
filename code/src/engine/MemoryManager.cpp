#include "pch.h"
#include "Basic.h"
#include "MemoryManager.h"

void memory_manager::Initialize()
{
    // Permanent memory : 500 MB
    // void* AllocatedMemory = (void*)VirtualAlloc(NULL, Megabytes(500), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    // PermanentMemoryBlock.BlockData.AllocatedSize = Megabytes(500);
    // PermanentMemoryBlock.BaseNodeAddress = (u8*)AllocatedMemory;
    PermanentMemoryBlock.Initialize(false, Megabytes(500));

    // Frame memory : 250 MB
    // AllocatedMemory = (void*)VirtualAlloc(NULL, Megabytes(500), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    // FrameMemoryBlock.BlockData.AllocatedSize = Megabytes(500);
    // FrameMemoryBlock.BaseNodeAddress = (u8*)AllocatedMemory;
    FrameMemoryBlock.Initialize(false, Megabytes(250));

    //DynamicMemoryBlock.Initialize(true, Megabytes(64), Kilobytes(16));
}

memory_manager::memory_manager(bool AutoInitialize)
{
    if (AutoInitialize)
    {
        Initialize();
    }
}

memory_block_data memory_manager::GetBlockData(memory_lifetime Lifetime)
{
    switch(Lifetime)
    {
        case 0: // frame
        {
            return FrameMemoryBlock.BlockData;
        }
        case 1: // permanent
        {
            return PermanentMemoryBlock.BlockData;
        }
    }
    return memory_block_data(); // unreachable
}

inline void ResetBlock(memory_block* Block)
{
    Block->BlockData.AmountUsed = 0;
}

inline size_t GetAlignmentOffset(memory_block* MemBlock, size_t Alignment)
{
    //
    // The way this works is that it takes a memory alignment and then
    // looks up the next available position in the passed-in Arena.
    // A mask is created, which is Alignment - 1. If we want 16-bit aligned, this is 15.
    // We then bitwise AND the Arena start position in question with the mask.
    // If this is anything but zero, which means that it is aligned, then
    // we know that the value that resulted from the AND can be subtracted from 
    // our requested alignment in order to come up with the offset that will get us
    // to the next position that is on the requested alignment border.
    //
    size_t AlignmentOffset = 0;
    
    size_t ResultPointer = (size_t)MemBlock->BaseNodeAddress + MemBlock->BlockData.AmountUsed;
    size_t AlignmentMask = Alignment - 1;
    if(ResultPointer & AlignmentMask)
    {
        AlignmentOffset = Alignment - (ResultPointer & AlignmentMask);
    }

    return(AlignmentOffset);
}

void* memory_block::Allocate(size_t Amount)
{
    Assert(BlockData.AmountUsed + Amount <= BlockData.AllocatedSize);
    
    size_t SizeToAlloc = Amount;
    size_t AlignmentOffset = GetAlignmentOffset(this, MemoryAlignment);

    SizeToAlloc += AlignmentOffset;
    void* AllocatedLocation = BaseNodeAddress + BlockData.AmountUsed + AlignmentOffset;

    BlockData.AmountUsed += SizeToAlloc;

    return AllocatedLocation;
}

void memory_block::Initialize(bool UseListMethod, size_t BlockSize, size_t NodeSize)
{
	BaseNodeAddress = memory::Allocate(BlockSize);
    BlockData.AllocatedSize = BlockSize;
    UseList = UseListMethod;

    if (UseList)
    {
        // init list
        for (u32 i = 0; i < (u32)(BlockSize / NodeSize); i++)
        {
            memory_node* NodeLocation = (memory_node*)(BaseNodeAddress + (i * NodeSize));
            NodeLocation->NextFreeNode = (u8*)(NodeLocation + BlockSize);

            if (i != 0)
                NodeLocation->PrevFreeNode = (u8*)(NodeLocation - BlockSize);
        }
    }
}

void* memory_manager::Allocate(size_t Amount, memory_lifetime Lifetime)
{
    switch(Lifetime)
    {
        case 0: // frame
        {
            return FrameMemoryBlock.Allocate(Amount);
        }
        case 1: // permanent
        {
            return PermanentMemoryBlock.Allocate(Amount);
        }
    }
    return 0; // unreachable
}

void memory_manager::ResetBlock(memory_lifetime Lifetime)
{
    switch(Lifetime)
    {
        case 0: // frame
        {
            FrameMemoryBlock.BlockData.AmountUsed = 0;
            return;
        }
        case 1: // permanent
        {
            PermanentMemoryBlock.BlockData.AmountUsed = 0;
            return;
        }
    }
}
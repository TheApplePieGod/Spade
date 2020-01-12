#include "pch.h"
#include "Basic.h"
#include "MemoryManager.h"

void memory_manager::Initialize()
{
    PermanentMemoryBlock.Initialize(false, Megabytes(250));

    FrameMemoryBlock.Initialize(false, Megabytes(250));

	//ActorMemoryBlock.Initialize(true, Megabytes(250));

	//ComponentMemoryBlock.Initialize(true, Megabytes(250));

    //DynamicMemoryBlock.Initialize(true, Megabytes(128), Kilobytes(16));
}

memory_manager::memory_manager(bool AutoInitialize)
{
    if (AutoInitialize)
    {
        Initialize();
    }
}

memory_block_data memory_manager::GetBlockData(memory_block_type Type)
{
    switch(Type)
    {
        case memory_block_type::Frame: // frame
        {
            return FrameMemoryBlock.BlockData;
        }
        case memory_block_type::Permanent: // permanent
        {
            return PermanentMemoryBlock.BlockData;
        }
		case memory_block_type::Actors: // permanent
		{
			return ActorMemoryBlock.BlockData;
		}
		case memory_block_type::Components: // permanent
		{
			return ComponentMemoryBlock.BlockData;
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

	void* AllocatedLocation = nullptr;

	if (UseList)
	{
		memory_node* NodeLocation = (memory_node*)BaseNodeAddress;
		memory_node* PrevNode = NodeLocation;

		bool Found = false;
		while (NodeLocation->NextFreeNode != nullptr)
		{
			PrevNode = NodeLocation;
			NodeLocation = (memory_node*)NodeLocation->NextFreeNode;
			if (NodeLocation->DataSize >= Amount)
			{
				Found = true;
				break;
			}
		}
			
		if (Found)
		{
			((memory_node*)(NodeLocation->NextFreeNode))->PrevFreeNode = (u8*)NodeLocation;
			((memory_node*)(NodeLocation->PrevFreeNode))->NextFreeNode = (u8*)NodeLocation;
			AllocatedLocation = NodeLocation->NodeData;
		}
		else
		{
			memory_node* NewNode = (memory_node*)(NodeLocation + BlockData.AmountUsed + AlignmentOffset);
			*NewNode = memory_node();
			NewNode->DataSize = (u32)Amount;

			AllocatedLocation = &NewNode->NodeData;
			BlockData.AmountUsed += sizeof(memory_node);
		}
	}
	else
		AllocatedLocation = BaseNodeAddress + BlockData.AmountUsed + AlignmentOffset;

    BlockData.AmountUsed += SizeToAlloc;
    return AllocatedLocation;
}

void memory_block::Initialize(bool DynamicList, size_t BlockSize)
{
	BaseNodeAddress = memory::Allocate(BlockSize);
    BlockData.AllocatedSize = BlockSize;
    UseList = DynamicList;

    if (UseList)
    {
		memory_node* NodeLocation = (memory_node*)(BaseNodeAddress);
		*NodeLocation = memory_node();
        // init list
        //for (u32 i = 0; i < (u32)(BlockSize / NodeSize); i++)
        //{
        //    memory_node* NodeLocation = (memory_node*)(BaseNodeAddress + (i * NodeSize));
        //    NodeLocation->NextFreeNode = (u8*)(NodeLocation + BlockSize);

        //    if (i != 0)
        //        NodeLocation->PrevFreeNode = (u8*)(NodeLocation - BlockSize);
        //}
    }
}

void* memory_manager::Allocate(size_t Amount, memory_block_type Type)
{
    switch(Type)
    {
		case memory_block_type::Frame: // frame
        {
            return FrameMemoryBlock.Allocate(Amount);
        }
		case memory_block_type::Permanent: // permanent
        {
            return PermanentMemoryBlock.Allocate(Amount);
        }
		case memory_block_type::Actors: // permanent
		{
			return ActorMemoryBlock.Allocate(Amount);
		}
		case memory_block_type::Components: // permanent
		{
			return ComponentMemoryBlock.Allocate(Amount);
		}
    }
    return 0; // unreachable
}

void memory_manager::ResetBlock(memory_block_type Type)
{
    switch(Type)
    {
        case memory_block_type::Frame: // frame
        {
            FrameMemoryBlock.BlockData.AmountUsed = 0;
            return;
        }
        case memory_block_type::Permanent: // permanent
        {
            PermanentMemoryBlock.BlockData.AmountUsed = 0;
            return;
        }
		case memory_block_type::Actors: // permanent
		{
			ActorMemoryBlock.BlockData.AmountUsed = 0;
			return;
		}
		case memory_block_type::Components: // permanent
		{
			ComponentMemoryBlock.BlockData.AmountUsed = 0;
			return;
		}
    }
}
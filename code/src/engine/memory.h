#if !defined(MEMORY_H)

struct memory_block_data
{
    size_t AllocatedSize;
    size_t AmountUsed = 0;
};

struct memory_block
{
    u8* BaseAddress;
    memory_block_data BlockData;
    size_t MemoryAlignment = 4;

    void* Allocate(size_t Amount);
};

enum memory_lifetime
{
    Frame,
    Permanent,
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

};

#define MEMORY_H
#endif
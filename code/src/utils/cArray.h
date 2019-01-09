#if !defined(CARRAY_H)

template<typename T>
struct cArray
{

public:

    /*
    * ManualFree() must be called after when using the default allocator
    */
    cArray()
    {
        Data = (T*)VirtualAlloc(NULL, sizeof(T) * DefaultAllocation, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);//new T[DefaultAllocation]; // in constructor because incorrect allocation size otherwise
        Length = 0;
        AllocatedMem = DefaultAllocation; // test this for all constructors
    }

    cArray(memory_manager* Manager, memory_lifetime Lifetime)
    {
        Data = (T*)Manager->Allocate(sizeof(T) * DefaultAllocation, Lifetime);
        Length = 0;
        AllocatedMem = DefaultAllocation; // test this for all constructors
        ManagerPtr = Manager;
        MemLifetime = Lifetime;
    }

    cArray(memory_block* Block)
    {
        Data = (T*)Block->Allocate(sizeof(T) * DefaultAllocation);
        Length = 0;
        AllocatedMem = DefaultAllocation; // test this for all constructors
        BlockPtr = Block;
    }

    // cArray(std::initializer_list<T> InitList)
    //     : Data(new T[InitList.size()])
    //     , Length(0)
    //     , AllocatedMem((u32)InitList.size())
    // {
    //     for (auto& Item : InitList)
    //     {
    //         Add(Item);
    //     }
    // }

    cArray(T* Ptr, u32 ArrayCount)
        : Data(Ptr)
        , Length(ArrayCount)
        , AllocatedMem(ArrayCount)
    {}

    // ~cArray()
    // {
    //     if (Data != nullptr)
    //     {
    //         VirtualFree(Data, 0, MEM_RELEASE);
    //     }
    // }

    /*
    * Returns index item was added at
    * Set stretch allocation to true for many add iterations
    */
    s32 Add(const T& Item, bool StretchAllocation = false)
    {
        if (Length + 1 > AllocatedMem)
        {
            Resize(Length + (StretchAllocation ? ExtraAllocation*100 : ExtraAllocation)); // allocate more mem than required for performance
        }
    
        Data[Length] = Item;
        Length++;
        return Length - 1;
    }

    /*
    * If index is bigger than the array then it will be added to the end
    */
    void AddAt(u32 Index, const T& Item, bool StretchAllocation = false)
    {
        if (Index > Length - 1)
        {
            Add(Item, StretchAllocation);
        }
        else
        {
            if (Length + 1 > AllocatedMem)
            {
                Resize(Length + (StretchAllocation ? ExtraAllocation*100 : ExtraAllocation)); // allocate more mem than required for performance
            }
        
            // push everything forward
            for (u32 i = Length; i > Index; i--)
            {
                Data[i] = Data[i - 1];
            }

            Data[Index] = Item;
            Length++;
        }
    }

    /*
    * Should not be used with pointer type arrays
    */
    void AddDefaults(u32 Amount, bool StretchAllocation = false)
    {
        T Item;
        for (u32 i = 0; i < Amount; i++)
        {
            if (Length + 1 > AllocatedMem)
            {
                Resize(Length + (StretchAllocation ? ExtraAllocation*100 : ExtraAllocation)); // allocate more mem than required for performance
            }

            Data[Length] = Item;
            Length++;
        }
    }

    void Append(const cArray& OtherArray, bool LargeArray = false) // append another cArray
    {
        if (OtherArray.Num() + Length > AllocatedMem)
        {
            Resize(AllocatedMem + OtherArray.Num() + LargeArray ? ExtraAllocation * 100 : ExtraAllocation);
        }
        for (u32 i = 0; i < OtherArray.Num(); i++)
        {
            Data[Length] = OtherArray[i];
            Length++;
        }
    }

    void Append(const T* OtherArray, u32 ArrayCount, bool LargeArray = false) // append a generic c++ array
    {
        if (ArrayCount + Length > AllocatedMem)
        {
            Resize(AllocatedMem + ArrayCount + LargeArray ? ExtraAllocation * 100 : ExtraAllocation);
        }
        for (u32 i = 0; i < ArrayCount; i++)
        {
            Data[Length] = OtherArray[i];
            Length++;
        }
    }

    // add mem resizing
    /*
    * Item at end of the array is duplicated, but it is ok because it gets overwritten
    */
    void RemoveAt(u32 Index)
    {
        for (u32 i = Index; i < Length - 1; i++)
        {
            Data[i] = Data[i + 1];
        }
        Length--;
    }

    /*
    * Manually free memory when no manager is in use
    */
    void ManualFree()
    {
        if (BlockPtr == nullptr && ManagerPtr == nullptr && Data != nullptr)
        {
            VirtualFree(Data, 0, MEM_RELEASE);
            Freed = true;
        }
    }

    // void Append(const void* OtherArray, u32 ArrayCount, bool LargeArray = false) // RISKY: Append an array from void pointer
    // {
    //     if (ArrayCount + Length > AllocatedMem)
    //     {
    //         Resize(AllocatedMem + ArrayCount + LargeArray ? ExtraAllocation * 100 : ExtraAllocation);
    //     }
    //     for (u32 i = 0; i < ArrayCount; i++)
    //     {
    //         Data[Length] = OtherArray[i];
    //         Length++;
    //     }
    // }

    inline bool Contains(T CheckingItem)
    {
        for (u32 i = 0; i < length; i++)
        {
            if (Data[i] == CheckingItem)
                return true;
        }
        return false;
    }

    inline void Clear() // nedd to add memory resizing
    {
        Length = 0;
    }

    /*
    * Use at own risk
    */
    void SetToCPPArray(const T* OtherArray, u32 ArrayCount) // optimize
    {
        if (ArrayCount > AllocatedMem)
        {
            Resize(AllocatedMem + ArrayCount + ExtraAllocation);
        }

        for (u32 i = 0; i < ArrayCount; i++)
        {
            Data[i] = OtherArray[i]; // COULD RETURN WRONG LENGTH IF ELEMENT IS EMPTY????
            Length++;
        }
    }

    inline T& operator[](u32 Index) const
    {
        if (Index < Length && Index >= 0)
        {
            return Data[Index];
        }
        else
        {
            OutputDebugMessage("Attempted to access invalid index");
            Assert(1==2);
        }

        return Data[Index]; // unreachable
    }

    // inline cArray<T>& operator=(const cArray<T>& OtherArray)
    // {
    //     if (Data != nullptr)
    //         ManualFree();

    //     this = OtherArray;
    // }

    // inline void operator=(T* OtherArray)
    // {
    //     if (Index < Length && Index >= 0)
    //     {
    //         return Data[Index];
    //     }
    //     else
    //     {
    //         OutputDebugMessage("Attempted to access invalid index");
    //         Assert(1==2);
    //     }

    //     return Data[Index]; // unreachable
    // }

    inline T* GetRaw() const
    {
        return Data;
    }

    inline u32 Num() const
    {
        return Length;
    }

private:

    void Resize(u32 NewSize)
    {
        AllocatedMem = NewSize;
        T* NewArray = nullptr;

        if (ManagerPtr != nullptr)
            NewArray = (T*)ManagerPtr->Allocate(sizeof(T) * AllocatedMem, MemLifetime); // stored via manager
        else if (BlockPtr != nullptr)
            NewArray = (T*)BlockPtr->Allocate(sizeof(T) * AllocatedMem); // stored via block
        else
            NewArray = (T*)VirtualAlloc(NULL, sizeof(T) * AllocatedMem, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE); // stored via VirtualAlloc

        for (u32 i = 0; i < Length; i++)
        {
            NewArray[i] = Data[i];
        }

        if (ManagerPtr == nullptr && BlockPtr == nullptr) // TODO: 'free' parts of memory block
            VirtualFree(Data, 0, MEM_RELEASE);

        Data = NewArray;
        Freed = false;
    }

    T* Data = nullptr;
    memory_lifetime MemLifetime;
    memory_manager* ManagerPtr = nullptr;
    memory_block* BlockPtr = nullptr;
    u32 Length, AllocatedMem;
    u32 DefaultAllocation = 32;
    u32 ExtraAllocation = 6;
    bool Freed = false;
};

struct yeet
{
    inline u32 operator[](u32 Index) const
    {
        return 0;
    }
};

#define CARRAY_H
#endif
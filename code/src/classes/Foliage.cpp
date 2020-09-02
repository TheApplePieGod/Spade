#include "pch.h"
#include "Foliage.h"
#include <MathTypes.h>
#include "../engine/platform/rendering/PlatformRenderer.h"
#include "../engine/Engine.h"

extern engine* Engine;

void foliage_manager::Initialize()
{

}

int foliage_manager::AddInstance(transform& Transform, int FoliageTypeID)
{
	Dirty = true;
	foliage_manager::instances_element NewInstance;
	NewInstance.Element.Transform = Transform;
	NewInstance.Element.FoliageTypeID = FoliageTypeID;
	NewInstance.Element.WorldMatrix = renderer::GenerateWorldMatrix(Transform);

	if (Instances.size() == 0)
	{
		Instances.push_back(NewInstance);
		Head = 0;
		return Head;
	}
	else if (Head == -1)
	{
		Instances[0] = NewInstance;
		Head = 0;
		return 0;
	}
	else
	{
		int FreeIndex;
		if (FirstFree == -1)
		{
			FreeIndex = Instances.size();
			Instances.push_back(NewInstance);
		}
		else
		{
			FreeIndex = FirstFree;
			FirstFree = Instances[FreeIndex].Next;
		}

		int Index = Head;
		while (Instances[Index].Next != -1 && Instances[Instances[Index].Next].Element.FoliageTypeID < FoliageTypeID)
		{
			Index = Instances[Index].Next;
		}

		NewInstance.Next = Instances[Index].Next;
		NewInstance.Prev = Index;
		Instances[Index].Next = FreeIndex;
		if (Instances[Index].Next != -1)
			Instances[Instances[Index].Next].Prev = FreeIndex;

		Instances[FreeIndex] = NewInstance;
		return FreeIndex;
	}
}

void foliage_manager::RemoveInstance(int InstanceID)
{
	Dirty = true;
	if (InstanceID == Head)
		Head = Instances[InstanceID].Next;
	if (Instances[InstanceID].Prev != -1)
		Instances[Instances[InstanceID].Prev].Next = Instances[InstanceID].Next;
	if (Instances[InstanceID].Next != -1)
		Instances[Instances[InstanceID].Next].Prev = Instances[InstanceID].Prev;

	Instances[InstanceID].Next = FirstFree;
	FirstFree = InstanceID;
}

void foliage_manager::Render()
{
	int Index = Head;
	while (Instances[Index].Next != -1)
	{
		//Index
	}
}
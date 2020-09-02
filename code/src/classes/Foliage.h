#pragma once
#include "FreeList.h"
#include "MathTypes.h"

struct foliage_type
{
	int MeshAssetID = -1;
	int MaterialID = -1;
	f32 CullDistance = 100.f;
};

struct foliage_instance
{
	transform Transform;
	matrix4x4 WorldMatrix;
	int FoliageTypeID = -1;
};

class foliage_manager
{
public:
	int AddInstance(transform& Transform, int FoliageTypeID);
	void RemoveInstance(int InstanceID);
	void Render();
	void Initialize();

private:
	void SortInstances();
	bool Dirty = false;
	std::vector<foliage_type> FoliageTypes;

	// instances custom free list implementation
	struct instances_element
	{
		foliage_instance Element;
		int Next = -1;
		int Prev = -1;
	};
	std::vector<instances_element> Instances;
	int FirstFree = -1;
	int Head = -1;
};
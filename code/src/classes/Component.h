#pragma once
#include "MathTypes.h"

class component
{
public:

	s32 RegistryID = -1;
	bool Active = true;

	inline bool IsActive()
	{
		return RegistryID != -1 && Active;
	}

	inline bool IsValid()
	{
		return RegistryID != -1;
	}

};

template <class T> //, typename... Args>
class component_registry
{
	// Compile-time check
	static_assert(std::is_base_of<component, T>::value, "Cannot create registry of non-component based class");

public:

	/* ForceNew is useful when adding many components b/c it skips checking for empty spaces */
	inline s32 CreateComponent(T Template, bool ForceNew = false)
	{
		u32 Size = (u32)Registry.size();
		if (!ForceNew)
		{
			// check for open slot
			for (u32 i = 0; i < Size; i++)
			{
				if (Registry[i].RegistryID == -1)
				{
					Registry[i] = Template;
					Registry[i].RegistryID = i;
					return i;
				}
			}
		}

		Registry.push_back(Template);
		Registry[Size].RegistryID = Size;

		return Size;
	}

	/* Pass args for component constructor
	* ForceNew is useful when adding many components b/c it skips checking for empty spaces
	*/
	//inline s32 CreateComponent(Args... args, bool ForceNew = false)
	//{
	//	T Comp = T(args);
	//	return CreateComponent(Comp, ForceNew);
	//}

	inline T& GetComponent(s32 ID)
	{
		Assert(ID != -1 && ID < Registry.size());
		return Registry[ID];
	}

	inline void DeleteComponent(s32 ID)
	{
		Assert(ID != -1 && ID < Registry.size());
		Registry[ID].RegistryID = -1;
	}

	// use for iteration only
	inline std::vector<T>& GetRegistry()
	{
		return Registry;
	}

	inline void ClearRegistry()
	{

	}

private:

	std::vector<T> Registry;
	//s32 ComponentsAdded;

};
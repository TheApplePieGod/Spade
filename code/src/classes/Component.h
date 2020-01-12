#pragma once

class component
{
public:

	s32 ComponentID = -1;
	bool Active = true;
	const char* TypeIdentifier = "";

};

template <class T>
class component_registry
{
	// Compile-time check
	static_assert(std::is_base_of<component, T>::value, "Cannot create registry of non-component based class");

public:

	inline s32 CreateComponent(T Template = T())
	{
		T Comp;
		Comp.ComponentID = ComponentsAdded;
		Registry.push_back(Comp);
		return ComponentsAdded++;
	}

	inline T& GetComponent(s32 ID)
	{
		if (ID != -1)
		{
			for (u32 i = 0; i < Registry.size(); i++)
			{
				if (Registry[i].ComponentID == ID)
					return Registry[i];
			}
			Assert(1 == 2); // not found
		}
		Assert(1 == 2); // cant get id of -1
		return Registry[0];
	}

	inline void DeleteComponent(s32 ID)
	{
		if (ID != -1)
		{
			for (u32 i = 0; i < Registry.size(); i++)
			{
				if (Registry[i].ComponentID == ID)
				{
					Registry.erase(Registry.begin() + i);
					return;
				}
			}
		}
	}

private:

	std::vector<T> Registry;
	s32 ComponentsAdded;

};
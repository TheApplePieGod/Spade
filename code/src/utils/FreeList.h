#pragma once

template <class T>
class free_list
{
public:

	free_list() = default;

	inline int Insert(const T& Element)
	{
		if (FirstFree == -1)
		{
			free_element NewElem = { Element, -1 };
			Data.push_back(NewElem);

			return static_cast<int>(Data.size() - 1);
		}
		else
		{
			int Index = FirstFree;
			Data[Index].Element = Element;
			FirstFree = Data[Index].Next;
			return Index;
		}
	}

	inline void Erase(int Index)
	{
		Data[Index].Next = FirstFree;
		FirstFree = Index;
	}

	inline void Clear()
	{
		Data.clear();
		FirstFree = -1;
	}

	inline int ValidRange()
	{
		return Data.size();
	}

	inline T& operator[](int Index)
	{
		return Data[Index].Element;
	}

	inline const T& operator[](int Index) const
	{
		return Data[Index].Element;
	}

private:
	struct free_element
	{
		T Element;
		int Next;
	};
	std::vector<free_element> Data;
	int FirstFree = -1;
};
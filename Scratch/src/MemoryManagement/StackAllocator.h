#pragma once
#include "MemoryTemplates.h"

typedef unsigned int U32;

typedef unsigned char byte;

class StackAllocator
{

private:
	static const size_t STACKALLOCATORDEFAULTSIZE = 1024;
	byte* m_data = nullptr;
	byte* m_head = nullptr;
	size_t m_size = 0;

public:
	//Stack marker: Represents the current top of the stack. You can only roll back to a marker, not to arbitrary locations within the stack;

	//Constructs a stack allocator with the given total size.
	explicit StackAllocator(size_t size = STACKALLOCATORDEFAULTSIZE)
		: m_size(size)
	{
		m_data = new byte[m_size];
		m_head = m_data;
	}
	
	~StackAllocator()
	{
		if (m_data != m_head)
		{
			//TODO: Add further error handling
			__debugbreak();
		}

		delete m_data;
		m_data = nullptr;
		m_head = nullptr;
	}

	//Prohibit the use of the copy constructors
	StackAllocator(const StackAllocator& other)                = delete;
	StackAllocator(const StackAllocator&& other)               = delete;
	StackAllocator& operator = (const StackAllocator& other)   = delete;
	StackAllocator&& operator = (const StackAllocator&& other) = delete;

	//Allocates a new block of the given size from the stack top using operator new.
	void* allocate(size_t size_bytes, const char* file, int line, size_t alignment = 1)
	{
		//Calculate the appropriate allocation location 
		byte* allocationLocation = (byte*)nextMultiple(alignment, (size_t)m_head);
		byte* newHeadPointer = allocationLocation + size_bytes;
		if (newHeadPointer <= m_data + m_size)
		{
			m_head = newHeadPointer;
			return allocationLocation;
		}
		else
		{
			//TODO: Add advanced error handling
			return nullptr;
		}
	};

	template <typename T, typename... arguments>
	T* allocateObject(size_t amountofObjects, arguments&&... args)
	{
		byte* allocationLocation = (byte*)nextMultiple(alignof(T), (size_t)m_head);
		byte* newHeadPointer = allocationLocation + amountofObjects * sizeof(T);

		if (newHeadPointer <= m_data + m_size)
		{
			T* returnPointer = reinterpret_cast<T*>(allocationLocation);
			m_head = newHeadPointer;
			for (size_t i = 0; i < amountofObjects; i++)
			{
				T* object = new (std::addressof(returnPointer[i])) T(std::forward<arguments>(args)...);

			}
			return returnPointer;
		}
		else
		{
			//TODO: Add advanced error handling
			return nullptr;
		}
	}

	//Returns a marker to the current stack top.
	byte* GetHead();
	
	//Rolls the stack back to a previous marker.
	//void FreeToMarker(Marker marker);

	//Clears the entire stack (rolls the stack back to zero).
	void Clear(void* ptr);
};
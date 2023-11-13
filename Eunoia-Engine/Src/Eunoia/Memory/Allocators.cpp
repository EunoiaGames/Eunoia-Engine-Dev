#include "Allocators.h"
#include <cstdlib>
#include <cstring>

namespace Eunoia {

	LinearAllocator::LinearAllocator(mem_size capacity, void* memoryToAllocate) :
		m_Offset(0),
		m_NumAllocations(0),
		m_Capacity(capacity)
	{
		if (memoryToAllocate)
		{
			m_Memory = (u8*)memoryToAllocate;
			m_FreeMemory = false;
		}
		else
		{
			m_Memory = (u8*)malloc(capacity);
			m_FreeMemory = true;
		}
	}

	LinearAllocator::~LinearAllocator()
	{
		if (m_FreeMemory)
			free(m_Memory);
	}

	void* LinearAllocator::Allocate(mem_size size)
	{
		void* mem = m_Memory + m_Offset;
		m_Offset += size;
		return mem;
	}

	void LinearAllocator::Free(void* memory) {}
	void LinearAllocator::Reset() { m_Offset = 0; m_NumAllocations = 0; }

	mem_size LinearAllocator::GetNumAllocations() const
	{
		return m_NumAllocations;
	}

	StackAllocator::StackAllocator(mem_size capacity, void* memoryToAllocate) :
		m_Capacity(capacity),
		m_Offset(0),
		m_NumAllocations(0)
	{
		if (memoryToAllocate)
		{
			m_Memory = (u8*)memoryToAllocate;
			m_FreeMemory = false;
		}
		else
		{
			m_Memory = (u8*)malloc(capacity);
			m_FreeMemory = true;
		}
	}

	StackAllocator::~StackAllocator()
	{
		if (m_FreeMemory)
			free(m_Memory);
	}

	void* StackAllocator::Allocate(mem_size size)
	{
		u8* mem = m_Memory + m_Offset;
		*(mem_size*)mem = m_Offset;
		mem += sizeof(mem_size);
		m_Offset += size + sizeof(mem_size);
		return mem;
	}

	void StackAllocator::Free(void* memory)
	{
		m_Offset = *((mem_size*)memory - 1);
	}

	void StackAllocator::Reset()
	{
		m_Offset = 0;
	}

	mem_size StackAllocator::GetNumAllocations() const
	{
		return m_NumAllocations;
	}

	PoolAllocator::PoolAllocator(mem_size numElements, mem_size elementSize, void* memoryToAllocate) :
		m_MaxElements(numElements),
		m_ElementSize(elementSize),
		m_NumAllocations(0)
	{
		union {
			PoolElement* as_self;
			unsigned char* as_char;
			void* as_void;
		};


		if (memoryToAllocate)
		{
			m_Next = (PoolElement*)memoryToAllocate;
			m_Memory = m_Next;
			m_FreeMemory = false;
		}
		else
		{
			m_Next = (PoolElement*)malloc((numElements) * elementSize);
			m_Memory = m_Next;
			m_FreeMemory = true;
		}

		as_self = m_Next;

		PoolElement* runner = m_Next;
		for (u32 i = 0; i < m_MaxElements; i++)
		{
			runner->next = as_self;
			runner = as_self;
			as_char += elementSize;
		}

		runner->next = 0;
	}

	PoolAllocator::~PoolAllocator()
	{
		if (m_FreeMemory)
			free(m_Memory);
	}

	mem_size PoolAllocator::GetMaxElements() const
	{
		return m_MaxElements;
	}

	void* PoolAllocator::Allocate(mem_size size)
	{
		PoolAllocator::PoolElement* head = m_Next;
		m_Next = head->next;
		m_NumAllocations++;
		return head;
	}

	void PoolAllocator::Free(void* memory)
	{
		PoolAllocator::PoolElement* head = (PoolAllocator::PoolElement*)memory;
		head->next = m_Next;
		m_Next = head;
	}

	void PoolAllocator::Reset() 
	{
		m_Next = (PoolElement*)m_Memory;
		m_NumAllocations = 0;

		union {
			PoolElement* as_self;
			unsigned char* as_char;
			void* as_void;
		};

		as_self = m_Next;
		PoolElement* runner = m_Next;
		for (u32 i = 0; i < m_MaxElements; i++)
		{
			runner->next = as_self;
			runner = as_self;
			as_char += m_ElementSize;
		}

		runner->next = 0;
	}

	mem_size PoolAllocator::GetNumAllocations() const
	{
		return m_NumAllocations;
	}

	DynamicPoolAllocator::DynamicPoolAllocator(mem_size elementSize, mem_size initialMaxCapacity) :
		m_ElementSize(elementSize)
	{
		m_Allocators.Push(new PoolAllocator(initialMaxCapacity, elementSize, 0));
	}

	DynamicPoolAllocator::~DynamicPoolAllocator()
	{
		for (u32 i = 0; i < m_Allocators.Size(); i++)
			delete m_Allocators[i];
	}

	void* DynamicPoolAllocator::Allocate(u32* allocatorIndex)
	{
		for (u32 i = 0; i < m_Allocators.Size(); i++)
		{
			if (m_Allocators[i]->GetNumAllocations() < m_Allocators[i]->GetMaxElements())
			{
				*allocatorIndex = i;
				return m_Allocators[i]->Allocate();
			}
		}

		PoolAllocator* allocator = new PoolAllocator(m_Allocators[m_Allocators.Size() - 1]->GetMaxElements() * 2, m_ElementSize, 0);
		m_Allocators.Push(allocator);
		*allocatorIndex = m_Allocators.Size() - 1;
		return allocator->Allocate();
	}

	void DynamicPoolAllocator::Free(void* memory, u32 allocatorIndex)
	{
		m_Allocators[allocatorIndex]->Free(memory);
	}
}
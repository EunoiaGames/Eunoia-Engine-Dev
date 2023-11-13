#pragma once

#include "../Common.h"
#include "../DataStructures/List.h"

#define EU_KB(kb) ((kb) * 1024)
#define EU_MB(mb) (EU_KB(mb) * 1024)
#define EU_GB(gb) (EU_MB(gb) * 1024)


namespace Eunoia {

	class EU_API Allocator
	{
	public:
		virtual ~Allocator() {}

		virtual void* Allocate(mem_size size) = 0;
		virtual void Free(void* memory) = 0;
		virtual void Reset() = 0;
		virtual mem_size GetNumAllocations() const = 0;
	};

	class EU_API LinearAllocator : public Allocator
	{
	public:
		LinearAllocator(mem_size capacity, void* memoryToAllocate = 0);
		~LinearAllocator();

		void* Allocate(mem_size size) override;
		void Free(void* memory) override;
		void Reset() override;
		mem_size GetNumAllocations() const override;
	private:
		u8* m_Memory;
		mem_size m_Offset;
		b32 m_FreeMemory;
		mem_size m_NumAllocations;
		mem_size m_Capacity;
	};

	class EU_API StackAllocator : public Allocator
	{
	public:
		StackAllocator(mem_size size, void* memoryToAllocate = 0);
		~StackAllocator();

		void* Allocate(mem_size size) override;
		void Free(void* memory) override;
		void Reset() override;
		mem_size GetNumAllocations() const override;
	private:
		u8* m_Memory;
		mem_size m_Offset;
		mem_size m_Capacity;
		b32 m_FreeMemory;
		mem_size m_NumAllocations;
	};

	class EU_API PoolAllocator : public Allocator
	{
	public:
		PoolAllocator(mem_size numElements, mem_size elementSize, void* memoryToAllocate = 0);
		~PoolAllocator();

		mem_size GetMaxElements() const;

		void* Allocate(mem_size size = 0) override;
		void Free(void* memory) override;
		void Reset() override;
		mem_size GetNumAllocations() const override;
	private:
		struct PoolElement
		{
			PoolElement* next;
		};

		PoolElement* m_Next;
		void* m_Memory;
		mem_size m_MaxElements;
		mem_size m_ElementSize;
		b32 m_FreeMemory;
		mem_size m_NumAllocations;
	};

	class EU_API DynamicPoolAllocator
	{
	public:
		DynamicPoolAllocator(mem_size elementSize, mem_size initialMaxCapacity);
		~DynamicPoolAllocator();

		void* Allocate(u32* allocatorIndex);
		void Free(void* memory, u32 allocatorIndex);
	private:
		List<PoolAllocator*> m_Allocators;
		mem_size m_ElementSize;
	};
}

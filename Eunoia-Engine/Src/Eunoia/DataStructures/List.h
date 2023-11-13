#pragma once

#include "../Common.h"
#include "../Math/GeneralMath.h"
#include <cstring>
#include <initializer_list>

namespace Eunoia {

	struct ListCapacityChange
	{
		r32 multiplyAmount;
		u32 addAmount;
	};

	template<typename T>
	class List
	{
	public:
		List(const ListCapacityChange& capacityChange, u32 initialCapacity = 8, u32 elementCount = 0, T* copyValue = 0) :
			m_Capacity(initialCapacity),
			m_CapacityChange(capacityChange),
			m_ElementCount(elementCount)
		{
			m_Memory = new T[m_Capacity];

			if (copyValue)
				for (u32 i = 0; i < m_Capacity; i++)
					m_Memory[i] = *copyValue;
		}

		List(u32 initialCapacity = 8, u32 elementCount = 0, T* copyValue = 0) :
			m_Capacity(initialCapacity),
			m_ElementCount(elementCount)
		{
			m_CapacityChange.addAmount = 0;
			m_CapacityChange.multiplyAmount = 2;
			m_Memory = new T[m_Capacity];

			if (copyValue)
				for (u32 i = 0; i < m_Capacity; i++)
					m_Memory[i] = *copyValue;
		}

		List(std::initializer_list<T> initList)
		{
			m_CapacityChange.addAmount = 0;
			m_CapacityChange.multiplyAmount = 2;
			m_Capacity = initList.size();
			m_ElementCount = initList.size();

			m_Memory = new T[m_Capacity];
			for (u32 i = 0; i < m_ElementCount; i++)
				//new(m_Memory + i) T(*(initList.begin() + i));
				m_Memory[i] = *(initList.begin() + i);
		}

		List(const List<T>& list)
		{
			m_ElementCount = list.m_ElementCount;
			m_CapacityChange = list.m_CapacityChange;
			m_Capacity = list.m_Capacity;

			m_Memory = new T[m_Capacity];
			for (u32 i = 0; i < m_ElementCount; i++)
				//new(m_Memory + i) T(*(list.m_Memory + i));
				m_Memory[i] = list.m_Memory[i];
		}

		~List()
		{
			if (m_Capacity > 0 && m_Memory)
				delete[] m_Memory;
		}

		void Clear()
		{
			m_ElementCount = 0;
		}

		b32 Empty() const
		{
			return m_ElementCount == 0;
		}

		u32 Size() const
		{
			return m_ElementCount;
		}

		void Push(const T& element)
		{
			if (m_ElementCount >= m_Capacity)
				Grow();

			//new(m_Memory + m_ElementCount) T(element);
			m_Memory[m_ElementCount] = element;
			m_ElementCount++;
		}

		void Insert(const T& element, u32 index)
		{
			if (m_ElementCount >= m_Capacity)
				Grow();

			m_ElementCount++;
			for (u32 i = index; i < m_ElementCount; i++)
			{
				//new(m_Memory[i + 1]) T(m_Memory[i]);
				m_Memory[i + 1] = m_Memory[i];
			}

			//new(m_Memory[index]) T(element);
			m_Memory[index] = element;
		}

		T& Pop()
		{
			return *(m_Memory + m_ElementCount--);
		}

		void Remove(u32 index)
		{
			for (u32 i = index; i < m_ElementCount - 1; i++)
			{
				m_Memory[i] = m_Memory[i + 1];
			}
			m_ElementCount--;
		}

		s32 GetIndexOfElement(const T& element, u32 offset = 0)
		{
			for (u32 i = offset; i < m_ElementCount; i++)
				if (m_Memory[i] == element)
					return i;

			return -1;
		}

		void SetCapacityChange(const ListCapacityChange& capacityChange)
		{
			m_CapacityChange = capacityChange;
		}

		void SetCapacity(u32 newCapacity)
		{
			if (newCapacity == m_Capacity)
				return;
			
			T* newMemory = new T[newCapacity];

			for (u32 i = 0; i < EU_MIN(m_Capacity, newCapacity); i++)
				//new(newMemory) T(*(m_Memory + i));
				newMemory[i] = m_Memory[i];
			
			if (newCapacity > m_Capacity)
				for (u32 i = m_Capacity; i < newCapacity; i++)
					//new(newMemory) T();
					newMemory[i] = T();


			delete[] m_Memory;
			m_Memory = newMemory;
			m_Capacity = newCapacity;
		}

		void SetCapacityToFitSize()
		{
			SetCapacity(m_ElementCount);
		}

		void SetCapacityAndElementCount(u32 newCapacity)
		{
			m_ElementCount = newCapacity;
			SetCapacity(newCapacity);
		}

		void AddToElementCount(u32 amount)
		{
			m_ElementCount += amount;
		}

		mem_size GetCapacity() const
		{
			return m_Capacity;
		}

		T& GetLastElement()
		{
			return *(m_Memory + (m_ElementCount - 1));
		}

		List<T>& operator=(const List<T>& list)
		{
			//if (m_Capacity > 0 && m_Memory)
				delete[] m_Memory;

			m_ElementCount = list.m_ElementCount;
			m_CapacityChange = list.m_CapacityChange;
			m_Capacity = list.m_Capacity;

			m_Memory = new T[m_Capacity];
			for (u32 i = 0; i < m_ElementCount; i++)
				//new(m_Memory + i) T(*(list.m_Memory + i));
				m_Memory[i] = list[i];

			return *this;
		}

		List<T>& operator=(std::initializer_list<T> initList)
		{
			if (m_Capacity > 0 && m_Memory)
				delete[] m_Memory;

			m_Memory = new T[m_Capacity];
			m_Capacity = initList.size();
			m_ElementCount = initList.size();
			for (u32 i = 0; i < m_ElementCount; i++)
				//new(m_Memory + i) T(*(initList.begin() + i));
				m_Memory[i] = *(initList.begin() + i);

			return *this;
		}

		T& operator[](u32 index)
		{
			return *(m_Memory + index);
		}

		const T& operator[](u32 index) const
		{
			return *(m_Memory + index);
		}
	private:
		void Grow()
		{
			u32 newCapacity = m_Capacity * m_CapacityChange.multiplyAmount + m_CapacityChange.addAmount;
			T* newMemory = new T[newCapacity];
			for (u32 i = 0; i < m_Capacity; i++)
				//new(newMemory + i) T(*(m_Memory + i));
				newMemory[i] = m_Memory[i];
			
			delete[] m_Memory;
			m_Memory = newMemory;
			m_Capacity = newCapacity;
		}
	private:
		T* m_Memory;
		u32 m_ElementCount;
		u32 m_Capacity;
		ListCapacityChange m_CapacityChange;
	};

}
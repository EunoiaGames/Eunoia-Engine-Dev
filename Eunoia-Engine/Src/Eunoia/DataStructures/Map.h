#pragma once

#include "List.h"

namespace Eunoia {

	template<typename MKey, typename MElem>
	struct MapKeyPair
	{
		MKey key;
		MElem elem;
	};

	template<typename MKey, typename MElem>
	class Map
	{
	public:
		Map()
		{

		}

		~Map()
		{

		}

		void Clear()
		{
			m_KeyPairs.Clear();
		}

		inline u32 Size() { return m_KeyPairs.Size(); }

		b32 FindElement(const MKey& key, MElem* element = 0)
		{
			for (u32 i = 0; i < m_KeyPairs.Size(); i++)
			{
				if (m_KeyPairs[i].key == key)
				{
					if(element)
						*element = m_KeyPairs[i].elem;
					return true;
				}
			}

			return false;
		}

		inline b32 GetKey(const MElem& elem, MKey* key = 0) const
		{
			for (u32 i = 0; i < m_KeyPairs.Size(); i++)
			{
				if (m_KeyPairs[i].elem == elem)
				{
					if (key)
						*key = m_KeyPairs[i].key;
					return true;
				}
			}
			return false;
		}

		inline const MElem& operator[](const MKey& key) const
		{
			for (u32 i = 0; i < m_KeyPairs.Size(); i++)
				if (m_KeyPairs[i].key == key)
					return m_KeyPairs[i].elem;

			//MapKeyPair<MKey, MElem> pair;
			//pair.key = key;
			
			//m_KeyPairs.Push(pair);
			//return m_KeyPairs[m_KeyPairs.Size() - 1].elem;
		}

		inline MElem& operator[](const MKey& key)
		{
			for (u32 i = 0; i < m_KeyPairs.Size(); i++)
				if (m_KeyPairs[i].key == key)
					return m_KeyPairs[i].elem;

			MapKeyPair<MKey, MElem> pair;
			pair.key = key;

			m_KeyPairs.Push(pair);
			return m_KeyPairs[m_KeyPairs.Size() - 1].elem;
		}

		inline const List<MapKeyPair<MKey, MElem>>& GetKeyPairList() const { return m_KeyPairs; }
		inline const MapKeyPair<MKey, MElem>& GetKeyPairAtIndex(u32 index) const { return m_KeyPairs[index]; }
	private:
		List<MapKeyPair<MKey, MElem>> m_KeyPairs;
	};

}
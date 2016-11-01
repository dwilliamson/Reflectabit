
#pragma once


#include <rflb/Container.h>
#include <rflb/Utils.h>
#include <map>


namespace rflb
{
	namespace internal
	{
		template <typename KEY, typename DATA, typename COMPARE, typename ALLOC>
		class MapReadIterator : public IReadIterator
		{
		public:
			typedef std::map<KEY, DATA, COMPARE, ALLOC> Container;
			typedef typename Container::const_iterator Iterator;

			MapReadIterator(const Container* container) :
				m_Container(*container),
				m_Iterator(container->begin())
			{
			}

			const void* GetKey() const
			{
				RFLB_ASSERT(m_Iterator != m_Container.end());
				return &m_Iterator->first;
			}

			const void* GetValue() const
			{
				RFLB_ASSERT(m_Iterator != m_Container.end());
				return &m_Iterator->second;
			}

			int GetCount() const
			{
				return m_Container.size();
			}

			void MoveNext()
			{
				RFLB_ASSERT(m_Iterator != m_Container.end());
				++m_Iterator;
			}

			bool IsValid() const
			{
				return m_Iterator != m_Container.end();
			}

		private:
			const Container& m_Container;
			Iterator m_Iterator;
		};


		template <typename KEY, typename DATA, typename COMPARE, typename ALLOC>
		class MapWriteIterator : public IWriteIterator
		{
		public:
			typedef std::map<KEY, DATA, COMPARE, ALLOC> Container;
			typedef typename Container::const_iterator Iterator;

			MapWriteIterator(Container* container) :
				m_Container(*container)
			{
			}

			void Add(void*)
			{
				RFLB_ASSERT(false);
			}

			void Add(void* key, void* value)
			{
				m_Container[*(KEY*)key] = *(DATA*)value;
			}

			void* AddEmpty()
			{
				RFLB_ASSERT(false);
				return 0;
			}

			void* AddEmpty(void* key)
			{
				//std::pair<Container::iterator, bool> ret = m_Container.insert(std::pair<KEY, DATA>(*(KEY*)key, DATA()));
				//return &*ret.first;
				m_Container[*(KEY*)key] = DATA();
				return &m_Container[*(KEY*)key];
			}

		private:
			Container& m_Container;
		};


		template <typename KEY, typename DATA, typename COMPARE, typename ALLOC>
		IContainerFactory* CreateContainerFactory(std::map<KEY, DATA, COMPARE, ALLOC>&, TypeInfo& key_type, TypeInfo& value_type)
		{
			// Can't deal with keys that are pointers
			RFLB_STATIC_ASSERT(is_pointer<KEY>::val == false);

			key_type = TypeInfo::Create<KEY>();
			value_type = TypeInfo::Create<DATA>();

			return new internal::ContainerFactory<
				std::map<KEY, DATA, COMPARE, ALLOC>,
				MapReadIterator<KEY, DATA, COMPARE, ALLOC>,
				MapWriteIterator<KEY, DATA, COMPARE, ALLOC> >();
		}
	}
}
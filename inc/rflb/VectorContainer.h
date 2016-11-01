
#pragma once


#include <rflb/Container.h>
#include <rflb/Utils.h>
#include <vector>


namespace rflb
{
	namespace internal
	{
		template <typename TYPE, typename ALLOCATOR>
		class VectorReadIterator : public IReadIterator
		{
		public:
			typedef std::vector<TYPE, ALLOCATOR> Container;
			typedef typename Container::const_iterator Iterator;

			VectorReadIterator(const Container* container) :
				m_Container(*container),
				m_Iterator(container->begin())
			{
			}

			const void* GetKey() const
			{
				RFLB_ASSERT(false);
				return 0;
			}

			const void* GetValue() const
			{
				RFLB_ASSERT(m_Iterator != m_Container.end());
				return &(*m_Iterator);
			}

			int GetCount() const
			{
				return (int)m_Container.size();
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


		template <typename TYPE, typename ALLOCATOR>
		class VectorWriteIterator : public IWriteIterator
		{
		public:
			typedef std::vector<TYPE, ALLOCATOR> Container;

			VectorWriteIterator(Container* container) :
				m_Container(*container)
			{
			}

			void Add(void* object)
			{
				m_Container.push_back(*(TYPE*)object);
			}

			void Add(void*, void*)
			{
				RFLB_ASSERT(false);
			}

			void* AddEmpty()
			{
				m_Container.push_back(TYPE());
				return &m_Container.back();
			}

			void* AddEmpty(void*)
			{
				RFLB_ASSERT(false);
				return 0;
			}

		private:
			Container& m_Container;
		};


		template <typename TYPE, typename ALLOCATOR>
		IContainerFactory* CreateContainerFactory(std::vector<TYPE, ALLOCATOR>&, TypeInfo&, TypeInfo& value_type)
		{
			value_type = TypeInfo::Create<TYPE>();

			return new internal::ContainerFactory<
				std::vector<TYPE, ALLOCATOR>,
				VectorReadIterator<TYPE, ALLOCATOR>,
				VectorWriteIterator<TYPE, ALLOCATOR> >();
		}
	}
}
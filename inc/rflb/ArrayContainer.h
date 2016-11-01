
#pragma once


#include <rflb/Container.h>
#include <rflb/Utils.h>


namespace rflb
{
	namespace internal
	{
		template <typename TYPE, int LENGTH>
		class ArrayReadIterator : public IReadIterator
		{
		public:
			ArrayReadIterator(const TYPE* container) :
				m_Container(container),
				m_Position(0)
			{
			}

			const void* GetKey() const
			{
				RFLB_ASSERT(false);
				return 0;
			}

			const void* GetValue() const
			{
				RFLB_ASSERT(m_Position >= 0 && m_Position < LENGTH);
				return m_Container + m_Position;
			}

			int GetCount() const
			{
				return LENGTH;
			}
			
			void MoveNext()
			{
				RFLB_ASSERT(m_Position >= 0 && m_Position < LENGTH);
				m_Position++;
			}

			bool IsValid() const
			{
				return m_Position < LENGTH;
			}

		private:
			const TYPE* m_Container;
			int m_Position;
		};


		template <typename TYPE, int LENGTH>
		class ArrayWriteIterator : public IWriteIterator
		{
		public:
			ArrayWriteIterator(TYPE* container) :
				m_Container(container),
				m_Position(0)
			{
			}

			void Add(void* object)
			{
				RFLB_ASSERT(m_Position >= 0 && m_Position < LENGTH);
				m_Container[m_Position++] = *(TYPE*)object;
			}

			void Add(void*, void*)
			{
				RFLB_ASSERT(false);
			}

			void* AddEmpty()
			{
				RFLB_ASSERT(m_Position < LENGTH);
				m_Container[m_Position] = TYPE();
				return &m_Container[m_Position++];
			}

			void* AddEmpty(void*)
			{
				RFLB_ASSERT(false);
				return 0;
			}

		private:
			TYPE* m_Container;
			int m_Position;
		};


		template <typename TYPE, int LENGTH>
		IContainerFactory* CreateContainerFactory(TYPE (&)[LENGTH], TypeInfo& key_type, TypeInfo& value_type)
		{
			value_type = TypeInfo::Create<TYPE>();

			return new internal::ContainerFactory<
				TYPE,
				ArrayReadIterator<TYPE, LENGTH>,
				ArrayWriteIterator<TYPE, LENGTH> >();
		}
	}
}

#pragma once


namespace rflb
{
	struct TypeInfo;
	class Type;


	// Read-only iteration over a container
	struct IReadIterator
	{
		virtual ~IReadIterator() { }
		virtual const void* GetKey() const = 0;
		virtual const void* GetValue() const = 0;
		virtual int GetCount() const = 0;
		virtual void MoveNext() = 0;
		virtual bool IsValid() const = 0;
	};


	// Write-only access to a container
	struct IWriteIterator
	{
		virtual ~IWriteIterator() { }
		virtual void Add(void* object) = 0;
		virtual void Add(void* key, void* object) = 0;
		virtual void* AddEmpty() = 0;
		virtual void* AddEmpty(void* key) = 0;
	};


	// Anonymous creation of iterators and containers
	struct IContainerFactory
	{
		IContainerFactory() :
			m_KeyType(0),
			m_ValueType(0),
			m_KeyIsPointer(false),
			m_ValueIsPointer(false)
		{
		}

		// Pointers to the key/value types
		Type* m_KeyType;
		Type* m_ValueType;

		// TODO: Does the Type*/bool pair need to be abstracted to a simpler level than Field?
		bool m_KeyIsPointer;
		bool m_ValueIsPointer;

		// Support for finding out how much memory an iterator/container consumes
		// before constructing it, allowing custom memory allocation -- even from the
		// runtime stack.
		virtual int GetReadIteratorSize() const = 0;
		virtual int GetWriteIteratorSize() const = 0;
		virtual IReadIterator* ConstructReadIterator(void* dest, const void* container) const = 0;
		virtual IWriteIterator* ConstructContainer(void* dest, void* container) const = 0;
		virtual void DestructIterator(IReadIterator* iterator) = 0;
		virtual void DestructIterator(IWriteIterator* iterator) = 0;
	};


	namespace internal
	{
		// Generic implementation of a container factory
		template <typename TYPE, typename READ_ITERATOR, typename WRITE_ITERATOR>
		struct ContainerFactory : public IContainerFactory
		{
			int GetReadIteratorSize() const
			{
				return sizeof(READ_ITERATOR);
			}

			int GetWriteIteratorSize() const
			{
				return sizeof(WRITE_ITERATOR);
			}

			IReadIterator* ConstructReadIterator(void* dest, const void* container) const
			{
				return new (dest) READ_ITERATOR((TYPE*)container);
			}

			IWriteIterator* ConstructContainer(void* dest, void* container) const
			{
				return new (dest) WRITE_ITERATOR((TYPE*)container);
			}

			void DestructIterator(IReadIterator* iterator)
			{
				((READ_ITERATOR*)iterator)->READ_ITERATOR::~READ_ITERATOR();
			}

			void DestructIterator(IWriteIterator* iterator)
			{
				((WRITE_ITERATOR*)iterator)->WRITE_ITERATOR::~WRITE_ITERATOR();
			}
		};


		// No container factory is created by default for all field types
		template <typename TYPE> IContainerFactory* CreateContainerFactory(TYPE&, TypeInfo& key_type, TypeInfo& value_type)
		{
			return 0;
		}
	}
}


//
// The following macros create temporary iterators and containers by quickly allocating
// them on the stack and correctly constructing them. The memory for these iterators/containers
// is released automatically when the stack unwinds and can't be returned to calling functions.
// Before that happens, the iterators/containers need to be destructed.
//


// Allocate and construct a temporary iterator on the stack
#define RFLB_NEW_TEMP_READ_ITERATOR(factory, container)	\
	factory->ConstructReadIterator(_alloca(factory->GetReadIteratorSize()), container)

// Allocate and construct a temporary container on the stack
#define RFLB_NEW_TEMP_WRITE_ITERATOR(factory, container)	\
	factory->ConstructContainer(_alloca(factory->GetWriteIteratorSize()), container)

// Destruct a temporary iterator
#define RFLB_DELETE_TEMP_ITERATOR(factory, container) \
	factory->DestructIterator(container)

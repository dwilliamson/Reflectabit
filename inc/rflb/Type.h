
#pragma once


#include <map>
#include <rflb/Utils.h>


namespace rflb
{
	struct FieldInfo;
	struct Field;
	class TypeDatabase;


	// Collection of fields that are optimised for searching
	typedef std::map<u32, Field> Fields;


	namespace internal
	{
		typedef void (*ConstructObjectFunc)(void* object);
		typedef void (*DestructObjectFunc)(void* object);


		// As the constructor/destructor are inaccessible, point to these wrappers for each type
		template <typename TYPE> inline void ConstructObject(void* object)
		{
			new (object) TYPE;
		}
		template <typename TYPE> inline void DestructObject(void* object)
		{
			((TYPE*)object)->TYPE::~TYPE();
		}
	}


	// This is a description of a type, not attached to any type database
	// Although it's not... as it contains the pointer bool
	// This is perhaps mis-named?
	struct TypeInfo
	{
		template <typename TYPE>
		static TypeInfo Create()
		{
			TypeInfo type_info;
			type_info.m_Name = Name(typeid(internal::strip_pointer<TYPE>::Type).name());
			type_info.m_IsPointer = internal::is_pointer<TYPE>::val;
			type_info.m_Size = sizeof(TYPE);
			type_info.m_Constructor = internal::ConstructObject<TYPE>;
			type_info.m_Destructor = internal::DestructObject<TYPE>;
			return type_info;
		}

		TypeInfo() : m_IsPointer(0), m_Size(0)
		{
		}

		bool operator == (const TypeInfo& rhs) const
		{
			return m_Name == rhs.m_Name &&
				   m_IsPointer == rhs.m_IsPointer &&
				   m_Size == rhs.m_Size;
		}

		bool operator != (const TypeInfo& rhs) const
		{
			return !(*this == rhs);
		}

		Name m_Name;
		bool m_IsPointer;
		int m_Size;
		internal::ConstructObjectFunc m_Constructor;
		internal::DestructObjectFunc m_Destructor;
	};


	// This is the persistent type object, stored in the type database
	class Type
	{
	public:
		Type(const TypeInfo& type_info);

		// Asserts on failure to find the field
		const Field& GetField(const Name& name) const;

		// Returns null on failure to find the field
		const Field* FindField(const Name& name) const;

		Type& LoadSaveBinary(SerialiseLoadFunc load, SerialiseSaveFunc save);
		Type& LoadSaveBinaryIFFv(SerialiseLoadFunc load, SerialiseSaveFunc save);
		Type& LoadSaveTextXML(SerialiseLoadFunc load, SerialiseSaveFunc save);

		// TODO: Store database locally so that this can be a templated function?
		Type& Inherits(Type& base);

		void ConstructObject(void* object);
		void DestructObject(void* object);

		int GetSize() const { return m_Size; }
		const Fields& GetFields() const { return m_Fields; }
		const Serialisers& GetSerialisers() const { return m_Serialisers; }
		int GetNbBaseTypes() const { return m_NbBaseTypes; }
		Type& GetBaseType(int index) const { RFLB_ASSERT(index >= 0 && index <  m_NbBaseTypes); return *m_BaseTypes[index]; }

		friend class TypeDatabase;

	private:
		void SetFields(const FieldInfo* fields, int nb_fields, TypeDatabase& type_db);

		// Description of the type
		Name m_Name;
		int m_Size;

		// Constructor/destructor
		internal::ConstructObjectFunc m_Constructor;
		internal::DestructObjectFunc m_Destructor;

		// Easily searchable array of fields in the type
		// Order is not guaranteed to match registration order
		Fields m_Fields;

		Serialisers m_Serialisers;

		// List of base types with very limited multiple inheritance
		static const int MAX_BASE_TYPES = 3;
		Type* m_BaseTypes[MAX_BASE_TYPES];
		int m_NbBaseTypes;
	};
}
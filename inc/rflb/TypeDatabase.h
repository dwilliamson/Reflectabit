
#pragma once


#include <typeinfo>
#include <map>
#include <rflb/Utils.h>


namespace rflb
{
	struct TypeInfo;
	struct FieldInfo;
	struct Field;
	class Type;


	class TypeDatabase
	{
	public:
		template <typename TYPE> Type& GetType()
		{
			return GetType(TypeInfo::Create<TYPE>());
		}

		template <typename TYPE> const Type& GetType() const
		{
			return GetType(TypeInfo::Create<TYPE>());
		}


		Type& GetType(const TypeInfo& type_info);
		const Type& GetType(const TypeInfo& type_info) const;


		template <typename TYPE, size_t N> Type& SetTypeFields(const FieldInfo (&fields)[N])
		{
			// Set the fields on the type
			Type& type = GetType<TYPE>();
			type.SetFields(fields, N, *this);
			return type;
		}

	private:
		// Map of all created types
		std::map<u32, Type*> m_Types;
	};
}
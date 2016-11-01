
#pragma once


#include <typeinfo>
#include <rflb/Type.h>
#include <rflb/Utils.h>
#include <rflb/Container.h>


namespace rflb
{
	class Type;
	class TypeDatabase;


	struct FieldAttr
	{
		enum
		{
			TRANSIENT = 1
		};

		FieldAttr() : m_Value(0)
		{
		}

		FieldAttr(u32 value) : m_Value(value)
		{
		}

		union
		{
			struct
			{
				u32 transient : 1;
			};
			u32 m_Value;
		};
	};


	struct FieldInfo
	{
		template <typename CLASS, typename TYPE> FieldInfo(const char* name, TYPE (CLASS::*field)) :
			m_Name(name),
			m_Offset((u32)offsetof(CLASS, *field)),
			m_TypeInfo(TypeInfo::Create<TYPE>()),
			m_Version(1)
		{
			// The object being passed is only used to figure out template parameters
			m_ContainerFactory = internal::CreateContainerFactory(((CLASS*)0)->*field, m_KeyTypeInfo, m_ValueTypeInfo);
		}

		// Chain these together to optionally modify field properties
		FieldInfo& Attributes(FieldAttr attributes);
		FieldInfo& LoadSaveBinary(SerialiseLoadFunc load, SerialiseSaveFunc save);
		FieldInfo& LoadSaveBinaryIFFv(SerialiseLoadFunc load, SerialiseSaveFunc save);
		FieldInfo& LoadSaveTextXML(SerialiseLoadFunc load, SerialiseSaveFunc save);
		FieldInfo& Version(u32 version);

		// All the data required for constructing a field
		Name m_Name;
		u32 m_Offset;
		TypeInfo m_TypeInfo;

		// Any extra container information
		IContainerFactory* m_ContainerFactory;
		TypeInfo m_KeyTypeInfo;
		TypeInfo m_ValueTypeInfo;

		FieldAttr m_Attributes;
		Serialisers m_Serialisers;
		u32 m_Version;
	};


	struct Field
	{
		Field();
		Field(const FieldInfo& field_info, TypeDatabase& type_db);

		// Parent-relative name
		Name m_Name;

		// Field type
		Type* m_Type;
		bool m_IsPointer;

		// Byte offset within parent
		u32 m_Offset;

		IContainerFactory* m_ContainerFactory;

		FieldAttr m_Attributes;
		Serialisers m_Serialisers;
		u32 m_Version;
	};
}
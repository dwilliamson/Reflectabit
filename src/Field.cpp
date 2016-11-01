
#include <rflb/Field.h>
#include <rflb/Type.h>
#include <rflb/TypeDatabase.h>
#include <rflb/Utils.h>


rflb::FieldInfo& rflb::FieldInfo::Attributes(FieldAttr attributes)
{
	m_Attributes = attributes;
	return *this;
}


rflb::FieldInfo& rflb::FieldInfo::Version(u32 version)
{
	m_Version = version;
	return *this;
}


rflb::FieldInfo& rflb::FieldInfo::LoadSaveBinary(SerialiseLoadFunc load, SerialiseSaveFunc save)
{
	m_Serialisers.m_LoadFuncs[SERIALISE_METHOD_BINARY] = load;
	m_Serialisers.m_SaveFuncs[SERIALISE_METHOD_BINARY] = save;
	return *this;
}


rflb::FieldInfo& rflb::FieldInfo::LoadSaveBinaryIFFv(SerialiseLoadFunc load, SerialiseSaveFunc save)
{
	m_Serialisers.m_LoadFuncs[SERIALISE_METHOD_BINARY_IFFV] = load;
	m_Serialisers.m_SaveFuncs[SERIALISE_METHOD_BINARY_IFFV] = save;
	return *this;
}


rflb::FieldInfo& rflb::FieldInfo::LoadSaveTextXML(SerialiseLoadFunc load, SerialiseSaveFunc save)
{
	m_Serialisers.m_LoadFuncs[SERIALISE_METHOD_TEXT_XML] = load;
	m_Serialisers.m_SaveFuncs[SERIALISE_METHOD_TEXT_XML] = save;
	return *this;
}


rflb::Field::Field()
{
	// For storing in std::map
}


rflb::Field::Field(const FieldInfo& field_info, TypeDatabase& type_db) :
	m_Name(field_info.m_Name),
	m_Type(&type_db.GetType(field_info.m_TypeInfo)),
	m_IsPointer(field_info.m_TypeInfo.m_IsPointer),
	m_Offset(field_info.m_Offset),
	m_ContainerFactory(field_info.m_ContainerFactory),
	m_Attributes(field_info.m_Attributes),
	m_Serialisers(field_info.m_Serialisers),
	m_Version(field_info.m_Version)
{
	// Resolve the container types, if present
	if (m_ContainerFactory)
	{
		if (field_info.m_KeyTypeInfo != TypeInfo())
		{
			m_ContainerFactory->m_KeyType = &type_db.GetType(field_info.m_KeyTypeInfo);
			m_ContainerFactory->m_KeyIsPointer = field_info.m_KeyTypeInfo.m_IsPointer;
		}
		if (field_info.m_ValueTypeInfo != TypeInfo())
		{
			m_ContainerFactory->m_ValueType = &type_db.GetType(field_info.m_ValueTypeInfo);
			m_ContainerFactory->m_ValueIsPointer = field_info.m_ValueTypeInfo.m_IsPointer;
		}
	}
}

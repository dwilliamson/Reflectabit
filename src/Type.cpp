
#include <rflb/Type.h>
#include <rflb/Field.h>
#include <rflb/Utils.h>


rflb::Type::Type(const TypeInfo& type_info) :
	m_Name(type_info.m_Name),
	m_Size(type_info.m_Size),
	m_Constructor(type_info.m_Constructor),
	m_Destructor(type_info.m_Destructor),
	m_NbBaseTypes(0)
{
}


const rflb::Field& rflb::Type::GetField(const Name& name) const
{
	Fields::const_iterator it = m_Fields.find(name.m_CRC);
	RFLB_ASSERT(it != m_Fields.end());
	return it->second;
}


const rflb::Field* rflb::Type::FindField(const Name& name) const
{
	Fields::const_iterator it = m_Fields.find(name.m_CRC);
	if (it == m_Fields.end())
		return 0;
	return &it->second;
}


void rflb::Type::SetFields(const FieldInfo* fields, int nb_fields, TypeDatabase& type_db)
{
	m_Fields.clear();

	// Create each field from the field infos provided
	for (int i = 0; i < nb_fields; i++)
	{
		const FieldInfo& field_info = fields[i];
		Field field = Field(field_info, type_db);
		m_Fields[field.m_Name.m_CRC] = field;
	}
}


rflb::Type& rflb::Type::LoadSaveBinary(SerialiseLoadFunc load, SerialiseSaveFunc save)
{
	m_Serialisers.m_LoadFuncs[SERIALISE_METHOD_BINARY] = load;
	m_Serialisers.m_SaveFuncs[SERIALISE_METHOD_BINARY] = save;
	return *this;
}


rflb::Type& rflb::Type::LoadSaveBinaryIFFv(SerialiseLoadFunc load, SerialiseSaveFunc save)
{
	m_Serialisers.m_LoadFuncs[SERIALISE_METHOD_BINARY_IFFV] = load;
	m_Serialisers.m_SaveFuncs[SERIALISE_METHOD_BINARY_IFFV] = save;
	return *this;
}


rflb::Type& rflb::Type::LoadSaveTextXML(SerialiseLoadFunc load, SerialiseSaveFunc save)
{
	m_Serialisers.m_LoadFuncs[SERIALISE_METHOD_TEXT_XML] = load;
	m_Serialisers.m_SaveFuncs[SERIALISE_METHOD_TEXT_XML] = save;
	return *this;
}


rflb::Type& rflb::Type::Inherits(Type& base)
{
	RFLB_ASSERT(m_NbBaseTypes < MAX_BASE_TYPES);
	m_BaseTypes[m_NbBaseTypes++] = &base;
	return *this;
}


void rflb::Type::ConstructObject(void* object)
{
	m_Constructor(object);
}


void rflb::Type::DestructObject(void* object)
{
	m_Destructor(object);
}
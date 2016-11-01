
#include <rflb/TypeDatabase.h>
#include <rflb/Type.h>


rflb::Type& rflb::TypeDatabase::GetType(const TypeInfo& type_info)
{
	// Add the type if it doesn't already exist
	if (m_Types.find(type_info.m_Name.m_CRC) == m_Types.end())
	{
		m_Types[type_info.m_Name.m_CRC] = new Type(type_info);
	}

	return *m_Types[type_info.m_Name.m_CRC];
}


const rflb::Type& rflb::TypeDatabase::GetType(const TypeInfo& type_info) const
{
	// Assert if the type doesn't already exist
	std::map<u32, Type*>::const_iterator it = m_Types.find(type_info.m_Name.m_CRC);
	RFLB_ASSERT(it != m_Types.end());
	return *it->second;
}

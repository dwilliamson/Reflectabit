
#include <rflb/SerialiseBinary.h>
#include <rflb/Type.h>
#include <rflb/Field.h>
#include <iostream>

using namespace rflb;


namespace
{
	struct FieldHeader
	{
		FieldHeader()
			: m_NameCRC(0)
			, m_Version(0)
			, m_DataSize(0)
			, m_WritePosition(0)
		{
		}

		FieldHeader(const Field& field)
			: m_NameCRC(field.m_Name.m_CRC)
			, m_Version(field.m_Version)
			, m_DataSize(field.m_Type->GetSize())
			, m_WritePosition(0)
		{
		}

		void Read(std::istream& stream)
		{
			StreamRead(stream, m_NameCRC);
			StreamRead(stream, m_Version);
			StreamRead(stream, m_DataSize);
		}

		void Write(std::ostream& stream)
		{
			StreamWrite(stream, m_NameCRC);
			StreamWrite(stream, m_Version);
			m_WritePosition = (u32)stream.tellp();
			StreamWrite(stream, m_DataSize);
		}

		// how do you backpatch a network stream?
		// you don't... you allocate space for the entire object and send that in one go
		void PatchDataSize(std::ostream& stream)
		{
			// Calculate size of data written since header write
			u32 cur_pos = (u32)stream.tellp();
			u32 size = cur_pos - (m_WritePosition + sizeof(u32));

			// Jump back and patch up the size
			stream.seekp(m_WritePosition, std::ostream::beg);
			StreamWrite(stream, size);
			stream.seekp(cur_pos, std::ostream::beg);
		}

		u32 m_NameCRC;
		u32 m_Version;
		u32 m_DataSize;
		u32 m_WritePosition;
	};


	void LoadObject(std::istream& stream, void* object, const Type* object_type, bool is_pointer, IContainerFactory* factory, SerialiseMethod method);
	void SaveObject(std::ostream& stream, const void* object, const Type* object_type, bool is_pointer, IContainerFactory* factory, SerialiseMethod method);
	void LoadBinary(std::istream& stream, void* object, const Type* object_type, SerialiseMethod method);
	void SaveBinary(std::ostream& stream, const void* object, const Type* object_type, SerialiseMethod method);


	void LoadCollection(std::istream& stream, void* object, IContainerFactory* factory, SerialiseMethod method)
	{
		// Create an iterator and read the count
		IWriteIterator* iterator = RFLB_NEW_TEMP_WRITE_ITERATOR(factory, object);
		int count;
		StreamRead(stream, count);

		if (Type* key_type = factory->m_KeyType)
		{
			// Construct a temporary for the key
			void* key = _alloca(key_type->GetSize());
			key_type->ConstructObject(key);

			// Load the key/value pairs of the container
			for (int i = 0; i < count; i++)
			{
				LoadObject(stream, key, key_type, false, 0, method);
				void* value_object = iterator->AddEmpty(key);
				LoadObject(stream, value_object, factory->m_ValueType, factory->m_ValueIsPointer, 0, method);
			}

			key_type->DestructObject(key);
		}

		else
		{
			// Just load the values of the container
			for (int i = 0; i < count; i++)
			{
				void* value_object = iterator->AddEmpty();
				LoadObject(stream, value_object, factory->m_ValueType, factory->m_ValueIsPointer, 0, method);
			}
		}

		RFLB_DELETE_TEMP_ITERATOR(factory, iterator);
	}



	// NOTE: All of these branches can be "baked" into the field load function

	void LoadObject(std::istream& stream, void* object, const Type* object_type, bool is_pointer, IContainerFactory* factory, SerialiseMethod method)
	{
		if (is_pointer)
		{
			// TODO: read CRC and lookup object
		}

		else if (SerialiseLoadFunc load = object_type->GetSerialisers().m_LoadFuncs[method])
		{
			load(stream, 0, object);
		}

		else if (factory)
		{
			LoadCollection(stream, object, factory, method);
		}

		else if (object_type->GetFields().empty())
		{
			// Straight read of PODs
			// TODO: endian-ness
			stream.read((char*)object, object_type->GetSize());
		}

		else
		{
			// Recurse into the fields of this object
			LoadBinary(stream, object, object_type, method);
		}
	}


	void LoadField(std::istream& stream, void* object, const Field& field, SerialiseMethod method)
	{
		void* field_data = (char*)object + field.m_Offset;

		if (SerialiseLoadFunc load_func = field.m_Serialisers.m_LoadFuncs[method])
		{
			load_func(stream, 0, field_data);
		}

		else
		{
			LoadObject(stream, field_data, field.m_Type, field.m_IsPointer, field.m_ContainerFactory, method);
		}
	}


	void LoadBinary(std::istream& stream, void* object, const Type* object_type, SerialiseMethod method)
	{
		if (method == SERIALISE_METHOD_BINARY_IFFV)
		{
			int nb_fields;
			StreamRead(stream, nb_fields);

			for (int i = 0; i < nb_fields; i++)
			{
				FieldHeader header;
				header.Read(stream);

				const Field* field = object_type->FindField(Name(header.m_NameCRC));
				if (field && field->m_Version == header.m_Version)
				{
					u32 field_start = (u32)stream.tellg();
					LoadField(stream, object, *field, method);

					if ((u32)stream.tellg() - field_start != header.m_DataSize)
					{
						// ERROR: Field over/under flow
						// Attempt to seek to the next field
						stream.seekg(field_start + header.m_DataSize);
					}
				}

				else
				{
					// Field not found or version mismatch
					stream.seekg(header.m_DataSize, std::ios_base::cur);
				}
			}
		}

		else
		{
			const Fields& fields = object_type->GetFields();
			for (Fields::const_iterator i = fields.begin(); i != fields.end(); ++i)
			{
				LoadField(stream, object, i->second, method);
			}
		}

		// Recurse into base types
		for (int i = 0; i < object_type->GetNbBaseTypes(); i++)
		{
			LoadBinary(stream, object, &object_type->GetBaseType(i), method);
		}
	}


	void SaveCollection(std::ostream& stream, const void* object, IContainerFactory* factory, SerialiseMethod method)
	{
		// Create an iterator and write the count
		IReadIterator* iterator = RFLB_NEW_TEMP_READ_ITERATOR(factory, object);
		StreamWrite(stream, iterator->GetCount());

		if (factory->m_KeyType)
		{
			// Save the key/value pairs of the container
			while (iterator->IsValid())
			{
				SaveObject(stream, iterator->GetKey(), factory->m_KeyType, factory->m_KeyIsPointer, 0, method);
				SaveObject(stream, iterator->GetValue(), factory->m_ValueType, factory->m_ValueIsPointer, 0, method);
				iterator->MoveNext();
			}
		}
		else
		{
			// Save just the values of the container
			while (iterator->IsValid())
			{
				SaveObject(stream, iterator->GetValue(), factory->m_ValueType, factory->m_ValueIsPointer, 0, method);
				iterator->MoveNext();
			}
		}

		RFLB_DELETE_TEMP_ITERATOR(factory, iterator);
	}


	void SaveObject(std::ostream& stream, const void* object, const Type* object_type, bool is_pointer, IContainerFactory* factory, SerialiseMethod method)
	{
		if (is_pointer)
		{
			// TODO: get CRC and serialise that
		}

		else if (SerialiseSaveFunc save = object_type->GetSerialisers().m_SaveFuncs[method])
		{
			// Custom save per type
			save(stream, 0, object);
		}

		// NOTE: This branch is taken before checking for POD status as the container, obviously,
		// has no fields
		else if (factory)
		{
			SaveCollection(stream, object, factory, method);
		}

		else if (object_type->GetFields().empty())
		{
			// Directly write PODs
			// TODO: endian-ness
			stream.write((char*)object, object_type->GetSize());
		}

		else
		{
			// Recurse into the fields of this object
			SaveBinary(stream, object, object_type, method);
		}
	}


	void SaveBinary(std::ostream& stream, const void* object, const Type* object_type, SerialiseMethod method)
	{
		const Fields& fields = object_type->GetFields();
		if (method == SERIALISE_METHOD_BINARY_IFFV)
		{
			StreamWrite(stream, fields.size());
		}

		for (Fields::const_iterator i = fields.begin(); i != fields.end(); ++i)
		{
			const Field& field = i->second;
			FieldHeader header(field);

			if (method == SERIALISE_METHOD_BINARY_IFFV)
			{
				header.Write(stream);
			}

			if (SerialiseSaveFunc save_func = field.m_Serialisers.m_SaveFuncs[method])
			{
				save_func(stream, 0, (const char*)object + field.m_Offset);
			}

			else
			{
				SaveObject(stream, (const char*)object + field.m_Offset, field.m_Type, field.m_IsPointer, field.m_ContainerFactory, method);
			}

			if (method == SERIALISE_METHOD_BINARY_IFFV)
			{
				header.PatchDataSize(stream);
			}
		}

		// Recurse into base types
		for (int i = 0; i < object_type->GetNbBaseTypes(); i++)
		{
			SaveBinary(stream, object, &object_type->GetBaseType(i), method);
		}
	}
}


void serialise::LoadBinary(std::istream& stream, void* object, const Type* object_type)
{
	::LoadBinary(stream, object, object_type, SERIALISE_METHOD_BINARY);
}


void serialise::SaveBinary(std::ostream& stream, const void* object, const Type* object_type)
{
	::SaveBinary(stream, object, object_type, SERIALISE_METHOD_BINARY);
}


void serialise::LoadBinaryIFFV(std::istream& stream, void* object, const Type* object_type)
{
	::LoadBinary(stream, object, object_type, SERIALISE_METHOD_BINARY_IFFV);
}


void serialise::SaveBinaryIFFV(std::ostream& stream, const void* object, const rflb::Type* object_type)
{
	::SaveBinary(stream, object, object_type, SERIALISE_METHOD_BINARY_IFFV);
}
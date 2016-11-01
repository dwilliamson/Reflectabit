
#pragma once


#include <iosfwd>


namespace rflb
{
	class Type;
}


namespace serialise
{
	void LoadBinary(std::istream& stream, void* object, const rflb::Type* object_type);
	void SaveBinary(std::ostream& stream, const void* object, const rflb::Type* object_type);

	void LoadBinaryIFFV(std::istream& stream, void* object, const rflb::Type* object_type);
	void SaveBinaryIFFV(std::ostream& stream, const void* object, const rflb::Type* object_type);
}

#pragma once


#include <assert.h>
#include <iosfwd>


typedef unsigned int u32;


namespace rflb
{
	namespace internal
	{
		// Very basic static assert, based on the Boost implementation - can only be used at function scope
		template <bool> struct StaticAssertionFailure;
		template <> struct StaticAssertionFailure<true> { };
		#define RFLB_STATIC_ASSERT(condition) sizeof(rflb::internal::StaticAssertionFailure<condition>)


		struct AssertException
		{
		};


		// Quick hashing function
		inline u32 adler32(const char *data)
		{
			static const u32 MOD_ADLER = 65521;
			u32 a = 1, b = 0;

			/* Loop over each byte of data, in order */
			for (size_t index = 0; data[index]; ++index)
			{
				a = (a + data[index]) % MOD_ADLER;
				b = (b + a) % MOD_ADLER;
			}

			return (b << 16) | a;
		}


		// Figure out if a type is a pointer
		template <typename TYPE> struct is_pointer
		{
			enum { val = 0 };
		};
		template <typename TYPE> struct is_pointer<TYPE*>
		{
			enum { val = 1 };
		};


		// Removes the pointer from a type
		template <typename TYPE> struct strip_pointer
		{
			typedef TYPE Type;
		};
		template <typename TYPE> struct strip_pointer<TYPE*>
		{
			typedef TYPE Type;
		};
	}


	// Name/CRC pair (32-bit)
	struct Name
	{
		Name() : m_Text(0), m_CRC(0)
		{
		}

		explicit Name(const char* text) : m_Text(text)
		{
			m_CRC = internal::adler32(text);
		}

		explicit Name(u32 crc) : m_Text(0), m_CRC(crc)
		{
		}

		bool operator == (const Name& rhs) const
		{
			return m_CRC == rhs.m_CRC;
		}

		const char* m_Text;
		u32 m_CRC;
	};


	typedef void (*SerialiseSaveFunc)(std::ostream&, u32 version, const void* data);
	typedef void (*SerialiseLoadFunc)(std::istream&, u32 version, void* data);

	enum SerialiseMethod
	{
		SERIALISE_METHOD_BINARY,
		SERIALISE_METHOD_BINARY_IFFV,
		SERIALISE_METHOD_TEXT_XML,
		SERIALISE_METHOD_COUNT
	};


	//
	// This stores a list of serialisation functions for each known method,
	// that will be embedded within a type or field. Rather than adding some
	// form of anonymous component system to types and fields, this much faster
	// method is also easier to use, while introducing a coupling of concepts
	// that's a worthy price to pay.
	//
	// This does not prevent the implementation of other serialisers (e.g.
	// a registry serialiser). You can:
	//
	//    * Reuse the existing serialisers.
	//    * Extend this class to add their own (at the cost of increasing the size of Type and pushing it out of a cacheline).
	//    * Use an alternate method of type serialiser lookup, e.g. a map.
	//
	struct Serialisers
	{
		Serialisers()
		{
			for (int i = 0; i < SERIALISE_METHOD_COUNT; i++)
			{
				m_SaveFuncs[i] = 0;
				m_LoadFuncs[i] = 0;
			}
		}

		Serialisers(const Serialisers& rhs)
		{
			for (int i = 0; i < SERIALISE_METHOD_COUNT; i++)
			{
				m_SaveFuncs[i] = rhs.m_SaveFuncs[i];
				m_LoadFuncs[i] = rhs.m_LoadFuncs[i];
			}
		}

		SerialiseSaveFunc m_SaveFuncs[SERIALISE_METHOD_COUNT];
		SerialiseLoadFunc m_LoadFuncs[SERIALISE_METHOD_COUNT];
	};


	// STL stream read/write for basic types that by-passes the stream formatting
	// Does the silly cast and figures out the data size

	template <typename TYPE> void StreamWrite(std::ostream& stream, const TYPE& data)
	{
		stream.write((char*)&data, sizeof(data));
	}

	template <typename TYPE> void StreamRead(std::istream& stream, TYPE& data)
	{
		stream.read((char*)&data, sizeof(data));
	}
}


#ifdef RFLB_ASSERT_THROWS
#define RFLB_ASSERT(condition) { if (!(condition)) throw rflb::internal::AssertException(); }
#else
#define RFLB_ASSERT(condition) { if (!(condition)) { __asm { int 3 } } }
#endif
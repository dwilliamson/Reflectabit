
#include <sstream>
#include <cstdarg>

#include <rflb/Type.h>
#include <rflb/Field.h>
#include <rflb/TypeDatabase.h>
#include <rflb/VectorContainer.h>
#include <rflb/ArrayContainer.h>
#include <rflb/MapContainer.h>
#include <rflb/SerialiseBinary.h>


#define TEST_ASSERT(condition) printf("Test (A:%s): %s\n", (condition) ? "Pass" : "FAIL", #condition);
#define TEST_EXCEPTION(expr) { printf("Test (E:"); try { expr; printf("FAIL"); } catch (const rflb::internal::AssertException&) { printf("Pass"); } printf("): %s\n", #expr); }



template <typename TYPE, int LENGTH>
bool ArraysEqual(const TYPE (&a0)[LENGTH], const TYPE (&a1)[LENGTH])
{
	for (int i = 0; i < LENGTH; i++)
	{
		if (!(a0[i] == a1[i]))
			return false;
	}

	return true;
}


template <typename TYPE> struct VAType
{
	typedef TYPE Type;
};
template <> struct VAType<float>
{
	typedef double Type;
};
#define va_arg_safe(args, type) (type)va_arg(args, VAType<type>::Type)


#define RFLB_BEGIN_TYPE_FIELDS(db, type)			\
	rflb::TypeDatabase& local_db = db;				\
	struct TypeFields								\
	{												\
		typedef type Type;							\
													\
		static void Set(rflb::TypeDatabase& db)		\
		{											\
			rflb::FieldInfo fields[] =				\
			{


#define RFLB_FIELD(name) rflb::FieldInfo(#name, &Type::name)

#define RFLB_END_TYPE_FIELDS()						\
			};										\
			db.SetTypeFields<Type>(fields);			\
		}											\
	};												\
	TypeFields::Set(local_db);


struct TestVector
{
	TestVector() : x(0), y(0) { }
	TestVector(int x_, int y_) : x(x_), y(y_) { }

	static void Register(rflb::TypeDatabase& db)
	{
		/*using namespace rflb;
		FieldInfo fields[] =
		{
			FieldInfo("x", &TestVector::x),
			FieldInfo("y", &TestVector::y)
		};
		db.SetTypeFields<TestVector>(fields);*/

		RFLB_BEGIN_TYPE_FIELDS(db, TestVector)
			RFLB_FIELD(x),
			RFLB_FIELD(y)
		RFLB_END_TYPE_FIELDS()
	}

	int x, y;

	bool operator == (const TestVector& rhs) const
	{
		return x == rhs.x && y == rhs.y;
	}

	// For use as a std::map key
	bool operator < (const TestVector& rhs) const
	{
		return y < rhs.y || (x < rhs.x && y == rhs.y);
	}
};


struct Values
{
	static void Register(rflb::TypeDatabase& db)
	{
		/*using namespace rflb;
		FieldInfo fields[] =
		{
			FieldInfo("char_value", &Values::char_value),
			FieldInfo("short_value", &Values::short_value),
			FieldInfo("int_value", &Values::int_value),
			FieldInfo("float_value", &Values::float_value),
			FieldInfo("double_value", &Values::double_value),
			FieldInfo("custom_string_type", &Values::custom_string_type),
			FieldInfo("embedded_pod", &Values::embedded_pod)
		};
		db.SetTypeFields<Values>(fields);*/

		RFLB_BEGIN_TYPE_FIELDS(db, Values)
			RFLB_FIELD(char_value),
			RFLB_FIELD(short_value),
			RFLB_FIELD(int_value),
			RFLB_FIELD(float_value),
			RFLB_FIELD(double_value),
			RFLB_FIELD(custom_string_type),
			RFLB_FIELD(embedded_pod)
		RFLB_END_TYPE_FIELDS()
	}

	void Set()
	{
		char_value = 3;
		short_value = 31000;
		int_value = 2329452;
		float_value = 1 / 256.0f;
		double_value = 1 / double((__int64)1 << 35);
		custom_string_type = "Blah";
		embedded_pod = TestVector(65536, 65537);
	}

	void TestAgainst(const Values& other) const
	{
		TEST_ASSERT(char_value == other.char_value);
		TEST_ASSERT(short_value == other.short_value);
		TEST_ASSERT(int_value == other.int_value);
		TEST_ASSERT(float_value == other.float_value);
		TEST_ASSERT(double_value == other.double_value);
		TEST_ASSERT(custom_string_type == other.custom_string_type);
		TEST_ASSERT(embedded_pod == other.embedded_pod);
	}

	char char_value;
	short short_value;
	int int_value;
	float float_value;
	double double_value;
	std::string custom_string_type;
	TestVector embedded_pod;
};


struct Arrays
{
	static void Register(rflb::TypeDatabase& db)
	{
		using namespace rflb;
		FieldInfo fields[] =
		{
			FieldInfo("char_array", &Arrays::char_array),
			FieldInfo("short_array", &Arrays::short_array),
			FieldInfo("int_array", &Arrays::int_array),
			FieldInfo("float_array", &Arrays::float_array),
			FieldInfo("double_array", &Arrays::double_array),
			FieldInfo("string_array", &Arrays::string_array),
			FieldInfo("pod_array", &Arrays::pod_array)
		};
		db.SetTypeFields<Arrays>(fields);
	}

	void Set()
	{
		Set(char_array, 6, 1, 2, 3, -4, 5, 6);
		Set(short_array, 4, 23000, 23001, -9, 32767);
		Set(int_array, 5, 100000, 50000, 123, 5838474, -46763);
		Set(float_array, 3, (1 / 32.0), -(1 / 4.0), (1 / 2048.0));
		Set(double_array, 4, 1.0, 2.0, -128e-4, 2e20);
		Set(string_array, 2, std::string("Bokhara"), std::string("Tai-du"));
		Set(pod_array, 2, TestVector(78, 142), TestVector(424, 23));
	}

	void TestAgainst(const Arrays& other) const
	{
		TEST_ASSERT(ArraysEqual(char_array, other.char_array));
		TEST_ASSERT(ArraysEqual(short_array, other.short_array));
		TEST_ASSERT(ArraysEqual(int_array, other.int_array));
		TEST_ASSERT(ArraysEqual(float_array, other.float_array));
		TEST_ASSERT(ArraysEqual(double_array, other.double_array));
		TEST_ASSERT(ArraysEqual(string_array, other.string_array));
		TEST_ASSERT(ArraysEqual(pod_array, other.pod_array));
	}

	template <typename VALUE_TYPE>
	void Set(VALUE_TYPE* array, int count, ...)
	{
		va_list args;
		va_start(args, count);
		for (int i = 0; i < count; i++)
		{
			array[i] = va_arg_safe(args, VALUE_TYPE);
		}
		va_end(args);
	}

	char char_array[6];
	short short_array[4];
	int int_array[5];
	float float_array[3];
	double double_array[4];
	std::string string_array[2];
	TestVector pod_array[2];
};


struct Vectors
{
	static void Register(rflb::TypeDatabase& db)
	{
		using namespace rflb;
		FieldInfo fields[] =
		{
			FieldInfo("char_vector", &Vectors::char_vector),
			FieldInfo("short_vector", &Vectors::short_vector),
			FieldInfo("int_vector", &Vectors::int_vector),
			FieldInfo("float_vector", &Vectors::float_vector),
			FieldInfo("double_vector", &Vectors::double_vector),
			FieldInfo("string_vector", &Vectors::string_vector),
			FieldInfo("pod_vector", &Vectors::pod_vector)
		};
		db.SetTypeFields<Vectors>(fields);
	}

	void Set()
	{
		Set(char_vector, 3, 34, 15, -3);
		Set(short_vector, 7, 32241, 8934, -2323, 988, 12398, 1222, 44);
		Set(int_vector, 2, 43928434, 2323);
		Set(float_vector, 1, 456.123);
		Set(double_vector, 3, 123.1555, 98.1, -841414415.23232);
		Set(string_vector, 4, std::string("Beijing"), std::string("Amoy"), std::string("Kanbalu"), std::string("Cathay"));
		Set(pod_vector, 2, TestVector(57, 12), TestVector(90, 391));
	}

	void TestAgainst(const Vectors& other) const
	{
		TEST_ASSERT(char_vector == other.char_vector);
		TEST_ASSERT(short_vector == other.short_vector);
		TEST_ASSERT(int_vector == other.int_vector);
		TEST_ASSERT(float_vector == other.float_vector);
		TEST_ASSERT(double_vector == other.double_vector);
		TEST_ASSERT(string_vector == other.string_vector);
		TEST_ASSERT(pod_vector == other.pod_vector);
	}

	template <typename VALUE_TYPE>
	void Set(std::vector<VALUE_TYPE>& vector, int count, ...)
	{
		va_list args;
		va_start(args, count);
		for (int i = 0; i < count; i++)
		{
			vector.push_back(va_arg_safe(args, VALUE_TYPE));
		}
		va_end(args);
	}

	std::vector<char> char_vector;
	std::vector<short> short_vector;
	std::vector<int> int_vector;
	std::vector<float> float_vector;
	std::vector<double> double_vector;
	std::vector<std::string> string_vector;
	std::vector<TestVector> pod_vector;
};


struct Maps
{
	static void Register(rflb::TypeDatabase& db)
	{
		using namespace rflb;
		FieldInfo fields[] =
		{
			FieldInfo("char_map", &Maps::char_map),
			FieldInfo("short_map", &Maps::short_map),
			FieldInfo("int_map", &Maps::int_map),
			FieldInfo("float_map", &Maps::float_map),
			FieldInfo("double_map", &Maps::double_map),
			FieldInfo("string_map", &Maps::string_map),
			FieldInfo("pod_map", &Maps::pod_map)
		};
		db.SetTypeFields<Maps>(fields);
	}

	void Set()
	{
		Set(char_map, 3, TestVector(1, 2), 3, TestVector(4242, 23), 5, TestVector(23, 23), 123);
		Set(short_map, 2, std::string("Nobby"), 23232, std::string("Vimes"), 3233);
		Set(int_map, 4, 23.0, 3323232, 44.44, 2309, 23.1444, -23, 1e45, 444444);
		Set(float_map, 3, 45.0f, 45.0f, 6.0f, 12.3f, 0.1f, 0.324f);
		Set(double_map, 1, 300, 0.5);
		Set(string_map, 2, 41, std::string("Carrot"), 32456, std::string("Detritus"));
		Set(pod_map, 2, 1, TestVector(320, 240), 2, TestVector(0xFC00, 0xA000));
	}

	void TestAgainst(const Maps& other) const
	{
		TEST_ASSERT(char_map == other.char_map);
		TEST_ASSERT(short_map == other.short_map);
		TEST_ASSERT(int_map == other.int_map);
		TEST_ASSERT(float_map == other.float_map);
		TEST_ASSERT(double_map == other.double_map);
		TEST_ASSERT(string_map == other.string_map);
		TEST_ASSERT(pod_map == other.pod_map);
	}

	template <typename KEY_TYPE, typename VALUE_TYPE>
	void Set(std::map<KEY_TYPE, VALUE_TYPE>& map, int count, ...)
	{
		va_list args;
		va_start(args, count);
		for (int i = 0; i < count; i++)
		{
			KEY_TYPE key = va_arg_safe(args, KEY_TYPE);
			VALUE_TYPE value = va_arg_safe(args, VALUE_TYPE);
			map[key] = value;
		}
		va_end(args);
	}

	// Specifically test custom operator< implementations
	std::map<TestVector, char> char_map;
	std::map<std::string, short> short_map;
	std::map<double, int> int_map;
	std::map<float, float> float_map;
	std::map<int, double> double_map;
	std::map<short, std::string> string_map;
	std::map<char, TestVector> pod_map;
};


struct TestData
{
	static void Register(rflb::TypeDatabase& db)
	{
		using namespace rflb;
		FieldInfo fields[] =
		{
			FieldInfo("values", &TestData::values),
			FieldInfo("arrays", &TestData::arrays),
			FieldInfo("vectors", &TestData::vectors),
			FieldInfo("maps", &TestData::maps)
		};
		db.SetTypeFields<TestData>(fields);
	}

	void Set()
	{
		values.Set();
		arrays.Set();
		vectors.Set();
		maps.Set();
	}

	void TestAgainst(const TestData& other) const
	{
		values.TestAgainst(other.values);
		arrays.TestAgainst(other.arrays);
		vectors.TestAgainst(other.vectors);
		maps.TestAgainst(other.maps);
	}

	Values values;
	Arrays arrays;
	Vectors vectors;
	Maps maps;
};


struct TestBase
{
	static void Register(rflb::TypeDatabase& db)
	{
		using namespace rflb;
		FieldInfo fields[] =
		{
			FieldInfo("data", &TestBase::data)
		};
		db.SetTypeFields<TestBase>(fields);
	}

	TestData data;
};


struct TestDerived : public TestBase
{
	static const rflb::Type& Register(rflb::TypeDatabase& db)
	{
		using namespace rflb;
		FieldInfo fields[] =
		{
			FieldInfo("data2", &TestDerived::data2)
		};
		return db.SetTypeFields<TestDerived>(fields).Inherits(db.GetType<TestBase>());
	}

	void Set()
	{
		data.Set();
		data2.Set();
	}

	TestData data2;
};


void TestBinarySerialisation(rflb::TypeDatabase& db)
{
	printf("\nTestBinarySerialisation\n\n");

	TestDerived src, dst;
	src.Set();

	std::stringstream binary_data;
	serialise::SaveBinary(binary_data, &src, &db.GetType<TestDerived>());
	serialise::LoadBinary(binary_data, &dst, &db.GetType<TestDerived>());

	printf("= BASE ====================================================\n");
	dst.data.TestAgainst(src.data);
	printf("= DERIVED =================================================\n");
	dst.data2.TestAgainst(src.data2);
	printf("===========================================================\n");
}


void TestBinaryIFFVSerialisation(rflb::TypeDatabase& db)
{
	printf("\nTestBinaryIFFVSerialisation\n\n");

	TestDerived src, dst;
	src.Set();

	std::stringstream binary_data;
	serialise::SaveBinaryIFFV(binary_data, &src, &db.GetType<TestDerived>());
	serialise::LoadBinaryIFFV(binary_data, &dst, &db.GetType<TestDerived>());

	printf("= BASE ====================================================\n");
	dst.data.TestAgainst(src.data);
	printf("= DERIVED =================================================\n");
	dst.data2.TestAgainst(src.data2);
	printf("===========================================================\n");
}


void TestSerialisation(rflb::TypeDatabase& db)
{
	// Register backwards to ensure out-of-order registration is supported
	TestDerived::Register(db);
	TestBase::Register(db);
	TestData::Register(db);
	Maps::Register(db);
	Vectors::Register(db);
	Arrays::Register(db);
	Values::Register(db);
	TestVector::Register(db);

	TestBinarySerialisation(db);
	TestBinaryIFFVSerialisation(db);
}

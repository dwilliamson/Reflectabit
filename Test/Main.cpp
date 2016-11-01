
#include <cstdio>
#include <vector>
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


void LoadStringBinary(std::istream& stream, u32, void* data)
{
	std::string& str = *(std::string*)data;
	int length = 0;
	stream.read((char*)&length, sizeof(length));
	str.resize(length);
	stream.read(&str[0], (int)length);
}


void SaveStringBinary(std::ostream& stream, u32, const void* data)
{
	const std::string& str = *(const std::string*)data;
	size_t length = str.length();
	stream.write((char*)&length, sizeof(length));
	stream.write(str.c_str(), (int)length);
}


void LoadCharStringBinary(std::istream& stream, u32, void* data)
{
	stream.read((char*)data, 6);
}


void SaveCharStringBinary(std::ostream& stream, u32, const void* data)
{
	stream.write((char*)data, 6);
}


void TestArrayContainer()
{
	printf("\nTestArrayContainer\n\n");

	using namespace rflb;

	int old_array[5] = { 1, 2, 3, 4, 5 };
	int new_array[5] = { 0 };

	// Create factory
	TypeInfo key_type, value_type;
	IContainerFactory* factory = internal::CreateContainerFactory(new_array, key_type, value_type);
	TEST_ASSERT(key_type == TypeInfo());
	TEST_ASSERT(value_type == TypeInfo::Create<int>());

	// Test adding to the end
	IWriteIterator* w_iterator = RFLB_NEW_TEMP_WRITE_ITERATOR(factory, new_array);
	w_iterator->Add(old_array + 4);
	w_iterator->Add(old_array + 3);
	w_iterator->Add(old_array + 2);
	w_iterator->Add(old_array + 1);
	w_iterator->Add(old_array + 0);
	TEST_ASSERT(new_array[0] == 5);
	TEST_ASSERT(new_array[1] == 4);
	TEST_ASSERT(new_array[2] == 3);
	TEST_ASSERT(new_array[3] == 2);
	TEST_ASSERT(new_array[4] == 1);

	// Add overflow and adding by key
	TEST_EXCEPTION(w_iterator->Add(old_array));
	TEST_EXCEPTION(w_iterator->Add(old_array, old_array));

	// Test iteration
	IReadIterator* r_iterator = RFLB_NEW_TEMP_READ_ITERATOR(factory, new_array);
	TEST_ASSERT(r_iterator->GetCount() == 5);
	int count = 5;
	while (r_iterator->IsValid())
	{	
		TEST_EXCEPTION(r_iterator->GetKey());
		int value = *(int*)r_iterator->GetValue();
		TEST_ASSERT(value == count--);
		r_iterator->MoveNext();
	}

	// Get/increment overflow
	TEST_EXCEPTION(r_iterator->GetKey());
	TEST_EXCEPTION(r_iterator->GetValue());
	TEST_EXCEPTION(r_iterator->MoveNext());

	RFLB_DELETE_TEMP_ITERATOR(factory, w_iterator);
	RFLB_DELETE_TEMP_ITERATOR(factory, r_iterator);
	delete factory;
}


void TestVectorContainer()
{
	printf("\nTestVectorContainer\n\n");

	using namespace rflb;

	int old_array[5] = { 1, 2, 3, 4, 5 };
	std::vector<int> new_vector;

	// Create factory
	TypeInfo key_type, value_type;
	IContainerFactory* factory = internal::CreateContainerFactory(new_vector, key_type, value_type);
	TEST_ASSERT(key_type == TypeInfo());
	TEST_ASSERT(value_type == TypeInfo::Create<int>());

	// Test adding to the end
	IWriteIterator* w_iterator = RFLB_NEW_TEMP_WRITE_ITERATOR(factory, &new_vector);
	w_iterator->Add(old_array + 4);
	w_iterator->Add(old_array + 3);
	w_iterator->Add(old_array + 2);
	w_iterator->Add(old_array + 1);
	w_iterator->Add(old_array + 0);
	TEST_ASSERT(new_vector[0] == 5);
	TEST_ASSERT(new_vector[1] == 4);
	TEST_ASSERT(new_vector[2] == 3);
	TEST_ASSERT(new_vector[3] == 2);
	TEST_ASSERT(new_vector[4] == 1);

	// Adding by key
	TEST_EXCEPTION(w_iterator->Add(old_array, old_array));

	// Test iteration
	IReadIterator* r_iterator = RFLB_NEW_TEMP_READ_ITERATOR(factory, &new_vector);
	TEST_ASSERT(r_iterator->GetCount() == 5);
	int count = 5;
	while (r_iterator->IsValid())
	{
		TEST_EXCEPTION(r_iterator->GetKey());
		int value = *(int*)r_iterator->GetValue();
		TEST_ASSERT(value == count--);
		r_iterator->MoveNext();
	}

	// Get/increment overflow
	TEST_EXCEPTION(r_iterator->GetKey());
	TEST_EXCEPTION(r_iterator->GetValue());
	TEST_EXCEPTION(r_iterator->MoveNext());

	RFLB_DELETE_TEMP_ITERATOR(factory, w_iterator);
	RFLB_DELETE_TEMP_ITERATOR(factory, r_iterator);
	delete factory;
}


void TestMapContainer()
{
	printf("\nTestMapContainer\n\n");

	using namespace rflb;

	int old_array[5] = { 1, 2, 3, 4, 5 };
	std::string keys[5] = { "0", "1", "2", "3", "4" };
	std::map<std::string, int> new_map;

	// Create factory
	TypeInfo key_type, value_type;
	IContainerFactory* factory = internal::CreateContainerFactory(new_map, key_type, value_type);
	TEST_ASSERT(key_type == TypeInfo::Create<std::string>());
	TEST_ASSERT(value_type == TypeInfo::Create<int>());

	// Test adding to the end
	IWriteIterator* w_iterator = RFLB_NEW_TEMP_WRITE_ITERATOR(factory, &new_map);
	w_iterator->Add(keys + 0, old_array + 4);
	w_iterator->Add(keys + 1, old_array + 3);
	w_iterator->Add(keys + 2, old_array + 2);
	w_iterator->Add(keys + 3, old_array + 1);
	w_iterator->Add(keys + 4, old_array + 0);
	TEST_ASSERT(new_map[keys[0]] == 5);
	TEST_ASSERT(new_map[keys[1]] == 4);
	TEST_ASSERT(new_map[keys[2]] == 3);
	TEST_ASSERT(new_map[keys[3]] == 2);
	TEST_ASSERT(new_map[keys[4]] == 1);

	// Adding by value
	TEST_EXCEPTION(w_iterator->Add(old_array));

	// Test iteration
	IReadIterator* r_iterator = RFLB_NEW_TEMP_READ_ITERATOR(factory, &new_map);
	TEST_ASSERT(r_iterator->GetCount() == 5);
	int count = 5;
	while (r_iterator->IsValid())
	{
		std::string key = *(std::string*)r_iterator->GetKey();
		int value = *(int*)r_iterator->GetValue();
		TEST_ASSERT(key == keys[5 - count]);
		TEST_ASSERT(value == count--);
		r_iterator->MoveNext();
	}

	// Get/increment overflow
	TEST_EXCEPTION(r_iterator->GetKey());
	TEST_EXCEPTION(r_iterator->GetValue());
	TEST_EXCEPTION(r_iterator->MoveNext());

	RFLB_DELETE_TEMP_ITERATOR(factory, w_iterator);
	RFLB_DELETE_TEMP_ITERATOR(factory, r_iterator);
	delete factory;
}


int main()
{
	TestArrayContainer();
	TestVectorContainer();
	TestMapContainer();

	using namespace rflb;
	TypeDatabase db;
	db.GetType<std::string>().LoadSaveBinary(LoadStringBinary, SaveStringBinary);
	db.GetType<std::string>().LoadSaveBinaryIFFv(LoadStringBinary, SaveStringBinary);

	extern void TestSerialisation(rflb::TypeDatabase& db);
	TestSerialisation(db);

	return 0;
}
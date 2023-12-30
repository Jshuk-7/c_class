#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"

#define DEBUG_BREAK(msg) __debugbreak();
#define STRINGIFY(s) #s

typedef enum MemberType {
	MEMBER_TYPE_F32,
	MEMBER_TYPE_F64,
	MEMBER_TYPE_I32,
	MEMBER_TYPE_U32,
} MemberType;

const char* member_type_to_string(MemberType type);

const char* member_type_to_string(MemberType type)
{
	switch (type)
	{
		case MEMBER_TYPE_F32: return "f32";
		case MEMBER_TYPE_F64: return "f64";
		case MEMBER_TYPE_I32: return "i32";
		case MEMBER_TYPE_U32: return "u32";
	}

	DEBUG_BREAK("unknown member type!");
	return "";
}

typedef union MemberData {
	float f_data;
	double d_data;
	int32_t i_data;
	uint32_t u_data;
} MemberData;

typedef struct Member {
	MemberData data;
	char* name;
	MemberType type;
} Member;

const char* member_get_name(const Member* member);
MemberType member_get_type(const Member* member);
MemberData member_get_data(const Member* member);

const char* member_get_name(const Member* member)
{
	if (member == NULL) {
		return NULL;
	}

	return member->name;
}

MemberType member_get_type(const Member* member)
{
	if (member == NULL) {
		return MEMBER_TYPE_I32;
	}

	return member->type;
}

MemberData member_get_data(const Member* member)
{
	if (member == NULL) {
		DEBUG_BREAK("member was null!");
	}

	return member->data;
}

typedef struct Class Class;

typedef struct Function {
	char* name;
	void (*fn) (const Class*);
} Function;

const char* function_get_name(const Function* function);

const char* function_get_name(const Function* function)
{
	if (function == NULL) {
		return NULL;
	}

	return function->name;
}

typedef struct Class {
	char* name;
	Member* members;
	size_t num_members;
	Function* functions;
	size_t num_functions;
} Class;

typedef struct ClassCreateInfo
{
	const char* name;
	const Member* members;
	size_t num_members;
	const Function* functions;
	size_t num_functions;
} ClassCreateInfo;

Class* class_create(const ClassCreateInfo* createInfo);
void class_destroy(Class* klass);
const char* class_get_name(const Class* klass);
void class_invoke_function(const Class* klass, size_t index);
void class_add_member(Class* klass, Member* member);
void class_add_function(Class* klass, Function* function);
Member* class_get_member(const Class* klass, size_t index);
Function* class_get_function(const Class* klass, size_t index);
size_t class_get_num_members(const Class* klass);
size_t class_get_num_functions(const Class* klass);
void class_print_info(const Class* klass);

Class* class_create(const ClassCreateInfo* createInfo)
{
	Class* klass = (Class*)malloc(sizeof(Class));
	if (klass == NULL) {
		return NULL;
	}
	klass->name = (char*)createInfo->name;
	klass->members = (Member*)createInfo->members;
	klass->num_members = createInfo->num_members;
	klass->functions = (Function*)createInfo->functions;
	klass->num_functions = createInfo->num_functions;
	return klass;
}

void class_destroy(Class* klass)
{
	if (klass == NULL) {
		DEBUG_BREAK("invalid class!");
		return;
	}

	const size_t num_members = class_get_num_members(klass);
	if (num_members > 0) {
		Member* members = klass->members;
		free(members);
	}

	const size_t num_functions = class_get_num_functions(klass);
	if (num_functions > 0) {
		Function* functions = klass->functions;
		free(functions);
	}

	free(klass);
}

const char* class_get_name(const Class* klass)
{
	if (klass == NULL) {
		return NULL;
	}

	return klass->name;
}

void class_invoke_function(const Class* klass, size_t index)
{
	if (klass == NULL) {
		DEBUG_BREAK("invalid class!");
		return;
	}

	const size_t num_functions = class_get_num_functions(klass);
	if (index > num_functions - 1) {
		DEBUG_BREAK("index out of bounds!");
		return;
	}

	const Function* function = class_get_function(klass, index);
	const void (*fn) (const Class*) = function->fn;
	fn(klass);
}

void memcpy(void* src, void* dest, size_t size)
{
	uint8_t* srcData = (uint8_t*)src;
	uint8_t* destData = (uint8_t*)dest;

	for (size_t i = 0; i < size; i++) {
		destData[i] = srcData[i];
	}
}

void class_add_member(Class* klass, Member* member)
{
	if (klass == NULL) {
		DEBUG_BREAK("invalid class!");
		return;
	}

	if (member == NULL) {
		DEBUG_BREAK("invalid member!");
		return;
	}

	const size_t element_size = sizeof(Member);
	const size_t num_members = class_get_num_members(klass);
	const size_t data_size = element_size * num_members;

	klass->members = realloc(klass->members, data_size + element_size);
	if (klass->members == NULL) {
		return;
	}

	klass->num_members++;

	klass->members[klass->num_members - 1] = *member;
}

void class_add_function(Class* klass, Function* function)
{
	if (klass == NULL) {
		DEBUG_BREAK("invalid class!");
		return;
	}

	if (function == NULL) {
		DEBUG_BREAK("invalid function!");
		return;
	}

	const size_t element_size = sizeof(Function);
	const size_t num_functions = class_get_num_functions(klass);
	const size_t data_size = element_size * num_functions;

	klass->functions = realloc(klass->functions, data_size + element_size);
	if (klass->functions == NULL) {
		return;
	}

	klass->num_functions++;

	klass->functions[klass->num_functions - 1] = *function;
}

Member* class_get_member(const Class* klass, size_t index)
{
	if (klass == NULL) {
		return NULL;
	}

	const size_t num_members = class_get_num_members(klass);
	if (index < num_members) {
		return &klass->members[index];
	}

	DEBUG_BREAK("index out of bounds!");
	return NULL;
}

Function* class_get_function(const Class* klass, size_t index)
{
	if (klass == NULL) {
		return NULL;
	}

	const size_t num_functions = class_get_num_functions(klass);
	if (index < num_functions) {
		return &klass->functions[index];
	}

	DEBUG_BREAK("index out of bounds!");
	return NULL;
}

size_t class_get_num_members(const Class* klass)
{
	if (klass == NULL) {
		return 0;
	}

	return klass->num_members;
}

size_t class_get_num_functions(const Class* klass)
{
	if (klass == NULL) {
		return 0;
	}

	return klass->num_functions;
}

void class_print_info(const Class* klass)
{
	if (klass == NULL) {
		return;
	}

	const char* class_name = class_get_name(klass);
	const size_t num_members = class_get_num_members(klass);
	const size_t num_functions = class_get_num_functions(klass);

	printf("Class: %s\n", class_name);
	printf("Members: %zu\n", num_members);
	printf("Functions: %zu\n", num_functions);
	
	putchar('\n');

	printf("Methods:\n");

	for (size_t i = 0; i < num_members; i++) {
		const Member* member = class_get_member(klass, i);
		const char* name = member_get_name(member);
		const MemberType member_type = member_get_type(member);
		const char* type = member_type_to_string(member_type);
		const MemberData data = member_get_data(member);
		printf("%zu.", i + 1);
		printf("\tName: %s\n", name);
		printf("\tType: %s\n", type);
		printf("\tData: ");
		switch (member_type)
		{
			case MEMBER_TYPE_F32: printf("%f", data.f_data); break;
			case MEMBER_TYPE_F64: printf("%lf", data.d_data); break;
			case MEMBER_TYPE_I32: printf("%d", data.i_data); break;
			case MEMBER_TYPE_U32: printf("%i", data.u_data); break;
		}
		putchar('\n');
	}

	putchar('\n');

	printf("Functions:\n");

	for (size_t i = 0; i < num_functions; i++) {
		const Function* function = class_get_function(klass, i);
		const char* name = function_get_name(function);
		printf("%zu.", i + 1);
		printf("\tName: %s\n", name);
	}
}

int strlen(const char* str)
{
	int pos = 0;
	while (str[pos] != '\0')
		pos++;
	return pos;
}

void my_class_method(const Class* this) {
	for (size_t i = 0; i < this->num_members; i++) {
		Member* member = class_get_member(this, i);
		printf("%s\n", member_get_name(member));
	}

	for (size_t i = 0; i < this->num_functions; i++) {
		Function* function = class_get_function(this, i);
		printf("%s\n", function_get_name(function));
	}
}

int main(int argc, const char** argv)
{
	ClassCreateInfo createInfo = {
		.name = "TestClass",
		.members = NULL,
		.num_members = 0,
		.functions = NULL,
		.num_functions = 0,
	};

	Class* klass = class_create(&createInfo);
	if (klass == NULL)
		return;

	Function* function = (Function*)malloc(sizeof(Function));
	if (function == NULL)
		return;
	function->fn = my_class_method;
	function->name = STRINGIFY(my_class_method);
	class_add_function(klass, function);

	class_print_info(klass);

	class_invoke_function(klass, 0);

	class_destroy(klass);

	int _ = getchar();
}
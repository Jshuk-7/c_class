#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"

#define DEBUG_BREAK(msg) __debugbreak();

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
		return "";
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

typedef struct Class {
	char* name;
	Member* members;
	size_t num_members;
} Class;

Class* class_new(const char* name, const Member* members, size_t num_members);
void class_free(Class* klass);
const char* class_get_name(const Class* klass);
void class_add_member(Class* klass, Member* member);
Member* class_get_member(const Class* klass, size_t index);
size_t class_get_num_members(const Class* klass);
void class_print_info(const Class* klass);

Class* class_new(const char* name, const Member* members, size_t num_members)
{
	Class* klass = (Class*)malloc(sizeof(Class));
	if (klass == NULL) {
		return NULL;
	}
	klass->name = (char*)name;
	klass->members = (Member*)members;
	klass->num_members = num_members;
	return klass;
}

void class_free(Class* klass)
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

	free(klass);
}

const char* class_get_name(const Class* klass)
{
	if (klass == NULL) {
		return NULL;
	}

	return klass->name;
}

void memcpy(void* src, void* dest, size_t size)
{
	uint8_t* srcData = (uint8_t*)src;
	uint8_t* dstData = (uint8_t*)dest;

	for (size_t i = 0; i < size; i++) {
		dstData[i] = srcData[i];
	}
}

void class_add_member(Class* klass, Member* member)
{
	if (klass == NULL || member == NULL || klass->members == NULL) {
		DEBUG_BREAK("invalid class!");
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

size_t class_get_num_members(const Class* klass)
{
	if (klass == NULL) {
		return 0;
	}

	return klass->num_members;
}

void class_print_info(const Class* klass)
{
	if (klass == NULL) {
		return;
	}

	const char* class_name = class_get_name(klass);
	printf("Class: %s\n", class_name);
	printf("Members: %zu\n", klass->num_members);

	const size_t num_members = class_get_num_members(klass);
	if (num_members == 0) {
		return;
	}

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
}

int strlen(const char* str)
{
	int pos = 0;
	while (str[pos] != '\0')
		pos++;
	return pos;
}

int main(int argc, const char** argv)
{
	const size_t num_members = 1;
	Member* members = (Member*)malloc(sizeof(Member) * num_members);
	if (members == NULL)
		return;

	Member* member = &members[0];
	member->name = "MyFloat";
	member->type = MEMBER_TYPE_F32;
	member->data.f_data = 1.0f;

	Class* klass = class_new("MyClass", members, num_members);
	if (klass == NULL)
		return;

	member = (Member*)malloc(sizeof(Member));
	if (member == NULL)
		return;
	member->name = "MyDouble";
	member->type = MEMBER_TYPE_F64;
	member->data.d_data = 2.3;
	class_add_member(klass, member);

	member = (Member*)malloc(sizeof(Member));
	if (member == NULL)
		return;
	member->name = "MyInt";
	member->type = MEMBER_TYPE_I32;
	member->data.i_data = 3;
	class_add_member(klass, member);

	class_print_info(klass);

	class_free(klass);

	int _ = getchar();
}
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
	switch (type) {
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

typedef enum FunctionType {
	FUNCTION_TYPE_CONSTRUCTOR,
	FUNCTION_TYPE_DESTRUCTOR,
	FUNCTION_TYPE_MEMBER_FUNCTION,
} FunctionType;

typedef struct Function {
	char* name;
	FunctionType type;
	void (*fn) (const Class*);
	Class* (*binary_member_fn) (const Class*, const Class*);
} Function;

const char* function_get_name(const Function* function);
FunctionType function_get_type(const Function* function);
Class* function_invoke(const Function* function, const Class* klass, const Class* other);

const char* function_get_name(const Function* function)
{
	if (function == NULL) {
		return NULL;
	}

	return function->name;
}

FunctionType function_get_type(const Function* function)
{
	if (function == NULL) {
		return FUNCTION_TYPE_MEMBER_FUNCTION;
	}

	return function->type;
}

Class* function_invoke(const Function* function, const Class* klass, const Class* other)
{
	if (function == NULL || klass == NULL) {
		DEBUG_BREAK("invalid function!");
		return NULL;
	}

	const FunctionType type = function_get_type(function);

	switch (type) {
		case FUNCTION_TYPE_CONSTRUCTOR: function->fn(klass); return NULL;
		case FUNCTION_TYPE_DESTRUCTOR: function->fn(klass); return NULL;
		case FUNCTION_TYPE_MEMBER_FUNCTION: {
			if (other == NULL) {
				function->fn(klass);
				return NULL;
			} else {
				return function->binary_member_fn(klass, other);
			}
		}
	}

	DEBUG_BREAK("unknown function type!");
	return NULL;
}

typedef struct Class {
	char* name;
	Function* ctor;
	Function* dtor;
	Member* members;
	size_t num_members;
	Function* functions;
	size_t num_functions;
} Class;

typedef struct ClassCreateInfo {
	const char* name;
	Function* ctor;
	Function* dtor;
	const Member* members;
	size_t num_members;
	const Function* functions;
	size_t num_functions;
} ClassCreateInfo;

Class* class_create(const ClassCreateInfo* createInfo);
void class_destroy(Class* klass);
const char* class_get_name(const Class* klass);
int class_has_constructor(const Class* klass);
int class_has_destructor(const Class* klass);
Function* class_get_constructor(const Class* klass);
Function* class_get_destructor(const Class* klass);
Class* class_invoke_function(const Class* klass, const Class* other, size_t index);
void class_add_member(Class* klass, Member* member);
void class_add_function(Class* klass, Function* function);
Member* class_get_member(const Class* klass, size_t index);
Function* class_get_function(const Class* klass, size_t index);
size_t class_get_num_members(const Class* klass);
size_t class_get_num_functions(const Class* klass);
void class_debug_print(const Class* klass);

Class* class_create(const ClassCreateInfo* createInfo)
{
	Class* klass = (Class*)malloc(sizeof(Class));
	if (klass == NULL) {
		return NULL;
	}

	klass->name = (char*)createInfo->name;

	if (createInfo->ctor != NULL) {
		klass->ctor = (Function*)malloc(sizeof(Function));
		if (klass->ctor == NULL) {
			return NULL;
		}
		klass->ctor->fn = createInfo->ctor->fn;
		klass->ctor->name = createInfo->ctor->name;
	} else {
		klass->ctor = NULL;
	}

	if (createInfo->dtor != NULL) {
		klass->dtor = (Function*)malloc(sizeof(Function));
		if (klass->dtor == NULL) {
			return NULL;
		}
		klass->dtor->fn = createInfo->dtor->fn;
		klass->dtor->name = createInfo->dtor->name;
	} else {
		klass->dtor = NULL;
	}

	klass->members = (Member*)malloc(sizeof(Member) * createInfo->num_members);
	if (klass->members == NULL) {
		return NULL;
	}

	klass->num_members = createInfo->num_members;
	for (size_t i = 0; i < klass->num_members; i++) {
		Member* member = &klass->members[i];
		const Member* other = &createInfo->members[i];
		member->name = other->name;
		member->type = other->type;
		member->data = other->data;
	}

	klass->functions = (Function*)malloc(sizeof(Function) * createInfo->num_functions);
	if (klass->functions == NULL) {
		return NULL;
	}

	klass->num_functions = createInfo->num_functions;
	for (size_t i = 0; i < klass->num_functions; i++) {
		Function* function = &klass->functions[i];
		const Function* other = &createInfo->functions[i];
		function->fn = other->fn;
		function->binary_member_fn = other->binary_member_fn;
		function->name = other->name;
		function->type = other->type;
	}

	if (class_has_constructor(klass) == 1) {
		klass->ctor->type = FUNCTION_TYPE_CONSTRUCTOR;
		function_invoke(klass->ctor, klass, NULL);
	}

	return klass;
}

void class_destroy(Class* klass)
{
	if (klass == NULL) {
		DEBUG_BREAK("invalid class!");
		return;
	}

	if (class_has_constructor(klass) == 1) {
		free(klass->ctor);
	}

	if (class_has_destructor(klass) == 1) {
		klass->dtor->type = FUNCTION_TYPE_DESTRUCTOR;
		function_invoke(klass->dtor, klass, NULL);
		free(klass->dtor);
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

int class_has_constructor(const Class* klass)
{
	if (klass == NULL) {
		return 0;
	}

	return klass->ctor != NULL;
}

int class_has_destructor(const Class* klass)
{
	if (klass == NULL) {
		return 0;
	}

	return klass->dtor != NULL;
}

Function* class_get_constructor(const Class* klass)
{
	if (klass == NULL) {
		return NULL;
	}

	return klass->ctor;
}

Function* class_get_destructor(const Class* klass)
{
	if (klass == NULL) {
		return NULL;
	}

	return klass->dtor;
}

Class* class_invoke_function(const Class* klass, const Class* other, size_t index)
{
	if (klass == NULL) {
		DEBUG_BREAK("invalid class!");
		return NULL;
	}

	const size_t num_functions = class_get_num_functions(klass);
	if (index > num_functions - 1) {
		DEBUG_BREAK("index out of bounds!");
		return NULL;
	}

	const Function* function = class_get_function(klass, index);
	return function_invoke(function, klass, other);
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

void class_debug_print(const Class* klass)
{
	if (klass == NULL) {
		return;
	}

	const char* class_name = class_get_name(klass);
	const size_t num_members = class_get_num_members(klass);
	const size_t num_functions = class_get_num_functions(klass);

	printf("Class: %s\n", class_name);
	printf("Data Members: %zu\n", num_members);
	printf("Member Functions: %zu\n", num_functions);

	printf("Ctor: ");
	if (class_has_constructor(klass) == 1) {
		const Function* ctor = class_get_constructor(klass);
		const char* ctor_name = function_get_name(ctor);
		printf("%s", ctor_name);
	} else {
		printf("(null)");
	}
	putchar('\n');

	printf("Dtor: ");
	if (class_has_destructor(klass) == 1) {
		const Function* dtor = class_get_destructor(klass);
		const char* dtor_name = function_get_name(dtor);
		printf("%s", dtor_name);
	} else {
		printf("(null)");
	}
	putchar('\n');
	
	putchar('\n');

	printf("Data Members: ");
	if (num_members == 0) {
		printf("(null)");
	}
	putchar('\n');

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
		switch (member_type) {
			case MEMBER_TYPE_F32: printf("%f", data.f_data); break;
			case MEMBER_TYPE_F64: printf("%lf", data.d_data); break;
			case MEMBER_TYPE_I32: printf("%d", data.i_data); break;
			case MEMBER_TYPE_U32: printf("%i", data.u_data); break;
		}
		putchar('\n');
	}

	putchar('\n');

	printf("Member Functions: ");
	if (num_functions == 0) {
		printf("(null)");
	}
	putchar('\n');

	for (size_t i = 0; i < num_functions; i++) {
		const Function* function = class_get_function(klass, i);
		const char* name = function_get_name(function);
		printf("%zu.", i + 1);
		printf("\tName: %s\n", name);
	}
}

void test_class_test(void) {
	ClassCreateInfo createInfo = { .name = "TestClass" };
	Class* klass = class_create(&createInfo);
	if (klass == NULL)
		return;
	
	class_debug_print(klass);
	class_destroy(klass);
}

void vec2_ctor(const Class* this) {
	
}

void vec2_dtor(const Class* this) {
	
}

Class* vec2_add(const Class* lhs, const Class* rhs);

Class* create_vec2(float x, float y) {
	Function ctor;
	ctor.fn = vec2_ctor;
	ctor.name = STRINGIFY(vec2_ctor);

	Function dtor;
	dtor.fn = vec2_dtor;
	dtor.name = STRINGIFY(vec2_dtor);

	Member x_member, y_member;
	x_member.name = "x";
	x_member.data.f_data = x;
	y_member.name = "y";
	y_member.data.f_data = y;
	Member members[] = { x_member, y_member };

	const size_t num_members = sizeof(members) / sizeof(members[0]);
	for (size_t i = 0; i < num_members; i++) {
		Member* member = &members[i];
		member->type = MEMBER_TYPE_F32;
	}

	Function add_fn;
	add_fn.type = FUNCTION_TYPE_MEMBER_FUNCTION;
	add_fn.binary_member_fn = vec2_add;
	add_fn.name = STRINGIFY(vec2_add);

	ClassCreateInfo createInfo = {
		.name = "Vec2",
		.ctor = &ctor,
		.dtor = &dtor,
		.members = members,
		.num_members = num_members,
		.functions = &add_fn,
		.num_functions = 1
	};

	Class* klass = class_create(&createInfo);
	if (klass == NULL)
		return NULL;

	return klass;
}

Class* vec2_add(const Class* lhs, const Class* rhs) {
	Class* result = create_vec2(0, 0);
	result->members[0].data.f_data = lhs->members[0].data.f_data + rhs->members[0].data.f_data;
	result->members[1].data.f_data = lhs->members[1].data.f_data + rhs->members[1].data.f_data;
	return result;
}

void test_vec2_class(void) {
	Class* a = create_vec2(1, 3);
	Class* b = create_vec2(2, 4);
	Class* c = class_invoke_function(a, b, 0);
	class_debug_print(c);
	Class* classes[] = { a, b, c };
	const size_t num_classes = sizeof(classes) / sizeof(classes[0]);
	for (size_t i = 0; i < num_classes; i++) {
		Class* klass = classes[i];
		class_destroy(klass);
	}
}

int main(int argc, char** argv) {
	test_class_test();

	return getchar();
}
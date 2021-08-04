#pragma once

#include "alice/core.h"
#include "alice/resource.h"

typedef enum alice_dtable_type_t {
	ALICE_DTABLE_NUMBER,
	ALICE_DTABLE_STRING,
	ALICE_DTABLE_BOOL,
	ALICE_DTABLE_EMPTY,
	ALICE_DTABLE_ARRAY
} alice_dtable_type_t;

typedef struct alice_dtable_value_array_t alice_dtable_value_array_t;

typedef struct alice_dtable_value_t {
	alice_dtable_type_t type;
	union {
		double number;
		bool boolean;
		char* string;
		alice_dtable_value_array_t* array;
	} as;
} alice_dtable_value_t;

ALICE_API void alice_free_dtable_value(alice_dtable_value_t* value);

typedef struct alice_dtable_value_array_t {
	alice_dtable_value_t* values;
	u32 count;
	u32 capacity;
} alice_dtable_value_array_t;

ALICE_API alice_dtable_value_array_t* alice_new_dtable_value_array();
ALICE_API void alice_free_dtable_value_array(alice_dtable_value_array_t* array);
ALICE_API void alice_dtable_value_array_add(alice_dtable_value_array_t* array, alice_dtable_value_t value);

typedef struct alice_dtable_t alice_dtable_t;

typedef struct alice_dtable_t {
	char* name;
	u32 name_hash;

	alice_dtable_value_t value;

	alice_dtable_t* children;
	u32 child_count;
	u32 child_capacity;
} alice_dtable_t;

ALICE_API alice_dtable_t alice_new_number_dtable(const char* name, double value);
ALICE_API alice_dtable_t alice_new_bool_dtable(const char* name, bool value);
ALICE_API alice_dtable_t alice_new_string_dtable(const char* name, const char* value);
ALICE_API alice_dtable_t alice_new_empty_dtable(const char* name);
ALICE_API alice_dtable_t alice_new_array_dtable(const char* name, alice_dtable_value_array_t* array);

ALICE_API alice_dtable_t* alice_dtable_find_child(alice_dtable_t* table, const char* name);

ALICE_API alice_dtable_t* alice_read_dtable(alice_resource_t* string);
ALICE_API void alice_write_dtable(alice_dtable_t* table, const char* file_path);
ALICE_API void alice_deinit_dtable(alice_dtable_t* table);
ALICE_API void alice_free_dtable(alice_dtable_t* table);

ALICE_API void alice_dtable_add_child(alice_dtable_t* table, alice_dtable_t child);
ALICE_API void alice_print_dtable(alice_dtable_t* table);

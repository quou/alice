#pragma once

#include "alice/core.h"
#include "alice/resource.h"

typedef enum alice_DTableType {
	ALICE_DTABLE_NUMBER,
	ALICE_DTABLE_STRING,
	ALICE_DTABLE_BOOL,
	ALICE_DTABLE_EMPTY,
	ALICE_DTABLE_ARRAY
} alice_DTableType;

typedef struct alice_DTableValueArray alice_DTableValueArray;

typedef struct alice_DTableValue {
	alice_DTableType type;
	union {
		double number;
		bool boolean;
		char* string;
		alice_DTableValueArray* array;
	} as;
} alice_DTableValue;

ALICE_API void alice_free_dtable_value(alice_DTableValue* value);

typedef struct alice_DTableValueArray {
	alice_DTableValue* values;
	u32 count;
	u32 capacity;
} alice_DTableValueArray;

ALICE_API alice_DTableValueArray* alice_new_dtable_value_array();
ALICE_API void alice_free_dtable_value_array(alice_DTableValueArray* array);
ALICE_API void alice_dtable_value_array_add(alice_DTableValueArray* array, alice_DTableValue value);

typedef struct alice_DTable alice_DTable;

typedef struct alice_DTable {
	char* name;
	u32 name_hash;

	alice_DTableValue value;

	alice_DTable* children;
	u32 child_count;
	u32 child_capacity;
} alice_DTable;

ALICE_API alice_DTable alice_new_number_dtable(const char* name, double value);
ALICE_API alice_DTable alice_new_bool_dtable(const char* name, bool value);
ALICE_API alice_DTable alice_new_string_dtable(const char* name, const char* value);
ALICE_API alice_DTable alice_new_empty_dtable(const char* name);
ALICE_API alice_DTable alice_new_array_dtable(const char* name, alice_DTableValueArray* array);

ALICE_API alice_DTable* alice_dtable_find_child(alice_DTable* table, const char* name);

ALICE_API alice_DTable* alice_read_dtable(alice_Resource* string);
ALICE_API void alice_write_dtable(alice_DTable* table, const char* file_path);
ALICE_API void alice_deinit_dtable(alice_DTable* table);
ALICE_API void alice_free_dtable(alice_DTable* table);

ALICE_API void alice_dtable_add_child(alice_DTable* table, alice_DTable child);
ALICE_API void alice_print_dtable(alice_DTable* table);

#pragma once

#include "alice/core.h"

typedef struct alice_hashtable_t alice_hashtable_t;
typedef struct alice_hashtable_iter_t alice_hashtable_iter_t;

typedef u32 (*alice_hashtable_hash_t)(void* in, u32 seed);
typedef void* (*alice_hashtable_key_copy_t)(void* in);
typedef bool (*alice_hashtable_key_equal_t)(void* a, void* b);
typedef void (*alice_hashtable_key_free_t)(void* in);
typedef void* (*alice_hashtable_value_copy_t)(void* in);
typedef void (*alice_hashtable_value_free_t)(void* in);

typedef struct alice_hashtable_callbacks_t {
	alice_hashtable_key_copy_t key_copy;
	alice_hashtable_key_free_t key_free;
	alice_hashtable_value_copy_t value_copy;
	alice_hashtable_value_free_t value_free;
} alice_hashtable_callbacks_t;

alice_hashtable_t* alice_new_hashtable(alice_hashtable_hash_t hash_func,
		alice_hashtable_key_equal_t key_equal_func, alice_hashtable_callbacks_t* callbacks);
void alice_free_hashtable(alice_hashtable_t* hashtable);

void alice_hashtable_set(alice_hashtable_t* hashtable, void* key, void* value);
void alice_hashtable_remove(alice_hashtable_t* hashtable, void* key);

bool alice_hashtable_get(alice_hashtable_t* hashtable, void* key, void** value);
void* alice_hashtable_get_direct(alice_hashtable_t* hashtable, void* key);

alice_hashtable_iter_t* alice_new_hashtable_iter(alice_hashtable_t* hashtable);
bool alice_hashtable_iter_next(alice_hashtable_iter_t* iter, void** key, void** value);
void alice_free_hashtable_iter(alice_hashtable_iter_t* iter);

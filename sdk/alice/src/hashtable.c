#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "alice/hashtable.h"

typedef struct alice_hashtable_bucket_t {
	void* key;
	void* value;
	struct alice_hashtable_bucket_t* next;
} alice_hashtable_bucket_t;

struct alice_hashtable_t {
	alice_hashtable_hash_t hash_func;
	alice_hashtable_key_equal_t key_equal_func;
	alice_hashtable_callbacks_t callbacks;
	alice_hashtable_bucket_t* buckets;
	u32 capacity;
	u32 count;
	u32 seed;
};

struct alice_hashtable_iter_t {
	alice_hashtable_t* hashtable;
	alice_hashtable_bucket_t* current;
	u32 index;
};

static void* alice_hashtable_default_copy(void* value) {
	return value;
}

static void alice_hashtable_default_free(void* value) {
	return;
}

static u32 alice_get_hashtable_bucket_index(alice_hashtable_t* hashtable, void* key) {
	return hashtable->hash_func(key, hashtable->seed) % hashtable->capacity;
}

static void alice_hashtable_add_to_bucket(alice_hashtable_t* hashtable, void* key, void* value, bool rehash) {
	const u32 index = alice_get_hashtable_bucket_index(hashtable, key);

	if (!hashtable->buckets[index].key) {
		if (!rehash) {
			key = hashtable->callbacks.key_copy(key);
			if (value) {
				value = hashtable->callbacks.value_copy(value);
			}

		}
		hashtable->buckets[index].key = key;
		hashtable->buckets[index].value = value;
		if (!rehash) {
			hashtable->count++;
		}
	} else {
		alice_hashtable_bucket_t* last = hashtable->buckets + index;
		alice_hashtable_bucket_t* current = hashtable->buckets + index;
		do {
			if (hashtable->key_equal_func(key, current->key)) {
				if (current->value) {
					hashtable->callbacks.value_free(current->value);
				}
				if (!rehash && value) {
					value = hashtable->callbacks.value_copy(value);
				}

				current->value = value;
				last = alice_null;
				break;
			}

			last = current;
			current = current->next;
		} while (current);

		if (last) {
			current = malloc(sizeof(alice_hashtable_bucket_t));
			current->next = alice_null;
			if (!rehash) {
				key = hashtable->callbacks.key_copy(value);
				if (value) {
					value = hashtable->callbacks.value_copy(value);
				}
			}

			current->key = key;
			current->value = value;
			last->next = current;
			if (!rehash) {
				hashtable->count++;
			}
		}
	}
}

static void alice_hashtable_rehash(alice_hashtable_t* hashtable) {
	if (
			hashtable->count + 1 < (u32)(hashtable->capacity * 0.75) ||
			hashtable->capacity >= 1 << 31) {
		return;
	}

	u32 capacity = hashtable->capacity;
	alice_hashtable_bucket_t* buckets = hashtable->buckets;
	hashtable->capacity <<= 1;
	hashtable->buckets = calloc(hashtable->capacity, sizeof(alice_hashtable_bucket_t));

	for (u32 i = 0; i < capacity; i++) {
		if (!buckets[i].key) {
			continue;
		}

		alice_hashtable_add_to_bucket(hashtable, buckets[i].key, buckets[i].value, true);
		if (buckets[i].next) {
			alice_hashtable_bucket_t* current = buckets[i].next;
			do {
				alice_hashtable_add_to_bucket(hashtable, current->key, current->value, true);
				alice_hashtable_bucket_t* next = current->next;
				free(current);
				current = next;
			} while (current);
		}
	}

	free(buckets);
}

alice_hashtable_t* alice_new_hashtable(alice_hashtable_hash_t hash_func,
		alice_hashtable_key_equal_t key_equal_func, alice_hashtable_callbacks_t* callbacks) {
	assert(hash_func);
	assert(key_equal_func);

	alice_hashtable_t* hashtable = malloc(sizeof(alice_hashtable_t));

	hashtable->hash_func = hash_func;
	hashtable->key_equal_func = key_equal_func;

	hashtable->callbacks.key_copy = alice_hashtable_default_copy;
	hashtable->callbacks.key_free = alice_hashtable_default_free;
	hashtable->callbacks.value_copy = alice_hashtable_default_copy;
	hashtable->callbacks.value_free = alice_hashtable_default_free;

	if (callbacks != alice_null) {
		if (callbacks->key_copy) { hashtable->callbacks.key_copy = callbacks->key_copy; }
		if (callbacks->key_copy) { hashtable->callbacks.key_free = callbacks->key_free; }
		if (callbacks->key_copy) { hashtable->callbacks.value_copy = callbacks->value_copy; }
		if (callbacks->key_copy) { hashtable->callbacks.value_free = callbacks->value_free; }
	}

	hashtable->count = 0;
	hashtable->capacity = 16;
	hashtable->buckets = calloc(16, sizeof(alice_hashtable_bucket_t));

	hashtable->seed = (u32)(u64)time(alice_null);
	hashtable->seed ^= (u32)((u64)alice_new_hashtable << 16) | (u64)hashtable;
	hashtable->seed ^= (u32)(u64)&hashtable->seed;

	return hashtable;
}

void alice_free_hashtable(alice_hashtable_t* hashtable) {
	assert(hashtable);

	for (u32 i = 0; i < hashtable->capacity; i++) {
		if (!hashtable->buckets[i].key) {
			continue;
		}

		hashtable->callbacks.key_free(hashtable->buckets[i].key);
		hashtable->callbacks.value_free(hashtable->buckets[i].value);

		alice_hashtable_bucket_t* next = hashtable->buckets[i].next;
		while (next) {
			alice_hashtable_bucket_t* current = next;
			hashtable->callbacks.key_free(current->key);
			hashtable->callbacks.value_free(current->value);
			next = current->next;
			free(current);
		}
	}

	free(hashtable->buckets);
	free(hashtable);
}

void alice_hashtable_set(alice_hashtable_t* hashtable, void* key, void* value) {
	assert(hashtable);
	assert(key);

	alice_hashtable_rehash(hashtable);
	alice_hashtable_add_to_bucket(hashtable, key, value, false);
}

void alice_hashtable_remove(alice_hashtable_t* hashtable, void* key) {
	assert(hashtable);
	assert(key);

	u32 index = alice_get_hashtable_bucket_index(hashtable, key);
	if (!hashtable->buckets[index].key) {
		return;
	}

	alice_hashtable_bucket_t* current;

	if (hashtable->key_equal_func(hashtable->buckets[index].key, key)) {
		hashtable->callbacks.key_free(hashtable->buckets[index].key);
		hashtable->callbacks.value_free(hashtable->buckets[index].value);
		hashtable->buckets[index].key = alice_null;

		current = hashtable->buckets[index].next;
		if (current) {
			hashtable->buckets[index].key = current->key;
			hashtable->buckets[index].value = current->value;
			hashtable->buckets[index].next = current->next;
			free(current);
		}

		hashtable->count--;
		return;
	}

	alice_hashtable_bucket_t* last = hashtable->buckets + index;
	current = last->next;
	while (current) {
		if (hashtable->key_equal_func(key, current->key)) {
			last->next = current->next;
			hashtable->callbacks.key_free(current->key);
			hashtable->callbacks.value_free(current->value);
			free(current);
			hashtable->count--;
			break;
		}
		last = current;
		current = current->next;
	}
}

bool alice_hashtable_get(alice_hashtable_t* hashtable, void* key, void** value) {
	assert(hashtable);
	assert(key);
	
	const u32 index = alice_get_hashtable_bucket_index(hashtable, key);
	if (!hashtable->buckets[index].key) {
		return false;
	}

	alice_hashtable_bucket_t* current = hashtable->buckets + index;

	while (current) {
		if (hashtable->key_equal_func(key, current->key)) {
			if (value != alice_null) {
				*value = current->value;
			}
			return true;
		}
		current = current->next;
	}

	return false;
}

void* alice_hashtable_get_direct(alice_hashtable_t* hashtable, void* key) {
	assert(hashtable);
	assert(key);

	void* value = alice_null;

	alice_hashtable_get(hashtable, key, &value);

	return value;
}

alice_hashtable_iter_t* alice_new_hashtable_iter(alice_hashtable_t* hashtable) {
	alice_hashtable_iter_t* iter = malloc(sizeof(alice_hashtable_iter_t));
	iter->hashtable = hashtable;
	iter->current = alice_null;
	iter->index = 0;

	return iter;
}

bool alice_hashtable_iter_next(alice_hashtable_iter_t* iter, void** key, void** value) {
	assert(iter);

	if (iter->index >= iter->hashtable->capacity) {
		return false;
	}

	void* mykey;
	void* myvalue;

	if (key == alice_null) {
		key = &mykey;
	}
	if (value == alice_null) {
		value = &myvalue;
	}

	if (!iter->current) {
		while (iter->index < iter->hashtable->capacity && iter->hashtable->buckets[iter->index].key) {
			iter->index++;
		}
		if (iter->index >= iter->hashtable->capacity) {
			return false;
		}
		iter->current = iter->hashtable->buckets + iter->index;
		iter->index++;
	}

	*key = iter->current->key;
	*value = iter->current->value;
	iter->current = iter->current->next;

	return true;
}

void alice_free_hashtable_iter(alice_hashtable_iter_t* iter) {
	assert(iter);

	free(iter);
}

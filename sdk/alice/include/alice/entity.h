#pragma once

#include "alice/core.h"
#include "alice/entity.h"
#include "alice/maths.h"

typedef struct alice_Scene alice_Scene;
typedef struct alice_Script alice_Script;
typedef struct alice_ScriptContext alice_ScriptContext;

typedef u64 alice_EntityHandle;

typedef struct alice_Entity {
	char* name;

	alice_v3f position;
	alice_v3f rotation;
	alice_v3f scale;

	alice_Script* script;

	alice_EntityHandle parent;
	alice_EntityHandle* children;
	u32 child_count;
	u32 child_capacity;
} alice_Entity;

ALICE_API alice_m4f alice_get_entity_transform(alice_Scene* scne, alice_Entity* entity);
ALICE_API void alice_entity_parent_to(alice_Scene* scene, alice_EntityHandle entity, alice_EntityHandle parent);
ALICE_API void alice_entity_add_child(alice_Scene* scene, alice_EntityHandle entity, alice_EntityHandle child);
ALICE_API void alice_entity_remove_child(alice_Scene* scene, alice_EntityHandle entity, alice_EntityHandle child);
ALICE_API void alice_entity_unparent(alice_Scene* scene, alice_EntityHandle entity);

ALICE_API alice_EntityHandle alice_find_entity_by_name(alice_Scene* scene, alice_EntityHandle parent_handle, const char* name);
ALICE_API alice_EntityHandle alice_find_entity_by_path(alice_Scene* scene, const char* path);

ALICE_API alice_v3f alice_get_entity_world_position(alice_Scene* scene, alice_Entity* entity);
ALICE_API alice_v3f alice_get_entity_world_rotation(alice_Scene* scene, alice_Entity* entity);
ALICE_API alice_v3f alice_get_entity_world_scale(alice_Scene* scene, alice_Entity* entity);

static const alice_EntityHandle alice_null_entity_handle =
		((alice_EntityHandle)UINT32_MAX << 32) | ((alice_EntityHandle)UINT32_MAX);

ALICE_API alice_EntityHandle alice_new_entity_handle(u32 id, u32 type_id);
ALICE_API u32 alice_get_entity_handle_type(alice_EntityHandle handle);
ALICE_API u32 alice_get_entity_handle_id(alice_EntityHandle handle);

typedef void (*alice_EntityCreateFunction)(alice_Scene* scene, alice_EntityHandle handle, void* ptr);
typedef void (*alice_EntityDestroyFunction)(alice_Scene* scene, alice_EntityHandle handle, void* ptr);

typedef struct alice_EntityPool {
	alice_EntityCreateFunction create;
	alice_EntityDestroyFunction destroy;

	u32 type_id;
	u32 element_size;

	void* data;
	u32 count;
	u32 capacity;
} alice_EntityPool;

ALICE_API void alice_init_entity_pool(alice_EntityPool* pool, u32 type_id, u32 element_size);
ALICE_API void alice_deinit_entity_pool(alice_EntityPool* pool);
ALICE_API u32 alice_entity_pool_add(alice_EntityPool* pool);
ALICE_API void alice_entity_pool_remove(alice_EntityPool* pool, u32 index);
ALICE_API void* alice_entity_pool_get(alice_EntityPool* pool, u32 index);

struct alice_Scene {
	alice_EntityPool* pools;
	u32 pool_count;
	u32 pool_capacity;

	alice_ScriptContext* script_context;
};

#define alice_register_entity_type(s_, t_) \
	impl_alice_register_entity_type((s_), alice_get_type_info(t_))

#define alice_new_entity(s_, t_) \
	impl_alice_new_entity((s_), alice_get_type_info(t_))

#define alice_set_entity_create_function(s_, t_, f_) \
	impl_alice_set_entity_create_function((s_), alice_get_type_info(t_), f_)

#define alice_set_entity_destroy_function(s_, t_, f_) \
	impl_alice_set_entity_destroy_function((s_), alice_get_type_info(t_), f_)

ALICE_API alice_Scene* alice_new_scene(const char* script_assembly);
ALICE_API void alice_free_scene(alice_Scene* scene);

ALICE_API alice_EntityPool* alice_get_entity_pool(alice_Scene* scene, u32 type_id);
ALICE_API void impl_alice_register_entity_type(alice_Scene* scene, alice_TypeInfo type);

ALICE_API alice_EntityHandle impl_alice_new_entity(alice_Scene* scene, alice_TypeInfo type);
ALICE_API void alice_destroy_entity(alice_Scene* scene, alice_EntityHandle handle);
ALICE_API void* alice_get_entity_ptr(alice_Scene* scene, alice_EntityHandle handle);

ALICE_API void impl_alice_set_entity_create_function(alice_Scene* scene,
		alice_TypeInfo type, alice_EntityCreateFunction function);
ALICE_API void impl_alice_set_entity_destroy_function(alice_Scene* scene,
		alice_TypeInfo type, alice_EntityDestroyFunction function);

#define alice_entity_iter(s_, n_, t_) \
	alice_EntityIter n_ = impl_alice_new_entity_iter((s_), alice_get_type_info(t_)); \
	alice_entity_iter_valid(&(n_)); \
	alice_entity_iter_next(&(n_))

typedef struct alice_EntityIter {
	u32 index;

	alice_EntityPool* pool;

	alice_Scene* scene;

	alice_TypeInfo type;

	alice_EntityHandle current;
	void* current_ptr;
} alice_EntityIter;

ALICE_API alice_EntityIter impl_alice_new_entity_iter(alice_Scene* scene, alice_TypeInfo type);
ALICE_API void alice_entity_iter_next(alice_EntityIter* iter);
ALICE_API bool alice_entity_iter_valid(alice_EntityIter* iter);

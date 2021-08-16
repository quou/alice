#pragma once

#include "alice/core.h"
#include "alice/entity.h"
#include "alice/maths.h"

typedef struct alice_scene_t alice_scene_t;
typedef struct alice_script_t alice_script_t;
typedef struct alice_script_context_t alice_script_context_t;
typedef struct alice_scene_renderer_3d_t alice_scene_renderer_3d_t;
typedef struct alice_scene_renderer_2d_t alice_scene_renderer_2d_t;
typedef struct alice_physics_engine_t alice_physics_engine_t;

typedef u64 alice_entity_handle_t;

typedef struct alice_entity_t {
	char* name;

	alice_v3f_t position;
	alice_v3f_t rotation;
	alice_v3f_t scale;

	alice_m4f_t transform;

	alice_script_t* script;

	alice_entity_handle_t parent;
	alice_entity_handle_t* children;
	u32 child_count;
	u32 child_capacity;
} alice_entity_t;

ALICE_API alice_m4f_t alice_get_entity_transform(alice_scene_t* scene, alice_entity_t* entity);
ALICE_API void alice_compute_scene_transforms(alice_scene_t* scene);
ALICE_API void alice_entity_parent_to(alice_scene_t* scene, alice_entity_handle_t entity, alice_entity_handle_t parent);
ALICE_API void alice_entity_add_child(alice_scene_t* scene, alice_entity_handle_t entity, alice_entity_handle_t child);
ALICE_API void alice_entity_remove_child(alice_scene_t* scene, alice_entity_handle_t entity, alice_entity_handle_t child);
ALICE_API void alice_entity_unparent(alice_scene_t* scene, alice_entity_handle_t entity);

ALICE_API alice_entity_handle_t alice_find_entity_by_name(alice_scene_t* scene, alice_entity_handle_t parent_handle, const char* name);
ALICE_API alice_entity_handle_t alice_find_entity_by_path(alice_scene_t* scene, const char* path);

ALICE_API alice_v3f_t alice_get_entity_world_position(alice_scene_t* scene, alice_entity_t* entity);
ALICE_API alice_v3f_t alice_get_entity_world_rotation(alice_scene_t* scene, alice_entity_t* entity);
ALICE_API alice_v3f_t alice_get_entity_world_scale(alice_scene_t* scene, alice_entity_t* entity);

static const alice_entity_handle_t alice_null_entity_handle =
		((alice_entity_handle_t)UINT32_MAX << 32) | ((alice_entity_handle_t)UINT32_MAX);

ALICE_API alice_entity_handle_t alice_new_entity_handle(u32 id, u32 type_id);
ALICE_API u32 alice_get_entity_handle_type(alice_entity_handle_t handle);
ALICE_API u32 alice_get_entity_handle_id(alice_entity_handle_t handle);

typedef void (*alice_entity_create_f)(alice_scene_t* scene, alice_entity_handle_t handle, void* ptr);
typedef void (*alice_entity_destroy_f)(alice_scene_t* scene, alice_entity_handle_t handle, void* ptr);

typedef struct alice_entity_pool_t {
	alice_entity_create_f create;
	alice_entity_destroy_f destroy;

	u32 type_id;
	u32 element_size;

	void* data;
	u32 count;
	u32 capacity;
} alice_entity_pool_t;

ALICE_API void alice_init_entity_pool(alice_entity_pool_t* pool, u32 type_id, u32 element_size);
ALICE_API void alice_deinit_entity_pool(alice_entity_pool_t* pool);
ALICE_API u32 alice_entity_pool_add(alice_entity_pool_t* pool);
ALICE_API void alice_entity_pool_remove(alice_entity_pool_t* pool, u32 index);
ALICE_API void* alice_entity_pool_get(alice_entity_pool_t* pool, u32 index);

struct alice_scene_t {
	alice_entity_pool_t* pools;
	u32 pool_count;
	u32 pool_capacity;

	alice_script_context_t* script_context;

	alice_scene_renderer_3d_t* renderer;
	alice_scene_renderer_2d_t* renderer_2d;
	alice_physics_engine_t* physics_engine;
};

#define alice_register_entity_type(s_, t_) \
	impl_alice_register_entity_type((s_), alice_get_type_info(t_))

#define alice_new_entity(s_, t_) \
	impl_alice_new_entity((s_), alice_get_type_info(t_))

#define alice_set_entity_create_function(s_, t_, f_) \
	impl_alice_set_entity_create_function((s_), alice_get_type_info(t_), f_)

#define alice_set_entity_destroy_function(s_, t_, f_) \
	impl_alice_set_entity_destroy_function((s_), alice_get_type_info(t_), f_)

ALICE_API alice_scene_t* alice_new_scene(const char* script_assembly);
ALICE_API void alice_free_scene(alice_scene_t* scene);

ALICE_API alice_entity_pool_t* alice_get_entity_pool(alice_scene_t* scene, u32 type_id);
ALICE_API void impl_alice_register_entity_type(alice_scene_t* scene, alice_type_info_t type);

ALICE_API alice_entity_handle_t impl_alice_new_entity(alice_scene_t* scene, alice_type_info_t type);
ALICE_API void alice_destroy_entity(alice_scene_t* scene, alice_entity_handle_t handle);
ALICE_API void* alice_get_entity_ptr(alice_scene_t* scene, alice_entity_handle_t handle);

ALICE_API void impl_alice_set_entity_create_function(alice_scene_t* scene,
		alice_type_info_t type, alice_entity_create_f function);
ALICE_API void impl_alice_set_entity_destroy_function(alice_scene_t* scene,
		alice_type_info_t type, alice_entity_destroy_f function);

#define alice_entity_iter(s_, n_, t_) \
	alice_entity_iter_t n_ = impl_alice_new_entity_iter((s_), alice_get_type_info(t_)); \
	alice_entity_iter_valid(&(n_)); \
	alice_entity_iter_next(&(n_))

typedef struct alice_entity_iter_t {
	u32 index;

	alice_entity_pool_t* pool;

	alice_scene_t* scene;

	alice_type_info_t type;

	alice_entity_handle_t current;
	void* current_ptr;
} alice_entity_iter_t;

ALICE_API alice_entity_iter_t impl_alice_new_entity_iter(alice_scene_t* scene, alice_type_info_t type);
ALICE_API void alice_entity_iter_next(alice_entity_iter_t* iter);
ALICE_API bool alice_entity_iter_valid(alice_entity_iter_t* iter);

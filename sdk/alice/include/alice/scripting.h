#pragma once

#include "alice/core.h"
#include "alice/entity.h"

typedef u32 (*alice_script_get_instance_size_f)();
typedef void (*alice_script_init_f)(alice_scene_t* scene, alice_entity_handle_t entity, void*);
typedef void (*alice_script_update_f)(alice_scene_t* scene, alice_entity_handle_t entity, void*, double);
typedef void (*alice_script_physics_update_f)(alice_scene_t* scene, alice_entity_handle_t entity, void*, double);
typedef void (*alice_script_free_f)(alice_scene_t* scene, alice_entity_handle_t entity, void*);

typedef struct alice_script_t {
	void* instance;

	char* get_instance_size_name;
	char* on_init_name;
	char* on_update_name;
	char* on_physics_update_name;
	char* on_free_name;

	alice_script_init_f on_init;
	alice_script_update_f on_update;
	alice_script_physics_update_f on_physics_update;
	alice_script_free_f on_free;

	alice_entity_handle_t entity;
} alice_script_t;

typedef struct alice_script_context_t {
	alice_script_t* scripts;
	u32 script_count;
	u32 script_capacity;

	alice_scene_t* scene;

	void* handle;
} alice_script_context_t;

ALICE_API alice_script_context_t* alice_new_script_context(alice_scene_t* scene, const char* assembly_path);
ALICE_API void alice_free_script_context(alice_script_context_t* context);
ALICE_API alice_script_t* alice_new_script(alice_script_context_t* context, alice_entity_handle_t entity,
		const char* get_instance_size_name,
		const char* on_init_name,
		const char* on_update_name,
		const char* on_physics_update_name,
		const char* on_free_name, bool init_on_create);
ALICE_API void alice_delete_script(alice_script_context_t* context, alice_script_t* script);
ALICE_API void alice_deinit_script(alice_script_context_t* context, alice_script_t* script);
ALICE_API void alice_init_scripts(alice_script_context_t* context);
ALICE_API void alice_update_scripts(alice_script_context_t* context, double timestep);
ALICE_API void alice_physics_update_scripts(alice_script_context_t* context, double timestep);
ALICE_API void alice_free_scripts(alice_script_context_t* context);

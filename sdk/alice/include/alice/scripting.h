#pragma once

#include "alice/core.h"
#include "alice/entity.h"

typedef u32 (*alice_ScriptGetInstanceSizeFunction)();
typedef void (*alice_ScriptInitFunction)(alice_Scene* scene, alice_EntityHandle entity, void*);
typedef void (*alice_ScriptUpdateFunction)(alice_Scene* scene, alice_EntityHandle entity, void*, double);
typedef void (*alice_ScriptFreeFunction)(alice_Scene* scene, alice_EntityHandle entity, void*);

typedef struct alice_Script {
	void* instance;

	char* get_instance_size_name;
	char* on_init_name;
	char* on_update_name;
	char* on_free_name;

	alice_ScriptInitFunction on_init;
	alice_ScriptUpdateFunction on_update;
	alice_ScriptFreeFunction on_free;

	alice_EntityHandle entity;
} alice_Script;

typedef struct alice_ScriptContext {
	alice_Script* scripts;
	u32 script_count;
	u32 script_capacity;

	alice_Scene* scene;

	void* handle;
} alice_ScriptContext;

ALICE_API alice_ScriptContext* alice_new_script_context(alice_Scene* scene, const char* assembly_path);
ALICE_API void alice_free_script_context(alice_ScriptContext* context);
ALICE_API alice_Script* alice_new_script(alice_ScriptContext* context, alice_EntityHandle entity,
		const char* get_instance_size_name,
		const char* on_init_name,
		const char* on_update_name,
		const char* on_free_name, bool init_on_create);
ALICE_API void alice_delete_script(alice_ScriptContext* context, alice_Script* script);
ALICE_API void alice_deinit_script(alice_ScriptContext* context, alice_Script* script);
ALICE_API void alice_init_scripts(alice_ScriptContext* context);
ALICE_API void alice_update_scripts(alice_ScriptContext* context, double timestep);
ALICE_API void alice_free_scripts(alice_ScriptContext* context);

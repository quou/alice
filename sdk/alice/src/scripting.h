#pragma once

#include "alice/core.h"

typedef struct alice_ScriptAssembly {
	char* name;
	void* handle;
} alice_ScriptAssembly;

typedef void (*alice_ScriptUpdateFunc)(double, alice_Scene*, alice_EntityHandle, void*);
typedef void (*alice_ScriptInitFunc)(alice_Scene*, alice_EntityHandle, void*);
typedef void (*alice_ScriptFreeFunc)(alice_Scene*, alice_EntityHandle, void*);

typedef struct alice_Script {
	alice_ScriptAssembly* sasm;
	alice_ScriptUpdateFunc update_func;
	alice_ScriptFreeFunc free_func;
	void* instance;
	i32 instance_size;
} alice_Script;

ALICE_API alice_ScriptAssembly alice_new_script_assembly(const char* lib_name);
ALICE_API void alice_free_script_assembly(alice_ScriptAssembly* sasm);
ALICE_API void* alice_get_script_function(alice_ScriptAssembly* sasm, const char* function);

ALICE_API alice_Script alice_new_script(alice_ScriptAssembly* sasm,
	alice_Scene* scene, alice_Entity entity,
	const char* instance_size_name,
	const char* init_func_name,
	const char* update_func_name,
	const char* free_func_name);

ALICE_API void alice_init_scene_scripting(alice_Scene* scene);
ALICE_API void alice_update_scripts(alice_Scene* scene, double timestep);

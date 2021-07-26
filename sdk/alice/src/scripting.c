#include <assert.h>
#include <stdlib.h>

#include "alice/scripting.h"
#include "alice/entity.h"

#ifdef ALICE_PLATFORM_WINDOWS
#include <windows.h>

static void alice_init_script_context_library(alice_ScriptContext* context, const char* assembly_path) {
	assert(context);

	context->handle = LoadLibraryA(assembly_path);
	if (!context->handle) {
		alice_log_error("Failed to load script assembly: `%s'", assembly_path);
	}
}

static void* alice_get_script_proc(alice_ScriptContext* context, const char* name) {
	assert(context);

	void* function = GetProcAddress(context->handle, name);
	if (!function) {
		alice_log_error("Failed to locate function `%s'", name);
	}

	return function;
}

static void alice_deinit_script_context_library(alice_ScriptContext* context) {
	assert(context);

	FreeLibrary(context->handle);
}

#else

#include <dlfcn.h>

static void alice_init_script_context_library(alice_ScriptContext* context, const char* assembly_path) {
	assert(context);

	context->handle = dlopen(assembly_path, RTLD_NOW);
	if (!context->handle) {
		alice_log_error("Failed to load script assembly: `%s': %s", assembly_path, dlerror());
	}
}

static void* alice_get_script_proc(alice_ScriptContext* context, const char* name) {
	assert(context);

	void* function = dlsym(context->handle, name);
	if (!function) {
		alice_log_error("Failed to locate function `%s'", name);
	}

	return function;
}

static void alice_deinit_script_context_library(alice_ScriptContext* context) {
	assert(context);

	dlclose(context->handle);
}

#endif

alice_ScriptContext* alice_new_script_context(alice_Scene* scene, const char* assembly_path) {
	assert(scene);

	alice_ScriptContext* new = malloc(sizeof(alice_ScriptContext));

	new->scripts = alice_null;
	new->script_count = 0;
	new->script_capacity = 0;

	new->scene = scene;

	alice_init_script_context_library(new, assembly_path);

	return new;
}

void alice_free_script_context(alice_ScriptContext* context) {
	assert(context);

	alice_deinit_script_context_library(context);

	if (context->script_capacity > 0) {
		free(context->scripts);
	}

	free(context);
}

alice_Script* alice_new_script(alice_ScriptContext* context, alice_EntityHandle entity,
		const char* get_instance_size_name,
		const char* on_init_name,
		const char* on_update_name,
		const char* on_physics_update_name,
		const char* on_free_name, bool init_on_create) {
	assert(context);

	if (context->script_count >= context->script_capacity) {
		context->script_capacity = alice_grow_capacity(context->script_capacity);
		context->scripts = realloc(context->scripts, context->script_capacity * sizeof(alice_Script));
	}

	alice_Script* new = &context->scripts[context->script_count++];

	new->instance = alice_null;

	new->entity = entity;

	alice_Entity* entity_ptr = alice_get_entity_ptr(context->scene, entity);
	entity_ptr->script = new;

	new->get_instance_size_name = alice_null;
	new->on_init_name = alice_null;
	new->on_update_name = alice_null;
	new->on_physics_update_name = alice_null;
	new->on_free_name = alice_null;

	new->on_init = alice_null;
	new->on_update = alice_null;
	new->on_physics_update = alice_null;
	new->on_free = alice_null;

	alice_ScriptGetInstanceSizeFunction get_size = alice_null;
	if (get_instance_size_name) {
		new->get_instance_size_name = alice_copy_string(get_instance_size_name);

		get_size = alice_get_script_proc(context, get_instance_size_name);
	}

	if (on_init_name) {
		new->on_init_name = alice_copy_string(on_init_name);
		new->on_init = alice_get_script_proc(context, on_init_name);
	}

	if (on_update_name) {
		new->on_update_name = alice_copy_string(on_update_name);
		new->on_update = alice_get_script_proc(context, on_update_name);
	}

	if (on_physics_update_name) {
		new->on_physics_update_name = alice_copy_string(on_physics_update_name);
		new->on_physics_update = alice_get_script_proc(context, on_physics_update_name);
	}

	if (on_free_name) {
		new->on_free_name = alice_copy_string(on_free_name);
		new->on_free = alice_get_script_proc(context, on_free_name);
	}

	if (get_size) {
		new->instance = malloc(get_size());
	}

	if (init_on_create && new->on_init) {
		new->on_init(context->scene, entity, new->instance);
	}

	return new;
}

void alice_delete_script(alice_ScriptContext* context, alice_Script* script) {
	assert(context);
	assert(script);

	i32 index = -1;

	for (u32 i = 0; i < context->script_count; i++) {
		if (&context->scripts[i] == script) {
			index = i;
			break;
		}
	}

	if (index == -1) {
		return;
	}

	alice_deinit_script(context, &context->scripts[index]);

	for (u32 i = index; i < context->script_count - 1; i++) {
		context->scripts[i] = context->scripts[i + 1];
	}

	context->script_count--;
}

void alice_deinit_script(alice_ScriptContext* context, alice_Script* script) {
	assert(context);
	assert(script);

	if (script->on_free) {
		script->on_free(context->scene, script->entity, script->instance);
	}

	if (script->instance) {
		free(script->instance);
	}

	alice_Entity* entity_ptr = alice_get_entity_ptr(context->scene, script->entity);
	entity_ptr->script = alice_null;

	free(script->get_instance_size_name);
	free(script->on_init_name);
	free(script->on_update_name);
	free(script->on_free_name);
}

void alice_init_scripts(alice_ScriptContext* context) {
	assert(context);

	for (u32 i = 0; i < context->script_count; i++) {
		alice_Script* script = &context->scripts[i];
		if (script->on_init) {
			script->on_init(context->scene, script->entity, script->instance);
		}
	}
}

void alice_update_scripts(alice_ScriptContext* context, double timestep) {
	assert(context);

	for (u32 i = 0; i < context->script_count; i++) {
		alice_Script* script = &context->scripts[i];
		if (script->on_update) {
			script->on_update(context->scene, script->entity, script->instance, timestep);
		}
	}
}

void alice_physics_update_scripts(alice_ScriptContext* context, double timestep) {
	assert(context);

	for (u32 i = 0; i < context->script_count; i++) {
		alice_Script* script = &context->scripts[i];
		if (script->on_physics_update) {
			script->on_physics_update(context->scene, script->entity, script->instance, timestep);
		}
	}
}

void alice_free_scripts(alice_ScriptContext* context) {
	assert(context);

	for (u32 i = 0; i < context->script_count; i++) {
		alice_Script* script = &context->scripts[i];
		alice_deinit_script(context, script);
	}
}

#pragma once

#include "alice/core.h"

/* Forward decls */
typedef enum alice_texture_flags_t alice_texture_flags_t;
typedef struct alice_texture_t alice_texture_t;
typedef struct alice_shader_t alice_shader_t;
typedef struct alice_material_t alice_material_t;
typedef struct alice_model_t alice_model_t;

typedef enum alice_resource_type_t {
	ALICE_RESOURCE_STRING,
	ALICE_RESOURCE_BINARY,
	ALICE_RESOURCE_TEXTURE,
	ALICE_RESOURCE_SHADER,
	ALICE_RESOURCE_ASSEMBLY,
	ALICE_RESOURCE_SCRIPT,
	ALICE_RESOURCE_MODEL,
	ALICE_RESOURCE_MATERIAL
} alice_resource_type_t;

typedef struct alice_resource_t {
	alice_resource_type_t type;

	void* payload;
	u32 payload_size;

	i64 modtime;

	char* file_name;
	u32 file_name_length;
	u32 file_name_hash;
} alice_resource_t;

ALICE_API void alice_init_resource_manager(const char* working_dir);
ALICE_API void alice_free_resource_manager();
ALICE_API void alice_init_default_resources();
ALICE_API void alice_free_resource(alice_resource_t* resource);
ALICE_API void alice_free_resource_payload(alice_resource_t* resource);
ALICE_API const char* alice_get_file_extension(const char* file_name);
ALICE_API const char* alice_get_file_name(const char* file_path);
ALICE_API alice_resource_type_t alice_predict_resource_type(const char* file_extension);

ALICE_API const char* alice_get_texture_resource_filename(alice_texture_t* texture);
ALICE_API const char* alice_get_shader_resource_filename(alice_shader_t* shader);
ALICE_API const char* alice_get_material_resource_filename(alice_material_t* material);
ALICE_API const char* alice_get_model_resource_filename(alice_model_t* model);

ALICE_API void alice_get_working_dir(char* working_dir);

ALICE_API void alice_reload_changed_resources();

ALICE_API void alice_reload_resource(alice_resource_t* resource);

ALICE_API alice_resource_t* alice_load_binary(const char* path);
ALICE_API alice_resource_t* alice_load_string(const char* path);
ALICE_API alice_texture_t* alice_load_texture(const char* path, alice_texture_flags_t flags);
ALICE_API alice_shader_t* alice_load_shader(const char* path);
ALICE_API alice_material_t* alice_load_material(const char* path);
ALICE_API alice_model_t* alice_load_model(const char* path);

ALICE_API void alice_save_material(alice_material_t* material, const char* path);

typedef void (*alice_resource_iterate_f)(const char* path, const char* extension,
	alice_resource_type_t predicted_type, bool is_directory, void* ud);

ALICE_API void alice_iterate_resource_directory(const char* directory, alice_resource_iterate_f function, void* ud);

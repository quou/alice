#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <physfs.h>

#include "alice/resource.h"
#include "alice/dtable.h"
#include "alice/graphics.h"

#define ALICE_RESOURCE_TABLE_MAX_LOAD 0.75

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include <assimp/matrix4x4.h>

typedef struct aiScene aiScene;
typedef struct aiNode aiNode;
typedef struct aiMesh aiMesh;
typedef struct aiFace aiFace;
typedef struct aiMatrix4x4 aiMatrix4x4;

typedef struct alice_ResourceTableEntry {
	u32 key;
	alice_Resource* value;
} alice_ResourceTableEntry;

typedef struct alice_ResourceTable {
	alice_ResourceTableEntry* entries;
	u32 count;
	u32 capacity;
} alice_ResourceTable;

static alice_ResourceTable* alice_new_resource_table() {
	alice_ResourceTable* table = malloc(sizeof(alice_ResourceTable));

	table->count = 0;
	table->capacity = 0;
	table->entries = NULL;

	return table;
}

static void alice_free_resource_table(alice_ResourceTable* table) {
	assert(table);

	for (u32 i = 0; i < table->capacity; i++) {
		alice_ResourceTableEntry* entry = &table->entries[i];

		if (entry->value) {
			alice_free_resource(entry->value);
		}
	}

	table->count = 0;
	table->capacity = 0;

	free(table->entries);

	table->entries = NULL;

	free(table);
}

static alice_ResourceTableEntry* alice_resource_table_find_entry(
	alice_ResourceTableEntry* entries, u32 capacity, u32 key) {

	u32 index = key % capacity;

	for (;;) {
		alice_ResourceTableEntry* entry = &entries[index];
		if (entry->key == key || entry->key == 0) {
			return entry;
		}

		index = (index + 1) % capacity;
	}
}

static void alice_resource_table_adjust_capacity(alice_ResourceTable* table, u32 capacity) {
	alice_ResourceTableEntry* entries =
		malloc(capacity * sizeof(alice_ResourceTableEntry));
	for (u32 i = 0; i < capacity; i++) {
		entries[i].key = 0;
		entries[i].value = NULL;
	}

	for (u32 i = 0; i < table->capacity; i++) {
		alice_ResourceTableEntry* entry = &table->entries[i];
		if (entry->key == 0) { continue; }

		alice_ResourceTableEntry* dest =
			alice_resource_table_find_entry(entries, capacity, entry->key);
		dest->key = entry->key;
		dest->value = entry->value;
	}

	free(table->entries);

	table->entries = entries;
	table->capacity = capacity;
}

static bool alice_resource_table_set(alice_ResourceTable* table,
	u32 key, alice_Resource* value) {

	if (table->count + 1 > table->capacity * ALICE_RESOURCE_TABLE_MAX_LOAD) {
		u32 capacity = alice_grow_capacity(table->capacity);
		alice_resource_table_adjust_capacity(table, capacity);
	}

	alice_ResourceTableEntry* entry =
		alice_resource_table_find_entry(table->entries, table->capacity, key);
	bool is_new_key = entry->key == 0;
	if (is_new_key) { table->count++; }

	entry->key = key;
	entry->value = value;

	return is_new_key;
}

static alice_Resource* alice_resource_table_get(
	alice_ResourceTable* table, u32 key) {

	if (table->count == 0) { return NULL; }

	alice_ResourceTableEntry* entry = alice_resource_table_find_entry(
		table->entries, table->capacity, key);
	if (entry->key == 0) { return NULL; }

	return entry->value;
}

static void alice_resource_table_copy(alice_ResourceTable* from, alice_ResourceTable* to) {
	for (u32 i = 0; i < from->capacity; i++) {
		alice_ResourceTableEntry* entry = &from->entries[i];
		if (entry->key != 0) {
			alice_resource_table_set(to, entry->key, entry->value);
		}
	}
}

typedef struct alice_ResourceManager {
	const char* working_dir;

	alice_ResourceTable* table;
} alice_ResourceManager;

alice_ResourceManager rm;

void alice_free_resource(alice_Resource* resource) {
	alice_free_resource_payload(resource);
	if (resource->type == ALICE_RESOURCE_SHADER ||
		resource->type == ALICE_RESOURCE_TEXTURE ||
		resource->type == ALICE_RESOURCE_MATERIAL ||
		resource->type == ALICE_RESOURCE_MODEL) {
		free(resource->payload);
	}
	free(resource->file_name);

	free(resource);
}

void alice_free_resource_payload(alice_Resource* resource) {
	if (resource->type == ALICE_RESOURCE_TEXTURE) {
		alice_deinit_texture(resource->payload);
	}
	else if (resource->type == ALICE_RESOURCE_SHADER) {
		alice_deinit_shader(resource->payload);
	}
	else if (resource->type == ALICE_RESOURCE_MATERIAL) {
		/* Nothing needs to be freed for materials */
		return;
	}
	else if (resource->type == ALICE_RESOURCE_MODEL) {
		alice_deinit_model(resource->payload);
	}
	else {
		free(resource->payload);
	}
}

static alice_Resource* alice_resource_manager_add(alice_Resource* resource) {
	alice_resource_table_set(rm.table, resource->file_name_hash, resource);

	return resource;
}

void alice_init_resource_manager(const char* working_dir) {
	rm.working_dir = working_dir;

	if (PHYSFS_isInit()) {
		alice_free_resource_manager();
	}

	PHYSFS_init(NULL);
	PHYSFS_mount(working_dir, "/", 1);
	PHYSFS_setWriteDir(working_dir);

	rm.table = alice_new_resource_table();
}

void alice_init_default_resources() {
	/* Default cube */
	{
		alice_Resource* cube_resource = malloc(sizeof(alice_Resource));
		*cube_resource = (alice_Resource){
			.type = ALICE_RESOURCE_MODEL,

			.payload = alice_new_model(),
			.payload_size = sizeof(alice_Model),

			.modtime = 0,

			.file_name = malloc(5),
			.file_name_length = 5,
			.file_name_hash = alice_hash_string("cube")
		};
		strcpy(cube_resource->file_name, "cube");

		alice_model_add_mesh(cube_resource->payload, alice_new_cube_mesh());

		alice_resource_manager_add(cube_resource);
	}

	/* Default sphere */
	{
		alice_Resource* sphere_resource = malloc(sizeof(alice_Resource));
		*sphere_resource = (alice_Resource){
			.type = ALICE_RESOURCE_MODEL,

			.payload = alice_new_model(),
			.payload_size = sizeof(alice_Model),

			.modtime = 0,

			.file_name = malloc(7),
			.file_name_length = 7,
			.file_name_hash = alice_hash_string("sphere")
		};
		alice_model_add_mesh(sphere_resource->payload, alice_new_sphere_mesh());

		strcpy(sphere_resource->file_name, "sphere");

		alice_resource_manager_add(sphere_resource);
	}

	/* Default material */
	{
		const char* name = "default_material";
		const u32 name_len = strlen(name);

		alice_Resource* default_material_resource = malloc(sizeof(alice_Resource));
		*default_material_resource = (alice_Resource) {
			.type = ALICE_RESOURCE_MATERIAL,

			.payload = malloc(sizeof(alice_Material)),
			.payload_size = sizeof(alice_Material),

			.modtime = 0,

			.file_name = malloc(name_len + 1),
			.file_name_length = name_len,
			.file_name_hash = alice_hash_string(name)
		};
		strcpy(default_material_resource->file_name, name);

		*((alice_Material*)default_material_resource->payload) = (alice_Material) {
			.shader = alice_load_shader("shaders/pbr.glsl"),
			.as.pbr = {
				.albedo = 0xffffff,
				.roughness = 0.3f,
				.metallic = 1.0f,
				.emissive = 0.0f,

				.albedo_map = alice_null,
				.normal_map = alice_null,
				.metallic_map = alice_null,
				.roughness_map = alice_null,
				.ambient_occlusion_map = alice_null
			}
		};

		alice_resource_manager_add(default_material_resource);
	}
}

void alice_get_working_dir(char* working_dir) {
	const char* ending = "/";
	if (rm.working_dir[strlen(rm.working_dir) - 1] == '/') {
		ending = "";
	}
	strcpy(working_dir, rm.working_dir);
	strcat(working_dir, ending);
}

void alice_free_resource_manager() {
	alice_free_resource_table(rm.table);

	PHYSFS_unmount(rm.working_dir);
	PHYSFS_deinit();
}

const char* alice_get_file_extension(const char* file_name) {
	assert(file_name);

	const char* dot = strrchr(file_name, '.');
	if (!dot || dot == file_name) return "";
	return dot + 1;
}

const char* alice_get_file_name(const char* file_path) {
	assert(file_path);

	const char* slash = strrchr(file_path, '/');
	return slash + 1;
}

const char* alice_get_texture_resource_filename(alice_Texture* texture) {
	assert(texture);

	for (u32 i = 0; i < rm.table->capacity; i++) {
		alice_Resource* resource = rm.table->entries[i].value;

		if (resource && resource->payload == texture) {
			return resource->file_name;
		}
	}

	return NULL;
}

const char* alice_get_shader_resource_filename(alice_Shader* shader) {
	assert(shader);

	for (u32 i = 0; i < rm.table->capacity; i++) {
		alice_Resource* resource = rm.table->entries[i].value;

		if (resource && resource->payload == shader) {
			return resource->file_name;
		}
	}

	return NULL;
}

const char* alice_get_material_resource_filename(alice_Material* material) {
	assert(material);

	for (u32 i = 0; i < rm.table->capacity; i++) {
		alice_Resource* resource = rm.table->entries[i].value;

		if (resource && resource->payload == material) {
			return resource->file_name;
		}
	}

	return NULL;
}

const char* alice_get_model_resource_filename(alice_Model* model) {
	assert(model);

	for (u32 i = 0; i < rm.table->capacity; i++) {
		alice_Resource* resource = rm.table->entries[i].value;

		if (resource && resource->payload == model) {
			return resource->file_name;
		}
	}

	return NULL;
}

static bool impl_alice_load_binary(alice_Resource* resource, const char* path) {
	PHYSFS_file* file = PHYSFS_openRead(path);
	if (!file) {
		alice_log_error("Failed to load resource %s", path);
		PHYSFS_close(file);
		return false;
	}

	PHYSFS_Stat stat;
	PHYSFS_stat(path, &stat);

	u32 file_size = PHYSFS_fileLength(file);

	void* buffer = malloc(file_size);

	PHYSFS_readBytes(file, buffer, file_size);

	const u32 file_name_length = strlen(path) + 1;

	(*resource) = (alice_Resource){
		.type = ALICE_RESOURCE_BINARY,

		.payload = buffer,
		.payload_size = file_size,

		.modtime = stat.modtime,

		.file_name_length = file_name_length,
		.file_name_hash = alice_hash_string(path)
	};

	resource->file_name = malloc(file_name_length * sizeof(char));
	strcpy(resource->file_name, path);

	PHYSFS_close(file);

	return true;
}

static bool impl_alice_load_string(alice_Resource* resource, const char* path) {
	PHYSFS_file* file = PHYSFS_openRead(path);
	if (!file) {
		alice_log_error("Failed to load resource %s", path);
		PHYSFS_close(file);
		return false;
	}

	PHYSFS_Stat stat;
	PHYSFS_stat(path, &stat);

	u32 file_size = PHYSFS_fileLength(file);

	char* buffer = malloc(file_size + 1);
	buffer[file_size] = '\0';

	PHYSFS_readBytes(file, buffer, file_size);

	const u32 file_name_length = strlen(path) + 1;

	(*resource) = (alice_Resource){
		.type = ALICE_RESOURCE_STRING,

		.payload = buffer,
		.payload_size = file_size,

		.modtime = stat.modtime,

		.file_name_length = file_name_length,
		.file_name_hash = alice_hash_string(path)
	};

	resource->file_name = malloc(file_name_length + 1 * sizeof(char));
	strcpy(resource->file_name, path);

	PHYSFS_close(file);

	return true;
}

static bool impl_alice_load_texture(alice_Resource* resource, const char* path, alice_TextureFlags flags, bool new) {
	alice_Resource* raw = malloc(sizeof(alice_Resource));
	if (!impl_alice_load_binary(raw, path)) {
		free(raw);
		return false;
	}

	resource->type = ALICE_RESOURCE_TEXTURE;
	resource->payload_size = sizeof(alice_Texture);
	resource->modtime = raw->modtime;
	resource->file_name = malloc(strlen(path) + 1);
	resource->file_name_hash = raw->file_name_hash;

	if (new) {
		resource->payload = malloc(sizeof(alice_Texture));
	}

	alice_init_texture(resource->payload, raw, flags);

	strcpy(resource->file_name, path);

	alice_free_resource(raw);

	return true;
}

static bool impl_alice_load_shader(alice_Resource* resource, const char* path, bool new) {
	alice_Resource* raw = malloc(sizeof(alice_Resource));
	if (!impl_alice_load_string(raw, path)) {
		free(raw);
		return false;
	}

	resource->type = ALICE_RESOURCE_SHADER;
	resource->payload_size = sizeof(alice_Shader);
	resource->modtime = raw->modtime;
	resource->file_name = malloc(strlen(path) + 1);
	resource->file_name_hash = raw->file_name_hash;

	if (new) {
		resource->payload = malloc(sizeof(alice_Shader));
	}

	alice_init_shader(resource->payload, raw->payload);

	strcpy(resource->file_name, path);

	alice_free_resource(raw);

	return true;
}

static alice_Texture* impl_alice_load_texture_from_dtable(alice_DTable* parent_table, const char* name) {
	assert(parent_table);

	alice_DTable* texture_table = alice_dtable_find_child(parent_table, name);

	if (texture_table) {
		alice_TextureFlags flags = 0;

		alice_DTable* is_antialiased_table = alice_dtable_find_child(texture_table, "is_antialiased");
		if (is_antialiased_table && is_antialiased_table->value.type == ALICE_DTABLE_BOOL &&
				is_antialiased_table->value.as.boolean) {
			flags |= ALICE_TEXTURE_ANTIALIASED;
		} else {
			flags |= ALICE_TEXTURE_ALIASED;
		}

		alice_DTable* path_table = alice_dtable_find_child(texture_table, "path");
		if (path_table && path_table->value.type == ALICE_DTABLE_STRING) {
			return alice_load_texture(path_table->value.as.string, flags);
		}
	}

	return alice_null;
}

static void alice_load_pbr_material(alice_DTable* table, alice_PBRMaterial* material) {
	assert(table);
	assert(material);

	material->albedo_map = impl_alice_load_texture_from_dtable(table, "albedo_map");
	material->normal_map = impl_alice_load_texture_from_dtable(table, "normal_map");
	material->metallic_map = impl_alice_load_texture_from_dtable(table, "metallic_map");
	material->roughness_map = impl_alice_load_texture_from_dtable(table, "roughness_map");
	material->ambient_occlusion_map = impl_alice_load_texture_from_dtable(table, "ao_map");

	alice_DTable* albedo_table = alice_dtable_find_child(table, "albedo");
	if (albedo_table && albedo_table->child_count == 3) {
		float r = 1.0, g = 1.0, b = 1.0;

		alice_DTable* r_table = alice_dtable_find_child(albedo_table, "r");
		if (r_table && r_table->value.type == ALICE_DTABLE_NUMBER) {
			r = (float)r_table->value.as.number;
		}

		alice_DTable* g_table = alice_dtable_find_child(albedo_table, "g");
		if (g_table && g_table->value.type == ALICE_DTABLE_NUMBER) {
			g = (float)g_table->value.as.number;
		}

		alice_DTable* b_table = alice_dtable_find_child(albedo_table, "b");
		if (b_table && b_table->value.type == ALICE_DTABLE_NUMBER) {
			b = (float)b_table->value.as.number;
		}

		material->albedo = alice_color_from_rgb_color((alice_RGBColor) { r, g, b });
	}

	alice_DTable* metallic_table = alice_dtable_find_child(table, "metallic");
	if (metallic_table && metallic_table->value.type == ALICE_DTABLE_NUMBER) {
		material->metallic = metallic_table->value.as.number;
	}

	alice_DTable* roughness_table = alice_dtable_find_child(table, "roughness");
	if (roughness_table && roughness_table->value.type == ALICE_DTABLE_NUMBER) {
		material->roughness = roughness_table->value.as.number;
	}

	alice_DTable* emissive_table = alice_dtable_find_child(table, "emissive");
	if (emissive_table && emissive_table->value.type == ALICE_DTABLE_NUMBER) {
		material->emissive = emissive_table->value.as.number;
	}
}

static void alice_load_phong_material(alice_DTable* table, alice_PhongMaterial* material) {
	assert(table);
	assert(material);

	material->diffuse_map = impl_alice_load_texture_from_dtable(table, "diffuse_map");

	alice_DTable* diffuse_table = alice_dtable_find_child(table, "diffuse");
	if (diffuse_table && diffuse_table->child_count == 3) {
		float r = 1.0, g = 1.0, b = 1.0;

		alice_DTable* r_table = alice_dtable_find_child(diffuse_table, "r");
		if (r_table && r_table->value.type == ALICE_DTABLE_NUMBER) {
			r = (float)r_table->value.as.number;
		}

		alice_DTable* g_table = alice_dtable_find_child(diffuse_table, "g");
		if (g_table && g_table->value.type == ALICE_DTABLE_NUMBER) {
			g = (float)g_table->value.as.number;
		}

		alice_DTable* b_table = alice_dtable_find_child(diffuse_table, "b");
		if (b_table && b_table->value.type == ALICE_DTABLE_NUMBER) {
			b = (float)b_table->value.as.number;
		}

		material->diffuse = alice_color_from_rgb_color((alice_RGBColor) { r, g, b });
	}
}

static bool impl_alice_load_material(alice_Resource* resource, const char* path, bool new) {
	alice_Resource* raw = malloc(sizeof(alice_Resource));
	if (!impl_alice_load_string(raw, path)) {
		free(raw);
		return false;
	}

	alice_DTable* table = alice_read_dtable(raw);

	resource->type = ALICE_RESOURCE_MATERIAL;
	resource->payload_size = sizeof(alice_Material);
	resource->modtime = raw->modtime;
	resource->file_name = malloc(strlen(path) + 1);
	resource->file_name_hash = raw->file_name_hash;

	strcpy(resource->file_name, path);

	if (new) {
		resource->payload = malloc(sizeof(alice_Material));
	}

	alice_Material* material = resource->payload;
	*material = (alice_Material){
		.shader = alice_null,
		.type = ALICE_MATERIAL_PBR,
		.as.pbr = {
			.albedo = 0xffffff,
			.roughness = 0.3f,
			.metallic = 1.0f,
			.emissive = 0.0f,

			.albedo_map = alice_null,
			.normal_map = alice_null,
			.metallic_map = alice_null,
			.roughness_map = alice_null,
			.ambient_occlusion_map = alice_null
		}
	};

	alice_DTable* shader_table = alice_dtable_find_child(table, "shader");
	if (shader_table && shader_table->value.type == ALICE_DTABLE_STRING) {
		material->shader = alice_load_shader(shader_table->value.as.string);
	}
	else {
		alice_log_warning("Material does not have a shader, so objects with this material won't render");
	}

	alice_DTable* pbr_table = alice_dtable_find_child(table, "pbr_material");
	if (pbr_table) {
		material->type = ALICE_MATERIAL_PBR;
		alice_load_pbr_material(pbr_table, &material->as.pbr);
	}

	alice_DTable* phong_table = alice_dtable_find_child(table, "phong_material");
	if (phong_table) {
		material->type = ALICE_MATERIAL_PHONG;
		alice_load_phong_material(phong_table, &material->as.phong);
	}

	alice_free_dtable(table);

	return true;
}

static alice_Mesh alice_process_model_mesh(aiNode* node, aiMesh* ai_mesh, const aiScene* scene) {
	assert(ai_mesh && scene);

	if (!ai_mesh->mNormals) {
		alice_log_error("Cannot load mesh that doesn't have normals");
	}

	if (!ai_mesh->mTextureCoords[0]) {
		alice_log_error("Cannot load mesh that doesn't have texture coordinates");
	}

	float* vertices = malloc(ai_mesh->mNumVertices * 8 * sizeof(float));
	u32 current_vertex = 0;

	for (u32 i = 0; i < ai_mesh->mNumVertices; i++) {
		vertices[current_vertex++] = ai_mesh->mVertices[i].x;
		vertices[current_vertex++] = ai_mesh->mVertices[i].y;
		vertices[current_vertex++] = ai_mesh->mVertices[i].z;

		vertices[current_vertex++] = ai_mesh->mNormals[i].x;
		vertices[current_vertex++] = ai_mesh->mNormals[i].y;
		vertices[current_vertex++] = ai_mesh->mNormals[i].z;

		vertices[current_vertex++] = ai_mesh->mTextureCoords[0][i].x;
		vertices[current_vertex++] = ai_mesh->mTextureCoords[0][i].y;
	}

	u32* indices = alice_null;
	u32 index_count = 0;
	u32 index_capacity = 0;

	for (u32 i = 0; i < ai_mesh->mNumFaces; i++) {
		aiFace face = ai_mesh->mFaces[i];

		index_capacity += face.mNumIndices;
		indices = realloc(indices, index_capacity * sizeof(u32));

		for (u32 ii = 0; ii < face.mNumIndices; ii++) {
			indices[index_count++] = face.mIndices[ii];
		}
	}


	alice_VertexBuffer* vb = alice_new_vertex_buffer(
		ALICE_VERTEXBUFFER_DRAW_TRIANGLES | ALICE_VERTEXBUFFER_STATIC_DRAW);

	alice_bind_vertex_buffer_for_edit(vb);
	alice_push_vertices(vb, vertices, current_vertex);
	alice_push_indices(vb, indices, index_count);
	alice_configure_vertex_buffer(vb, 0, 3, 8, 0); /* vec3 position */
	alice_configure_vertex_buffer(vb, 1, 3, 8, 3); /* vec3 normal */
	alice_configure_vertex_buffer(vb, 2, 2, 8, 6); /* vec2 uv */
	alice_bind_vertex_buffer_for_edit(NULL);

	alice_Mesh mesh;

	alice_init_mesh(&mesh, vb);

	aiMatrix4x4 ai_transform = node->mTransformation;

	mesh.transform.elements[0][0] = ai_transform.a1;
	mesh.transform.elements[1][0] = ai_transform.a2;
	mesh.transform.elements[2][0] = ai_transform.a3;
	mesh.transform.elements[3][0] = ai_transform.a4;

	mesh.transform.elements[0][1] = ai_transform.b1;
	mesh.transform.elements[1][1] = ai_transform.b2;
	mesh.transform.elements[2][1] = ai_transform.b3;
	mesh.transform.elements[3][1] = ai_transform.b4;

	mesh.transform.elements[0][2] = ai_transform.c1;
	mesh.transform.elements[1][2] = ai_transform.c2;
	mesh.transform.elements[2][2] = ai_transform.c3;
	mesh.transform.elements[3][2] = ai_transform.c4;

	mesh.transform.elements[0][3] = ai_transform.d1;
	mesh.transform.elements[1][3] = ai_transform.d2;
	mesh.transform.elements[2][3] = ai_transform.d3;
	mesh.transform.elements[3][3] = ai_transform.d4;

	alice_calculate_aabb_from_mesh(&mesh.aabb, vertices, current_vertex, 8);

	free(indices);
	free(vertices);

	return mesh;
}

static void alice_process_model_node(alice_Model* model, aiNode* node, const aiScene* scene) {
	assert(model && node && scene);

	for (u32 i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		alice_model_add_mesh(model, alice_process_model_mesh(node, mesh, scene));
	}

	for (u32 i = 0; i < node->mNumChildren; i++) {
		alice_process_model_node(model, node->mChildren[i], scene);
	}
}

static bool impl_alice_load_model(alice_Resource* resource, const char* path, bool new) {
	assert(resource);

	alice_Resource* raw = malloc(sizeof(alice_Resource));
	if (!impl_alice_load_binary(raw, path)) {
		free(raw);
		return false;
	}

	resource->type = ALICE_RESOURCE_MODEL;
	resource->payload_size = sizeof(alice_Model);
	resource->modtime = raw->modtime;
	resource->file_name = malloc(strlen(path) + 1);
	resource->file_name_hash = raw->file_name_hash;

	if (new) {
		resource->payload = malloc(sizeof(alice_Model));
	}

	alice_Model* model = resource->payload;
	alice_init_model(model);

	strcpy(resource->file_name, path);

	const char* extension = alice_get_file_extension(path);

	const aiScene* scene = aiImportFileFromMemory(raw->payload, raw->payload_size,
			aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs,
			extension);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		alice_log_error("Error importing model `%s': `%s'", path, aiGetErrorString());
		alice_free_resource(raw);
		return false;
	}

	alice_process_model_node(model, scene->mRootNode, scene);

	aiReleaseImport(scene);

	alice_free_resource(raw);

	return true;
}

alice_Resource* alice_load_binary(const char* path) {
	alice_Resource* got_resource = alice_resource_table_get(rm.table, alice_hash_string(path));
	if (got_resource) {
		return got_resource;
	}

	alice_Resource* resource = malloc(sizeof(alice_Resource));
	if (!impl_alice_load_binary(resource, path)) {
		free(resource);
		return NULL;
	}

	alice_resource_manager_add(resource);

	return resource;
}

alice_Resource* alice_load_string(const char* path) {
	alice_Resource* got_resource = alice_resource_table_get(rm.table, alice_hash_string(path));
	if (got_resource) {
		return got_resource;
	}

	alice_Resource* resource = malloc(sizeof(alice_Resource));
	if (!impl_alice_load_string(resource, path)) {
		free(resource);
		return NULL;
	}

	alice_resource_manager_add(resource);

	return resource;
}

alice_Texture* alice_load_texture(const char* path, alice_TextureFlags flags) {
	alice_Resource* got_resource = alice_resource_table_get(rm.table, alice_hash_string(path));
	if (got_resource) {
		return got_resource->payload;
	}

	alice_Resource* resource = malloc(sizeof(alice_Resource));
	if (!impl_alice_load_texture(resource, path, flags, true)) {
		free(resource);
		return NULL;
	}

	alice_resource_manager_add(resource);

	return resource->payload;
}

alice_Shader* alice_load_shader(const char* path) {
	alice_Resource* got_resource = alice_resource_table_get(rm.table, alice_hash_string(path));
	if (got_resource) {
		return got_resource->payload;
	}

	alice_Resource* resource = malloc(sizeof(alice_Resource));
	if (!impl_alice_load_shader(resource, path, true)) {
		free(resource);
		return NULL;
	}

	alice_resource_manager_add(resource);

	return resource->payload;
}

alice_Material* alice_load_material(const char* path) {
	alice_Resource* got_resource = alice_resource_table_get(rm.table, alice_hash_string(path));
	if (got_resource) {
		return got_resource->payload;
	}

	alice_Resource* resource = malloc(sizeof(alice_Resource));
	if (!impl_alice_load_material(resource, path, true)) {
		free(resource);
		return NULL;
	}

	alice_resource_manager_add(resource);

	return resource->payload;
}

alice_Model* alice_load_model(const char* path) {
	alice_Resource* got_resource = alice_resource_table_get(rm.table, alice_hash_string(path));
	if (got_resource) {
		return got_resource->payload;
	}

	alice_Resource* resource = malloc(sizeof(alice_Resource));
	if (!impl_alice_load_model(resource, path, true)) {
		free(resource);
		return NULL;
	}

	alice_resource_manager_add(resource);

	return resource->payload;
}

void alice_reload_resource(alice_Resource* resource) {
	assert(resource);

	char name[1024];
	strcpy(name, resource->file_name);

	alice_free_resource_payload(resource);
	free(resource->file_name);

	switch (resource->type) {
	case ALICE_RESOURCE_BINARY:
		impl_alice_load_binary(resource, name);
		break;
	case ALICE_RESOURCE_STRING:
		impl_alice_load_string(resource, name);
		break;
	case ALICE_RESOURCE_TEXTURE:
		impl_alice_load_texture(resource, name, ((alice_Texture*)resource->payload)->flags, false);
		break;
	case ALICE_RESOURCE_SHADER:
		impl_alice_load_shader(resource, name, false);
		break;
	case ALICE_RESOURCE_MATERIAL:
		impl_alice_load_material(resource, name, false);
		break;
	default: /* Unreachable */
		break;
	}
}

void alice_reload_changed_resources() {
	for (u32 i = 0; i < rm.table->capacity; i++) {
		alice_ResourceTableEntry* entry = &rm.table->entries[i];

		alice_Resource* r = entry->value;
		if (r) {
			PHYSFS_Stat stat;
			PHYSFS_stat(r->file_name, &stat);

			if (stat.modtime > r->modtime) {
				alice_reload_resource(r);
			}
		}
	}
}

alice_ResourceType alice_predict_resource_type(const char* file_extension) {
	assert(file_extension);

	if (strcmp(file_extension, "png") == 0 ||
		strcmp(file_extension, "jpg") == 0 ||
		strcmp(file_extension, "jpeg") == 0 ||
		strcmp(file_extension, "tga") == 0 ||
		strcmp(file_extension, "bmp") == 0 ||
		strcmp(file_extension, "psd") == 0 ||
		strcmp(file_extension, "gif") == 0 ||
		strcmp(file_extension, "hdr") == 0 ||
		strcmp(file_extension, "pic") == 0 ||
		strcmp(file_extension, "pnm") == 0) {
		return ALICE_RESOURCE_TEXTURE;
	}
	else if (strcmp(file_extension, "glsl") == 0) {
		return ALICE_RESOURCE_SHADER;
	}
	else if (strcmp(file_extension, "txt") == 0 ||
		strcmp(file_extension, "md") == 0) {
		return ALICE_RESOURCE_STRING;
	}
	else if (strcmp(file_extension, "c") == 0 ||
		strcmp(file_extension, "h") == 0 ||
		strcmp(file_extension, "cpp") == 0 ||
		strcmp(file_extension, "hpp") == 0) {
		return ALICE_RESOURCE_SCRIPT;
	}
	else if (strcmp(file_extension, "dll") == 0 ||
		strcmp(file_extension, "so") == 0) {
		return ALICE_RESOURCE_ASSEMBLY;
	}
	else if (strcmp(file_extension, "mat") == 0) {
		return ALICE_RESOURCE_MATERIAL;
	}

	return ALICE_RESOURCE_BINARY;
}
void alice_iterate_resource_directory(const char* directory, alice_ResourceIterateFunction function, void* ud) {
	assert(function);

	char** iterator = PHYSFS_enumerateFiles(directory);
	for (char** i = iterator; *i != NULL; i++) {
		char path[256];
		strncpy(path, directory, 128);
		if (path[strlen(path) - 1] != '/') {
			strncat(path, "/", 1);
		}
		strncat(path, *i, 127);

		const char* extension = alice_get_file_extension(*i);

		alice_ResourceType type_prediction = alice_predict_resource_type(extension);

		PHYSFS_Stat stat;
		PHYSFS_stat(*i, &stat);

		bool is_directory = stat.filetype == PHYSFS_FILETYPE_DIRECTORY;

		function(path, extension, type_prediction, is_directory, ud);
	}
	PHYSFS_freeList(iterator);
}

static void impl_alice_save_material_texture(alice_DTable* parent_table,
		const char* name, alice_Texture* texture) {
	if (!texture) { return; }

	const char* texture_path = alice_get_texture_resource_filename(texture);
	alice_DTable texture_table = alice_new_empty_dtable(name);

	alice_DTable path_table = alice_new_string_dtable("path", texture_path);
	alice_dtable_add_child(&texture_table, path_table);

	alice_DTable is_antialiased_table = alice_new_bool_dtable("is_antialiased",
			texture->flags & ALICE_TEXTURE_ANTIALIASED);
	alice_dtable_add_child(&texture_table, is_antialiased_table);

	alice_dtable_add_child(parent_table, texture_table);
}

static void alice_save_pbr_material(alice_DTable* table, alice_PBRMaterial* material) {
	assert(table);
	assert(material);

	alice_DTable material_table = alice_new_empty_dtable("pbr_material");

	impl_alice_save_material_texture(&material_table, "albedo_map", material->albedo_map);
	impl_alice_save_material_texture(&material_table, "normal_map", material->normal_map);
	impl_alice_save_material_texture(&material_table, "roughness_map", material->roughness_map);
	impl_alice_save_material_texture(&material_table, "metallic_map", material->metallic_map);
	impl_alice_save_material_texture(&material_table, "ao_map", material->ambient_occlusion_map);

	alice_RGBColor color = alice_rgb_color_from_color(material->albedo);

	alice_DTable color_table = alice_new_empty_dtable("albedo");
	alice_DTable r_table = alice_new_number_dtable("r", color.r);
	alice_DTable g_table = alice_new_number_dtable("g", color.g);
	alice_DTable b_table = alice_new_number_dtable("b", color.b);

	alice_dtable_add_child(&color_table, r_table);
	alice_dtable_add_child(&color_table, g_table);
	alice_dtable_add_child(&color_table, b_table);

	alice_dtable_add_child(&material_table, color_table);

	alice_DTable metallic_table = alice_new_number_dtable("metallic", material->metallic);
	alice_DTable roughness_table = alice_new_number_dtable("roughness", material->roughness);
	alice_DTable emissive_table = alice_new_number_dtable("emissive", material->emissive);

	alice_dtable_add_child(&material_table, metallic_table);
	alice_dtable_add_child(&material_table, roughness_table);
	alice_dtable_add_child(&material_table, emissive_table);

	alice_dtable_add_child(table, material_table);
}

static void alice_save_phong_matarial(alice_DTable* table, alice_PhongMaterial* material) {
	assert(table);
	assert(material);

	alice_DTable material_table = alice_new_empty_dtable("phong_material");

	impl_alice_save_material_texture(&material_table, "diffuse_map", material->diffuse_map);

	alice_RGBColor color = alice_rgb_color_from_color(material->diffuse);

	alice_DTable color_table = alice_new_empty_dtable("diffuse");
	alice_DTable r_table = alice_new_number_dtable("r", color.r);
	alice_DTable g_table = alice_new_number_dtable("g", color.g);
	alice_DTable b_table = alice_new_number_dtable("b", color.b);

	alice_dtable_add_child(&color_table, r_table);
	alice_dtable_add_child(&color_table, g_table);
	alice_dtable_add_child(&color_table, b_table);

	alice_dtable_add_child(&material_table, color_table);

	alice_dtable_add_child(table, material_table);
}

void alice_save_material(alice_Material* material, const char* path) {
	assert(material);

	alice_DTable material_table = alice_new_empty_dtable("material");

	const char* shader_path = alice_get_shader_resource_filename(material->shader);
	alice_DTable shader_table = alice_new_string_dtable("shader", shader_path);
	alice_dtable_add_child(&material_table, shader_table);

	switch (material->type) {
		case ALICE_MATERIAL_PBR:
			alice_save_pbr_material(&material_table, &material->as.pbr);
			break;
		case ALICE_MATERIAL_PHONG:
			alice_save_phong_matarial(&material_table, &material->as.phong);
			break;
		default: break;
	}

	alice_write_dtable(&material_table, path);

	alice_deinit_dtable(&material_table);
}

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alice/entity.h"
#include "alice/graphics.h"
#include "alice/scripting.h"
#include "alice/physics.h"

alice_m4f_t alice_get_entity_transform(alice_scene_t* scene, alice_entity_t* entity) {
	assert(entity);

	alice_m4f_t matrix = alice_m4f_identity();

	matrix = alice_m4f_translate(matrix, entity->position);

	matrix = alice_m4f_rotate(matrix, entity->rotation.z, (alice_v3f_t){0.0f, 0.0f, 1.0f});
	matrix = alice_m4f_rotate(matrix, entity->rotation.y, (alice_v3f_t){0.0f, 1.0f, 0.0f});
	matrix = alice_m4f_rotate(matrix, entity->rotation.x, (alice_v3f_t){1.0f, 0.0f, 0.0f});

	matrix = alice_m4f_scale(matrix, entity->scale);

	if (entity->parent != alice_null_entity_handle) {
		alice_entity_t* parent_ptr = alice_get_entity_ptr(scene, entity->parent);
		matrix = alice_m4f_multiply(alice_get_entity_transform(scene, parent_ptr), matrix);
	}

	return matrix;
}

static alice_m4f_t alice_compute_entity_transform(alice_scene_t* scene, alice_m4f_t parent,
		alice_entity_t* entity) {
	assert(scene);

	alice_m4f_t matrix = alice_m4f_identity();

	matrix = alice_m4f_translate(matrix, entity->position);

	matrix = alice_m4f_rotate(matrix, entity->rotation.z, (alice_v3f_t){0.0f, 0.0f, 1.0f});
	matrix = alice_m4f_rotate(matrix, entity->rotation.y, (alice_v3f_t){0.0f, 1.0f, 0.0f});
	matrix = alice_m4f_rotate(matrix, entity->rotation.x, (alice_v3f_t){1.0f, 0.0f, 0.0f});

	matrix = alice_m4f_scale(matrix, entity->scale);

	matrix = alice_m4f_multiply(parent, matrix);

	for (u32 i = 0; i < entity->child_count; i++) {
		alice_entity_t* child_ptr = alice_get_entity_ptr(scene, entity->children[i]);
		
		alice_compute_entity_transform(scene, matrix, child_ptr);
	}

	entity->transform = matrix;

	return matrix;
}

void alice_compute_scene_transforms(alice_scene_t* scene) {
	assert(scene);

	for (u32 i = 0; i < scene->pool_count; i++) {
		alice_entity_pool_t* pool = &scene->pools[i];
		for (u32 ii = 0; ii < pool->count; ii++) {
			alice_entity_handle_t handle = alice_new_entity_handle(ii, pool->type_id);

			alice_entity_t* ptr = alice_entity_pool_get(pool, ii);

			if (ptr->parent == alice_null_entity_handle) {
				alice_compute_entity_transform(scene, alice_m4f_identity(), ptr);
			}
		}
	}
}

void alice_entity_parent_to(alice_scene_t* scene, alice_entity_handle_t entity, alice_entity_handle_t parent) {
	assert(scene);

	alice_entity_t* entity_ptr = alice_get_entity_ptr(scene, entity);
	alice_entity_t* parent_ptr = alice_get_entity_ptr(scene, parent);

	if (parent_ptr->child_count >= parent_ptr->child_capacity) {
		parent_ptr->child_capacity = alice_grow_capacity(parent_ptr->child_capacity);
		parent_ptr->children = realloc(parent_ptr->children,
				parent_ptr->child_capacity * sizeof(alice_entity_handle_t));
	}

	entity_ptr->parent = parent;
	parent_ptr->children[parent_ptr->child_count++] = entity;
}

void alice_entity_add_child(alice_scene_t* scene, alice_entity_handle_t entity, alice_entity_handle_t child) {
	assert(scene);

	alice_entity_parent_to(scene, child, entity);
}

void alice_entity_remove_child(alice_scene_t* scene, alice_entity_handle_t entity, alice_entity_handle_t child) {
	assert(scene);

	alice_entity_t* entity_ptr = alice_get_entity_ptr(scene, entity);
	alice_entity_t* child_ptr = alice_get_entity_ptr(scene, child);

	child_ptr->parent = alice_null_entity_handle;

	i32 index_to_remove = -1;
	for (u32 i = 0; i < entity_ptr->child_count; i++) {
		if (entity_ptr->children[i] == child) {
			index_to_remove = i;
		}
	}

	if (index_to_remove != -1) {
		for (u32 i = index_to_remove; i < entity_ptr->child_count - 1; i++) {
			entity_ptr->children[i] = entity_ptr->children[i + 1];
		}
		entity_ptr->child_count--;
	} else {
		alice_log_warning("Child doesn't exist on this entity");
	}
}

void alice_entity_unparent(alice_scene_t* scene, alice_entity_handle_t entity) {
	assert(scene);

	alice_entity_t* entity_ptr = alice_get_entity_ptr(scene, entity);

	alice_entity_handle_t parent = entity_ptr->parent;

	if (parent != alice_null_entity_handle) {
		alice_entity_remove_child(scene, parent, entity);
	}
}

alice_entity_handle_t alice_find_entity_by_name(alice_scene_t* scene, alice_entity_handle_t parent_handle, const char* name) {
	assert(scene);

	alice_entity_t* parent = alice_get_entity_ptr(scene, parent_handle);

	if (parent == alice_null) {
		for (u32 i = 0; i < scene->pool_count; i++) {
			alice_entity_pool_t* pool = &scene->pools[i];
			for (u32 ii = 0; ii < pool->count; ii++) {
				alice_entity_handle_t handle = alice_new_entity_handle(ii, pool->type_id);

				alice_entity_t* ptr = alice_entity_pool_get(pool, ii);

				if (ptr->parent == alice_null_entity_handle && ptr->name &&
						strcmp(ptr->name, name) == 0) {
					return handle;
				}
			}
		}

		return alice_null_entity_handle;
	}

	for (u32 i = 0; i < parent->child_count; i++) {
		alice_entity_handle_t handle = parent->children[i];

		alice_entity_t* ptr = alice_get_entity_ptr(scene, handle);

		if (strcmp(ptr->name, name) == 0) {
			return handle;
		}
	}

	return alice_null_entity_handle;
}

alice_entity_handle_t alice_find_entity_by_path(alice_scene_t* scene, const char* path) {
	assert(scene);

	char* copied_path = alice_copy_string(path);

	char* token = strtok(copied_path, "/");
	alice_entity_handle_t last = alice_null;
	alice_entity_handle_t current_handle = alice_null_entity_handle;

	while (token) {
		current_handle = alice_find_entity_by_name(scene, last, token);
		if (current_handle == alice_null_entity_handle) {
			alice_log_error("Failed to find entity with path `%s'", path);
			return alice_null_entity_handle;
		}

		last = current_handle;

		token = strtok(alice_null, "/");
	}

	free(copied_path);

	return current_handle;
}

alice_v3f_t alice_get_entity_world_position(alice_scene_t* scene, alice_entity_t* entity) {
	assert(scene);
	assert(entity);

	return (alice_v3f_t) {
		entity->transform.elements[3][0],
		entity->transform.elements[3][1],
		entity->transform.elements[3][2]
	};
}

alice_v3f_t alice_get_entity_world_rotation(alice_scene_t* scene, alice_entity_t* entity) {
	assert(scene);
	assert(entity);

	alice_v3f_t result = entity->rotation;

	if (entity->parent != alice_null_entity_handle) {
		alice_entity_t* parent = alice_get_entity_ptr(scene, entity->parent);

		alice_v3f_t parent_rotation = alice_get_entity_world_rotation(
			scene, parent);

		result = (alice_v3f_t) {
			result.x + parent_rotation.x,
			result.y + parent_rotation.y,
			result.z + parent_rotation.z
		};
	}

	return result;
}

alice_v3f_t alice_get_entity_world_scale(alice_scene_t* scene, alice_entity_t* entity) {
	assert(scene);
	assert(entity);

	alice_v3f_t result = entity->rotation;

	if (entity->parent != alice_null_entity_handle) {
		alice_entity_t* parent = alice_get_entity_ptr(scene, entity->parent);

		alice_v3f_t parent_rotation = alice_get_entity_world_rotation(
			scene, parent);

		result = (alice_v3f_t) {
			result.x + parent_rotation.x,
			result.y + parent_rotation.y,
			result.z + parent_rotation.z
		};
	}

	return result;
}

alice_entity_handle_t alice_new_entity_handle(u32 id, u32 type_id) {
	return ((alice_entity_handle_t)id << 32) | ((alice_entity_handle_t)type_id);
}

u32 alice_get_entity_handle_type(alice_entity_handle_t handle) {
	return (u32)handle;
}

u32 alice_get_entity_handle_id(alice_entity_handle_t handle) {
	return handle >> 32;
}

void alice_init_entity_pool(alice_entity_pool_t* pool, u32 type_id, u32 element_size) {
	assert(pool);

	pool->create = alice_null;
	pool->destroy = alice_null;

	pool->type_id = type_id;
	pool->element_size = element_size;

	pool->data = alice_null;
	pool->count = 0;
	pool->capacity = 0;
}

void alice_deinit_entity_pool(alice_entity_pool_t* pool) {
	assert(pool);

	if (pool->capacity > 0) {
		free(pool->data);
	}

	pool->create = alice_null;
	pool->destroy = alice_null;

	pool->type_id = 0;
	pool->element_size = 0;

	pool->data = alice_null;
	pool->count = 0;
	pool->capacity = 0;
}

u32 alice_entity_pool_add(alice_entity_pool_t* pool) {
	assert(pool);

	if (pool->count >= pool->capacity) {
		pool->capacity = alice_grow_capacity(pool->capacity);
		pool->data = realloc(pool->data, pool->capacity * pool->element_size);
	}

	return pool->count++;
}

void* alice_entity_pool_get(alice_entity_pool_t* pool, u32 index) {
	assert(pool);

	return &((char*)pool->data)[index * pool->element_size];
}

void alice_entity_pool_remove(alice_entity_pool_t* pool, u32 index) {
	assert(pool);

	memmove(
		&((char*)pool->data)[index * pool->element_size],
		&((char*)pool->data)[(pool->count - 1) * pool->element_size],
		pool->element_size);

	pool->count--;
}

alice_scene_t* alice_new_scene(const char* script_assembly) {
	alice_scene_t* new = malloc(sizeof(alice_scene_t));

	*new = (alice_scene_t){
		.pools = alice_null,
		.pool_count = 0,
		.pool_capacity = 0,

		.script_context = alice_new_script_context(new, script_assembly),

		.renderer = alice_null,
		.physics_engine = alice_null
	};

	alice_register_entity_type(new, alice_entity_t);
	alice_register_entity_type(new, alice_camera_3d_t);
	alice_register_entity_type(new, alice_camera_2d_t);
	alice_register_entity_type(new, alice_renderable_3d_t);
	alice_register_entity_type(new, alice_point_light_t);
	alice_register_entity_type(new, alice_directional_light_t);
	alice_register_entity_type(new, alice_rigidbody_3d_t);
	alice_register_entity_type(new, alice_sprite_2d_t);
	alice_register_entity_type(new, alice_tilemap_t);

	alice_set_entity_create_function(new, alice_renderable_3d_t, alice_on_renderable_3d_create);
	alice_set_entity_destroy_function(new, alice_renderable_3d_t, alice_on_renderable_3d_destroy);

	alice_set_entity_create_function(new, alice_tilemap_t, alice_on_tilemap_create);
	alice_set_entity_destroy_function(new, alice_tilemap_t, alice_on_tilemap_destroy);

	alice_set_entity_create_function(new, alice_rigidbody_3d_t, alice_on_rigidbody_3d_create);

	return new;
}

static void alice_free_entity(alice_scene_t* scene, alice_entity_t* ptr) {
	assert(ptr);

	if (ptr->script) {
		alice_delete_script(scene->script_context, ptr->script);
	}

	if (ptr->child_capacity > 0) {
		free(ptr->children);
	}

	if (ptr->name) {
		free(ptr->name);
	}
}

void alice_free_scene(alice_scene_t* scene) {
	assert(scene);

	alice_free_scripts(scene->script_context);
	alice_free_script_context(scene->script_context);

	if (scene->renderer) {
		alice_free_scene_renderer_3d(scene->renderer);
	}

	if (scene->renderer_2d) {
		alice_free_scene_renderer_2d(scene->renderer_2d);
	}

	if (scene->physics_engine) {
		alice_free_physics_engine(scene->physics_engine);
	}

	for (u32 i = 0; i < scene->pool_count; i++) {
		alice_entity_pool_t* pool = &scene->pools[i];

		for (u32 i = 0; i < pool->count; i++) {
			alice_entity_handle_t handle = alice_new_entity_handle(i, pool->type_id);
			alice_entity_t* ptr = alice_entity_pool_get(pool, i);

			if (pool->destroy) {
				pool->destroy(scene, handle, ptr);
			}

			alice_free_entity(scene, ptr);
		}

		alice_deinit_entity_pool(&scene->pools[i]);
	}

	if (scene->pool_capacity > 0) {
		free(scene->pools);
	}

	free(scene);
}

alice_entity_pool_t* alice_get_entity_pool(alice_scene_t* scene, u32 type_id) {
	assert(scene);

	for (u32 i = 0; i < scene->pool_count; i++) {
		if (scene->pools[i].type_id == type_id) {
			return &scene->pools[i];
		}
	}

	alice_log_error("Entity type (%u) not registered", type_id);

	return NULL;
}

void impl_alice_register_entity_type(alice_scene_t* scene, alice_type_info_t type) {
	assert(scene);

	for (u32 i = 0; i < scene->pool_count; i++) {
		alice_entity_pool_t* pool = &scene->pools[i];
		if (pool->type_id == type.id) {
			alice_log_warning("Entity type (%u) already registered", type.id);
			return;
		}
	}

	if (scene->pool_count >= scene->pool_capacity) {
		scene->pool_capacity = alice_grow_capacity(scene->pool_capacity);
		scene->pools = realloc(scene->pools, scene->pool_capacity * sizeof(alice_entity_pool_t));
	}

	alice_init_entity_pool(&scene->pools[scene->pool_count++], type.id, type.size);
}

alice_entity_handle_t impl_alice_new_entity(alice_scene_t* scene, alice_type_info_t type) {
	assert(scene);

	alice_entity_pool_t* pool = alice_get_entity_pool(scene, type.id);
	if (!pool) { return alice_null_entity_handle; }

	u32 index = alice_entity_pool_add(pool);

	alice_entity_handle_t new = alice_new_entity_handle(index, type.id);

	void* e_ptr = alice_entity_pool_get(pool, alice_get_entity_handle_id(new));
	*((alice_entity_t*)e_ptr) = (alice_entity_t) {
		.name = alice_null,
		.position = (alice_v3f_t){0.0f, 0.0f, 0.0f},
		.rotation = (alice_v3f_t){0.0f, 0.0f, 0.0f},
		.scale = (alice_v3f_t){1.0f, 1.0f, 1.0f},

		.script = alice_null,

		.parent = alice_null_entity_handle,
		.children = alice_null,
		.child_count = 0,
		.child_capacity = 0
	};

	if (pool->create) {
		pool->create(scene, new, e_ptr);
	}

	return new;
}

void alice_destroy_entity(alice_scene_t* scene, alice_entity_handle_t handle) {
	assert(scene);

	alice_entity_pool_t* pool = alice_get_entity_pool(scene, alice_get_entity_handle_type(handle));
	alice_entity_t* ptr = alice_get_entity_ptr(scene, handle);

	while (ptr->child_count > 0) {
		alice_destroy_entity(scene, ptr->children[0]);
	}

	if (ptr->parent != alice_null_entity_handle) {
		alice_entity_remove_child(scene, ptr->parent, handle);
	}

	alice_free_entity(scene, ptr);

	alice_entity_pool_remove(pool, alice_get_entity_handle_id(handle));
}

void* alice_get_entity_ptr(alice_scene_t* scene, alice_entity_handle_t handle) {
	assert(scene);

	alice_entity_pool_t* pool = alice_get_entity_pool(scene, alice_get_entity_handle_type(handle));

	return alice_entity_pool_get(pool, alice_get_entity_handle_id(handle));
}

void impl_alice_set_entity_create_function(alice_scene_t* scene,
		alice_type_info_t type, alice_entity_create_f function) {
	assert(scene);
	assert(function);

	alice_entity_pool_t* pool = alice_get_entity_pool(scene, type.id);

	pool->create = function;
}

void impl_alice_set_entity_destroy_function(alice_scene_t* scene,
		alice_type_info_t type, alice_entity_destroy_f function) {
	assert(scene);
	assert(function);

	alice_entity_pool_t* pool = alice_get_entity_pool(scene, type.id);

	pool->destroy = function;
}

alice_entity_iter_t impl_alice_new_entity_iter(alice_scene_t* scene, alice_type_info_t type) {
	assert(scene);

	alice_entity_pool_t* pool = alice_get_entity_pool(scene, type.id);

	alice_entity_handle_t current_handle = alice_null_entity_handle;
	void* current_ptr = alice_null;
	if (pool->count > 0) {
		current_handle = alice_new_entity_handle(0, type.id);
		current_ptr = alice_entity_pool_get(pool, 0);
	}

	return (alice_entity_iter_t) {
		.index = 0,

		.pool = pool,

		.scene = scene,

		.type = type,

		.current = current_handle,
		.current_ptr = current_ptr
	};
}

void alice_entity_iter_next(alice_entity_iter_t* iter) {
	assert(iter);

	iter->index++;

	iter->current = alice_new_entity_handle(iter->index, iter->type.id);
	iter->current_ptr = alice_entity_pool_get(iter->pool, iter->index);
}

bool alice_entity_iter_valid(alice_entity_iter_t* iter) {
	assert(iter);

	return iter->index < iter->pool->count;
}

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alice/entity.h"
#include "alice/graphics.h"
#include "alice/scripting.h"
#include "alice/physics.h"

alice_m4f alice_get_entity_transform(alice_Scene* scene, alice_Entity* entity) {
	assert(entity);

	alice_m4f matrix = alice_m4f_identity();

	matrix = alice_m4f_translate(matrix, entity->position);

	matrix = alice_m4f_rotate(matrix, entity->rotation.z, (alice_v3f){0.0f, 0.0f, 1.0f});
	matrix = alice_m4f_rotate(matrix, entity->rotation.y, (alice_v3f){0.0f, 1.0f, 0.0f});
	matrix = alice_m4f_rotate(matrix, entity->rotation.x, (alice_v3f){1.0f, 0.0f, 0.0f});

	matrix = alice_m4f_scale(matrix, entity->scale);

	if (entity->parent != alice_null_entity_handle) {
		alice_Entity* parent_ptr = alice_get_entity_ptr(scene, entity->parent);
		matrix = alice_m4f_multiply(alice_get_entity_transform(scene, parent_ptr), matrix);
	}

	return matrix;
}

void alice_entity_parent_to(alice_Scene* scene, alice_EntityHandle entity, alice_EntityHandle parent) {
	assert(scene);

	alice_Entity* entity_ptr = alice_get_entity_ptr(scene, entity);
	alice_Entity* parent_ptr = alice_get_entity_ptr(scene, parent);

	if (parent_ptr->child_count >= parent_ptr->child_capacity) {
		parent_ptr->child_capacity = alice_grow_capacity(parent_ptr->child_capacity);
		parent_ptr->children = realloc(parent_ptr->children,
				parent_ptr->child_capacity * sizeof(alice_EntityHandle));
	}

	entity_ptr->parent = parent;
	parent_ptr->children[parent_ptr->child_count++] = entity;
}

void alice_entity_add_child(alice_Scene* scene, alice_EntityHandle entity, alice_EntityHandle child) {
	assert(scene);

	alice_entity_parent_to(scene, child, entity);
}

void alice_entity_remove_child(alice_Scene* scene, alice_EntityHandle entity, alice_EntityHandle child) {
	assert(scene);

	alice_Entity* entity_ptr = alice_get_entity_ptr(scene, entity);
	alice_Entity* child_ptr = alice_get_entity_ptr(scene, child);

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

void alice_entity_unparent(alice_Scene* scene, alice_EntityHandle entity) {
	assert(scene);

	alice_Entity* entity_ptr = alice_get_entity_ptr(scene, entity);

	alice_EntityHandle parent = entity_ptr->parent;

	if (parent != alice_null_entity_handle) {
		alice_entity_remove_child(scene, parent, entity);
	}
}

alice_EntityHandle alice_find_entity_by_name(alice_Scene* scene, alice_EntityHandle parent_handle, const char* name) {
	assert(scene);

	alice_Entity* parent = alice_get_entity_ptr(scene, parent_handle);

	if (parent == alice_null) {
		for (u32 i = 0; i < scene->pool_count; i++) {
			alice_EntityPool* pool = &scene->pools[i];
			for (u32 ii = 0; ii < pool->count; ii++) {
				alice_EntityHandle handle = alice_new_entity_handle(ii, pool->type_id);

				alice_Entity* ptr = alice_entity_pool_get(pool, ii);

				if (ptr->parent == alice_null_entity_handle && ptr->name &&
						strcmp(ptr->name, name) == 0) {
					return handle;
				}
			}
		}

		return alice_null_entity_handle;
	}

	for (u32 i = 0; i < parent->child_count; i++) {
		alice_EntityHandle handle = parent->children[i];

		alice_Entity* ptr = alice_get_entity_ptr(scene, handle);

		if (strcmp(ptr->name, name) == 0) {
			return handle;
		}
	}

	return alice_null_entity_handle;
}

alice_EntityHandle alice_find_entity_by_path(alice_Scene* scene, const char* path) {
	assert(scene);

	char* copied_path = alice_copy_string(path);

	char* token = strtok(copied_path, "/");
	alice_EntityHandle last = alice_null;
	alice_EntityHandle current_handle = alice_null_entity_handle;

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

alice_v3f alice_get_entity_world_position(alice_Scene* scene, alice_Entity* entity) {
	assert(scene);
	assert(entity);

	alice_m4f matrix = alice_get_entity_transform(scene, entity);

	return (alice_v3f) {
		matrix.elements[3][0],
		matrix.elements[3][1],
		matrix.elements[3][2]
	};
}

alice_v3f alice_get_entity_world_rotation(alice_Scene* scene, alice_Entity* entity) {
	assert(scene);
	assert(entity);

	alice_v3f result = entity->rotation;

	if (entity->parent != alice_null_entity_handle) {
		alice_Entity* parent = alice_get_entity_ptr(scene, entity->parent);

		alice_v3f parent_rotation = alice_get_entity_world_rotation(
			scene, parent);

		result = (alice_v3f) {
			result.x + parent_rotation.x,
			result.y + parent_rotation.y,
			result.z + parent_rotation.z
		};
	}

	return result;
}

alice_v3f alice_get_entity_world_scale(alice_Scene* scene, alice_Entity* entity) {
	assert(scene);
	assert(entity);

	alice_v3f result = entity->rotation;

	if (entity->parent != alice_null_entity_handle) {
		alice_Entity* parent = alice_get_entity_ptr(scene, entity->parent);

		alice_v3f parent_rotation = alice_get_entity_world_rotation(
			scene, parent);

		result = (alice_v3f) {
			result.x + parent_rotation.x,
			result.y + parent_rotation.y,
			result.z + parent_rotation.z
		};
	}

	return result;
}

alice_EntityHandle alice_new_entity_handle(u32 id, u32 type_id) {
	return ((alice_EntityHandle)id << 32) | ((alice_EntityHandle)type_id);
}

u32 alice_get_entity_handle_type(alice_EntityHandle handle) {
	return (u32)handle;
}

u32 alice_get_entity_handle_id(alice_EntityHandle handle) {
	return handle >> 32;
}

void alice_init_entity_pool(alice_EntityPool* pool, u32 type_id, u32 element_size) {
	assert(pool);

	pool->create = alice_null;
	pool->destroy = alice_null;

	pool->type_id = type_id;
	pool->element_size = element_size;

	pool->data = alice_null;
	pool->count = 0;
	pool->capacity = 0;
}

void alice_deinit_entity_pool(alice_EntityPool* pool) {
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

u32 alice_entity_pool_add(alice_EntityPool* pool) {
	assert(pool);

	if (pool->count >= pool->capacity) {
		pool->capacity = alice_grow_capacity(pool->capacity);
		pool->data = realloc(pool->data, pool->capacity * pool->element_size);
	}

	return pool->count++;
}

void* alice_entity_pool_get(alice_EntityPool* pool, u32 index) {
	assert(pool);

	return &((char*)pool->data)[index * pool->element_size];
}

void alice_entity_pool_remove(alice_EntityPool* pool, u32 index) {
	assert(pool);

	memmove(
		&((char*)pool->data)[index * pool->element_size],
		&((char*)pool->data)[(pool->count - 1) * pool->element_size],
		pool->element_size);

	pool->count--;
}

alice_Scene* alice_new_scene(const char* script_assembly) {
	alice_Scene* new = malloc(sizeof(alice_Scene));

	*new = (alice_Scene){
		.pools = alice_null,
		.pool_count = 0,
		.pool_capacity = 0,

		.script_context = alice_new_script_context(new, script_assembly)
	};

	alice_register_entity_type(new, alice_Entity);
	alice_register_entity_type(new, alice_Camera3D);
	alice_register_entity_type(new, alice_Renderable3D);
	alice_register_entity_type(new, alice_PointLight);
	alice_register_entity_type(new, alice_DirectionalLight);
	alice_register_entity_type(new, alice_Rigidbody3D);

	alice_set_entity_create_function(new, alice_Renderable3D, alice_on_renderable_3d_create);
	alice_set_entity_destroy_function(new, alice_Renderable3D, alice_on_renderable_3d_destroy);

	alice_set_entity_create_function(new, alice_Rigidbody3D, alice_on_rigidbody_3d_create);

	return new;
}

void alice_free_scene(alice_Scene* scene) {
	assert(scene);

	alice_free_scripts(scene->script_context);

	for (u32 i = 0; i < scene->pool_count; i++) {
		alice_EntityPool* pool = &scene->pools[i];

		if (pool->destroy) {
			for (u32 i = 0; i < pool->count; i++) {
				alice_EntityHandle handle = alice_new_entity_handle(i, pool->type_id);

				pool->destroy(scene, handle, alice_entity_pool_get(pool, i));
			}
		}

		alice_deinit_entity_pool(&scene->pools[i]);
	}

	if (scene->pool_capacity > 0) {
		free(scene->pools);
	}

	free(scene);
}

alice_EntityPool* alice_get_entity_pool(alice_Scene* scene, u32 type_id) {
	assert(scene);

	for (u32 i = 0; i < scene->pool_count; i++) {
		if (scene->pools[i].type_id == type_id) {
			return &scene->pools[i];
		}
	}

	alice_log_error("Entity type (%u) not registered", type_id);

	return NULL;
}

void impl_alice_register_entity_type(alice_Scene* scene, alice_TypeInfo type) {
	assert(scene);

	for (u32 i = 0; i < scene->pool_count; i++) {
		alice_EntityPool* pool = &scene->pools[i];
		if (pool->type_id == type.id) {
			alice_log_warning("Entity type (%u) already registered", type.id);
			return;
		}
	}

	if (scene->pool_count >= scene->pool_capacity) {
		scene->pool_capacity = alice_grow_capacity(scene->pool_capacity);
		scene->pools = realloc(scene->pools, scene->pool_capacity * sizeof(alice_EntityPool));
	}

	alice_init_entity_pool(&scene->pools[scene->pool_count++], type.id, type.size);
}

alice_EntityHandle impl_alice_new_entity(alice_Scene* scene, alice_TypeInfo type) {
	assert(scene);

	alice_EntityPool* pool = alice_get_entity_pool(scene, type.id);
	if (!pool) { return alice_null_entity_handle; }

	u32 index = alice_entity_pool_add(pool);

	alice_EntityHandle new = alice_new_entity_handle(index, type.id);

	void* e_ptr = alice_entity_pool_get(pool, alice_get_entity_handle_id(new));
	*((alice_Entity*)e_ptr) = (alice_Entity) {
		.name = alice_null,
		.position = (alice_v3f){0.0f, 0.0f, 0.0f},
		.rotation = (alice_v3f){0.0f, 0.0f, 0.0f},
		.scale = (alice_v3f){1.0f, 1.0f, 1.0f},

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

void alice_destroy_entity(alice_Scene* scene, alice_EntityHandle handle) {
	assert(scene);

	alice_EntityPool* pool = alice_get_entity_pool(scene, alice_get_entity_handle_type(handle));
	alice_Entity* ptr = alice_get_entity_ptr(scene, handle);

	for (u32 i = 0; i < ptr->child_count; i++) {
		alice_destroy_entity(scene, ptr->children[i]);
	}

	if (ptr->script) {
		alice_delete_script(scene->script_context, ptr->script);
	}

	if (pool->destroy) {
		pool->destroy(scene, handle, ptr);
	}

	if (ptr->name) {
		free(ptr->name);
	}

	alice_entity_pool_remove(pool, alice_get_entity_handle_id(handle));
}

void* alice_get_entity_ptr(alice_Scene* scene, alice_EntityHandle handle) {
	assert(scene);

	alice_EntityPool* pool = alice_get_entity_pool(scene, alice_get_entity_handle_type(handle));

	return alice_entity_pool_get(pool, alice_get_entity_handle_id(handle));
}

void impl_alice_set_entity_create_function(alice_Scene* scene,
		alice_TypeInfo type, alice_EntityCreateFunction function) {
	assert(scene);
	assert(function);

	alice_EntityPool* pool = alice_get_entity_pool(scene, type.id);

	pool->create = function;
}

void impl_alice_set_entity_destroy_function(alice_Scene* scene,
		alice_TypeInfo type, alice_EntityDestroyFunction function) {
	assert(scene);
	assert(function);

	alice_EntityPool* pool = alice_get_entity_pool(scene, type.id);

	pool->destroy = function;
}

alice_EntityIter impl_alice_new_entity_iter(alice_Scene* scene, alice_TypeInfo type) {
	assert(scene);

	alice_EntityPool* pool = alice_get_entity_pool(scene, type.id);

	alice_EntityHandle current_handle = alice_null_entity_handle;
	void* current_ptr = alice_null;
	if (pool->count > 0) {
		current_handle = alice_new_entity_handle(0, type.id);
		current_ptr = alice_entity_pool_get(pool, 0);
	}

	return (alice_EntityIter) {
		.index = 0,

		.pool = pool,

		.scene = scene,

		.type = type,

		.current = current_handle,
		.current_ptr = current_ptr
	};
}

void alice_entity_iter_next(alice_EntityIter* iter) {
	assert(iter);

	iter->index++;

	iter->current = alice_new_entity_handle(iter->index, iter->type.id);
	iter->current_ptr = alice_entity_pool_get(iter->pool, iter->index);
}

bool alice_entity_iter_valid(alice_EntityIter* iter) {
	assert(iter);

	return iter->index < iter->pool->count;
}

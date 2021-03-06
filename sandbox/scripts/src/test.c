#include <alice/core.h>
#include <alice/entity.h>
#include <alice/input.h>
#include <alice/graphics.h>
#include <alice/resource.h>

typedef struct test_script_t {
	float thing;
} test_script_t;

ALICE_API u32 ALICE_CALL get_test_script_instance_size() {
	return sizeof(test_script_t);
}

ALICE_API void ALICE_CALL
on_test_script_init(alice_scene_t* scene, alice_entity_handle_t entity, void* instance) {
	alice_log("Hello, world");
}

ALICE_API void ALICE_CALL
on_test_script_update(alice_scene_t* scene, alice_entity_handle_t entity, void* instance, double timestep) {
	alice_entity_t* entity_ptr = alice_get_entity_ptr(scene, entity);

	if (alice_key_pressed(ALICE_KEY_LEFT)) {
		entity_ptr->position.x += 3.0f * timestep;
	} else if (alice_key_pressed(ALICE_KEY_RIGHT)) {
		entity_ptr->position.x -= 3.0f * timestep;
	}

	if (alice_key_pressed(ALICE_KEY_UP)) {
		entity_ptr->position.y += 3.0f * timestep;
	}
	else if (alice_key_pressed(ALICE_KEY_DOWN)) {
		entity_ptr->position.y -= 3.0f * timestep;
	}

	if (alice_key_just_pressed(ALICE_KEY_SPACE) &&
		alice_get_entity_handle_type(entity) == alice_get_type_info(alice_renderable_3d_t).id) {
		alice_renderable_3d_t* renderable = (alice_renderable_3d_t*)entity_ptr;

		renderable->materials[0]->as.pbr.albedo = ALICE_COLOR_YELLOW;
	}

	if (alice_key_just_pressed(ALICE_KEY_D)) {
		alice_destroy_entity(scene, entity);
	}
}

ALICE_API void ALICE_CALL
on_test_script_free(alice_scene_t* scene, alice_entity_handle_t entity, void* instance) {
	alice_log("Test script free");
}
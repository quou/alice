#include <alice/core.h>
#include <alice/entity.h>
#include <alice/input.h>
#include <alice/graphics.h>
#include <alice/resource.h>
#include <alice/physics.h>

typedef struct monkey_physics_t {
	bool panic;
} monkey_physics_t;

ALICE_API u32 ALICE_CALL alice_get_monkey_physics_size() {
	return sizeof(monkey_physics_t);
}

ALICE_API void ALICE_CALL monkey_physics_init(alice_scene_t* scene, alice_entity_handle_t handle, void* instance) {
	monkey_physics_t* mp = instance;

	mp->panic = false;
	if (alice_get_entity_handle_type(handle) != alice_get_type_info(alice_rigidbody_3d_t).id) {
		mp->panic = true;
	}
}

ALICE_API void ALICE_CALL
monkey_physics_update(alice_scene_t* scene, alice_entity_handle_t handle, void* instance, double timestep) {
	monkey_physics_t* mp = instance;

	if (mp->panic) {
		return;
	}

	alice_rigidbody_3d_t* rigidbody = alice_get_entity_ptr(scene, handle);

	if (alice_key_pressed(ALICE_KEY_SPACE)) {
		rigidbody->force.y = 100.0f;
	} else {
		rigidbody->force.y = 0.0f;
	}

	if (alice_key_pressed(ALICE_KEY_LEFT)) {
		rigidbody->force.x = 20.0f;
	} else if (alice_key_pressed(ALICE_KEY_RIGHT)) {
		rigidbody->force.x = -20.0f;
	} else {
		rigidbody->force.x = 0.0f;
	}

	if (alice_key_pressed(ALICE_KEY_UP)) {
		rigidbody->force.z = 20.0f;
	} else if (alice_key_pressed(ALICE_KEY_DOWN)) {
		rigidbody->force.z = -20.0f;
	} else {
		rigidbody->force.z = 0.0f;
	}
}


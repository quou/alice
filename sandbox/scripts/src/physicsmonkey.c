#include <alice/core.h>
#include <alice/entity.h>
#include <alice/input.h>
#include <alice/graphics.h>
#include <alice/resource.h>
#include <alice/physics.h>

typedef struct MonkeyPhysics {
	bool panic;
} MonkeyPhysics;

ALICE_API u32 ALICE_CALL alice_get_monkey_physics_size() {
	return sizeof(MonkeyPhysics);
}

ALICE_API void ALICE_CALL monkey_physics_init(alice_Scene* scene, alice_EntityHandle handle, void* instance) {
	MonkeyPhysics* mp = instance;

	mp->panic = false;
	if (alice_get_entity_handle_type(handle) != alice_get_type_info(alice_Rigidbody3D).id) {
		mp->panic = true;
	}
}

ALICE_API void ALICE_CALL
monkey_physics_update(alice_Scene* scene, alice_EntityHandle handle, void* instance, double timestep) {
	MonkeyPhysics* mp = instance;

	if (mp->panic) {
		return;
	}

	alice_Rigidbody3D* rigidbody = alice_get_entity_ptr(scene, handle);

	if (alice_key_pressed(ALICE_KEY_SPACE)) {
		rigidbody->velocity.y = 10.0f;
	}

	if (alice_key_pressed(ALICE_KEY_LEFT)) {
		rigidbody->velocity.x = 2.0f;
	} else if (alice_key_pressed(ALICE_KEY_RIGHT)) {
		rigidbody->velocity.x = -2.0f;
	}
}


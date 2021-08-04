#include <alice/core.h>
#include <alice/entity.h>
#include <alice/input.h>
#include <alice/graphics.h>
#include <alice/resource.h>
#include <alice/application.h>

ALICE_API void ALICE_CALL
spawn_monkeys(alice_scene_t* scene, alice_entity_handle_t entity, void* instance) {
	alice_model_t* monkey_model = alice_load_model("models/monkey.glb");

	for (u32 x = 0; x < 25; x++) {
		for (u32 y = 0; y < 25; y++) {
			alice_entity_handle_t new_monkey_handle = alice_new_entity(scene, alice_renderable_3d_t);

			alice_renderable_3d_t* new_monkey = alice_get_entity_ptr(scene, new_monkey_handle);

			new_monkey->base.position.x = x * 3.0f;
			new_monkey->base.position.z = y * 2.0f;

			new_monkey->model = monkey_model;

			if (alice_random_int(0, 1) == 0) {
				alice_renderable_3d_add_material(new_monkey, "materials/pirategold.mat");
			} else {
				alice_renderable_3d_add_material(new_monkey, "default_material");
			}

			alice_entity_parent_to(scene, new_monkey_handle, entity);
		}
	}
}

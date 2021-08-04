#include <alice/core.h>
#include <alice/entity.h>
#include <alice/input.h>
#include <alice/graphics.h>
#include <alice/resource.h>
#include <alice/application.h>

typedef struct camera_controller_t {
	bool panic;

	bool first_move;
	alice_v2i_t old_mouse;
	alice_v2i_t mouse_delta;
} camera_controller_t;

ALICE_API u32 ALICE_CALL get_camera_controller_size() {
	return sizeof(camera_controller_t);
}

ALICE_API void ALICE_CALL camera_controller_init(alice_scene_t* scene, alice_entity_handle_t entity, void* instance) {
	camera_controller_t* controller = instance;

	if (alice_get_entity_handle_type(entity) != alice_get_type_info(alice_camera_3d_t).id) {
		alice_log_error("Camera controller must be applied to a 3D camera entity");

		controller->panic = true;
		return;
	} else {
		controller->panic = false;
	}

	alice_hide_mouse();
}

ALICE_API void ALICE_CALL camera_controller_update(alice_scene_t* scene, alice_entity_handle_t entity, void* instance, double timestep) {
	camera_controller_t* controller = instance;

	if (controller->panic) { return; };

	if (alice_key_just_pressed(ALICE_KEY_ESCAPE)) {
		alice_show_mouse();
	} else if (alice_mouse_button_just_pressed(ALICE_MOUSE_BUTTON_LEFT)) {
		alice_hide_mouse();
	}

	alice_camera_3d_t* camera = alice_get_entity_ptr(scene, entity);

	if (controller->first_move) {
		controller->first_move = false;
		controller->old_mouse = alice_get_mouse_position();
	}

	alice_v2i_t mouse_pos = alice_get_mouse_position();

	i32 change_x = mouse_pos.x - controller->old_mouse.x;
	i32 change_y = controller->old_mouse.y - mouse_pos.y;

	controller->old_mouse = mouse_pos;

	camera->base.rotation.y -= (float)change_x * 0.1f;
	camera->base.rotation.x += (float)change_y * 0.1f;

	if (camera->base.rotation.x >= 89.0f) {
		camera->base.rotation.x = 89.0f;
	}

	if (camera->base.rotation.x <= -89.0f) {
		camera->base.rotation.x = -89.0f;
	}
}

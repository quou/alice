#include <math.h>

#include <alice/core.h>
#include <alice/entity.h>
#include <alice/input.h>
#include <alice/physics.h>
#include <alice/graphics.h>
#include <alice/application.h>

typedef struct fps_controller_t {
	bool panic;
	float speed;
	float jump_height;

	alice_camera_3d_t* camera;

	bool first_move;
	alice_v2i_t old_mouse;
	alice_v2i_t mouse_delta;
} fps_controller_t;

ALICE_API u32 ALICE_CALL get_fps_controller_size() {
	return sizeof(fps_controller_t);
}

ALICE_API void ALICE_CALL
init_fps_controller(alice_scene_t* scene, alice_entity_handle_t handle, void* instance) {
	fps_controller_t* fps = instance;

	fps->panic = false;

	if (alice_get_entity_handle_type(handle) != alice_get_type_info(alice_rigidbody_3d_t).id) {
		alice_log_error("FPS controller script must be applied to a rigidbody");
	}

	fps->speed = 30.0f;
	fps->jump_height = 40.0f;

	fps->camera = alice_get_entity_ptr(scene, alice_find_entity_by_name(scene, handle, "camera"));

	fps->first_move = true;

	alice_hide_mouse();
}

ALICE_API void ALICE_CALL
update_fps_controller(alice_scene_t* scene, alice_entity_handle_t handle, void* instance, double timestep) {
	fps_controller_t* fps = instance;

	if (fps->panic) { return; }

	if (alice_key_just_pressed(ALICE_KEY_ESCAPE)) {
		alice_show_mouse();
	} else if (alice_mouse_button_just_pressed(ALICE_MOUSE_BUTTON_LEFT)) {
		alice_hide_mouse();
	}

	alice_rigidbody_3d_t* rigidbody = alice_get_entity_ptr(scene, handle);

	if (alice_key_pressed(ALICE_KEY_SPACE)) {
		rigidbody->force.y = fps->jump_height;
	} else {
		rigidbody->force.y = 0.0f;
	}

	if (fps->first_move) {
		fps->first_move = false;
		fps->old_mouse = alice_get_mouse_position();
	}

	alice_v2i_t mouse_pos = alice_get_mouse_position();

	i32 change_x = mouse_pos.x - fps->old_mouse.x;
	i32 change_y = fps->old_mouse.y - mouse_pos.y;

	fps->old_mouse = mouse_pos;

	fps->camera->base.rotation.y -= (float)change_x * 0.1f;
	fps->camera->base.rotation.x += (float)change_y * 0.1f;

	if (fps->camera->base.rotation.x >= 89.0f) {
		fps->camera->base.rotation.x = 89.0f;
	}

	if (fps->camera->base.rotation.x <= -89.0f) {
		fps->camera->base.rotation.x = -89.0f;
	}

	alice_v3f_t rotation = alice_torad_v3f(alice_get_entity_world_rotation(scene, (alice_entity_t*)fps->camera));

	alice_v3f_t direction = (alice_v3f_t) {
		.x = cosf(rotation.x) * sinf(rotation.y),
		.y = sinf(rotation.x),
		.z = cosf(rotation.x) * cosf(rotation.y)
	};

	alice_v3f_t velocity = (alice_v3f_t){
		0.0f, 0.0f, 0.0f
	};

	if (alice_key_pressed(ALICE_KEY_W)) {
		velocity.x += direction.x * fps->speed;
		velocity.z += direction.z * fps->speed;
	} else if (alice_key_pressed(ALICE_KEY_S)) {
		velocity.x += -direction.x * fps->speed;
		velocity.z += -direction.z * fps->speed;
	}

	alice_v3f_t right = alice_v3f_cross(direction, (alice_v3f_t){0.0f, 1.0f, 0.0f});

	if (alice_key_pressed(ALICE_KEY_A)) {
		velocity.x += -right.x * fps->speed;
		velocity.z += -right.z * fps->speed;
	} else if (alice_key_pressed(ALICE_KEY_D)) {
		velocity.x += right.x * fps->speed;
		velocity.z += right.z * fps->speed;
	}

	rigidbody->force.x = velocity.x;
	rigidbody->force.z = velocity.z;
}

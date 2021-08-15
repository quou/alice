#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <microui.h>

#include <alice/application.h>
#include <alice/entity.h>
#include <alice/resource.h>
#include <alice/graphics.h>
#include <alice/sceneserialise.h>
#include <alice/ui.h>
#include <alice/input.h>
#include <alice/scripting.h>
#include <alice/physics.h>
#include <alice/debugrenderer.h>

typedef struct sandbox_t {
	alice_scene_t* scene;
} sandbox_t;

void main() {
	sandbox_t sandbox;

	alice_init_resource_manager("res");
	alice_init_application((alice_application_config_t){
				.name = "sandbox",
				.splash_image = "splash.png",
				.splash_shader = "shaders/splash.glsl",
				.width = 1024,
				.height = 728,
				.fullscreen = false,
			});

	alice_init_default_resources();

#ifdef ALICE_PLATFORM_WINDOWS
	const char* script_lib_name = "scripts.dll";
#else
	const char* script_lib_name = "./libscripts.so";
#endif

	alice_scene_t* scene = alice_new_scene(script_lib_name);

	sandbox.scene = scene;

	alice_deserialise_scene(scene, "scenes/physicstest.ascn");
	alice_serialise_scene(scene, "scenes/physicstest.ascn");

	if (scene->renderer) {
		scene->renderer->ambient_intensity = 0.2f;
		scene->renderer->ambient_color = 0xadb0ea;
	}

	/*	{
		alice_entity_handle_t monkey_handle = alice_new_entity(scene, alice_rigidbody_3d_t);
		alice_rigidbody_3d_t* monkey = alice_get_entity_ptr(scene, monkey_handle);
		monkey->base.position.y = 10.0f;
		monkey->base.position.z = 3.0f;

		monkey->mass = 1.0f;
		monkey->restitution = 0.3f;

		monkey->velocity = (alice_v3f_t){0.0f, 0.0f, 0.0f};
		monkey->force = (alice_v3f_t){0.0f, 0.0f, 0.0f};

		monkey->gravity_scale = 1.0f;

		monkey->box = (alice_box_collider_t) {
			.dimentions = (alice_v3f_t) {
				.x = 2.0f,
				.y = 2.0f,
				.z = 2.0f
			}
		};

		alice_entity_handle_t monkey_visible_handle = alice_new_entity(scene, alice_renderable_3d_t);
		alice_renderable_3d_t* monkey_visible = alice_get_entity_ptr(scene, monkey_visible_handle);

		monkey_visible->base.rotation.y = 180.0f;

		monkey_visible->model = alice_load_model("models/monkey.glb");
		alice_renderable_3d_add_material(monkey_visible, "default_material");

		alice_entity_parent_to(scene, monkey_visible_handle, monkey_handle);
	}

	{
		alice_entity_handle_t monkey_handle = alice_new_entity(scene, alice_rigidbody_3d_t);
		alice_rigidbody_3d_t* monkey = alice_get_entity_ptr(scene, monkey_handle);
		monkey->base.position.y = 5.0f;
		monkey->base.position.z = 3.0f;

		monkey->mass = 1.0f;
		monkey->restitution = 0.3f;

		monkey->velocity = (alice_v3f_t){0.0f, 0.0f, 0.0f};
		monkey->force = (alice_v3f_t){0.0f, 0.0f, 0.0f};

		monkey->gravity_scale = 1.0f;

		monkey->box = (alice_box_collider_t) {
			.dimentions = (alice_v3f_t) {
				.x = 2.0f,
				.y = 2.0f,
				.z = 2.0f
			}
		};

		alice_entity_handle_t monkey_visible_handle = alice_new_entity(scene, alice_renderable_3d_t);
		alice_renderable_3d_t* monkey_visible = alice_get_entity_ptr(scene, monkey_visible_handle);

		monkey_visible->base.rotation.y = 180.0f;

		monkey_visible->model = alice_load_model("models/monkey.glb");
		alice_renderable_3d_add_material(monkey_visible, "default_material");

		alice_entity_parent_to(scene, monkey_visible_handle, monkey_handle);
	}

	{
		alice_entity_handle_t ground_handle = alice_new_entity(scene, alice_rigidbody_3d_t);
		alice_rigidbody_3d_t* ground = alice_get_entity_ptr(scene, ground_handle);
		ground->base.position.z = 3.0f;
		ground->base.position.y = -1.0f;

		ground->mass = 0.0f;
		ground->restitution = 0.3f;

		ground->velocity = (alice_v3f_t){0.0, 0.0f, 0.0f};
		ground->force = (alice_v3f_t){0.0f, 0.0f, 0.0f};

		ground->gravity_scale = 0.0f;

		ground->box = (alice_box_collider_t) {
			.dimentions = (alice_v3f_t) {
				.x = 10.0f,
				.y = 0.4f,
				.z = 10.0f
			}
		};

		alice_entity_handle_t ground_visible_handle = alice_new_entity(scene, alice_renderable_3d_t);
		alice_renderable_3d_t* ground_visible = alice_get_entity_ptr(scene, ground_visible_handle);

		ground_visible->base.scale = (alice_v3f_t) {
			.x = 10.0f,
			.y = 0.4f,
			.z = 10.0f
		};

		ground_visible->model = alice_load_model("cube");
		alice_renderable_3d_add_material(ground_visible, "default_material");

		alice_entity_parent_to(scene, ground_visible_handle, ground_handle);
	}

	{
		alice_entity_handle_t sun_handle = alice_new_entity(scene, alice_directional_light_t);
		alice_directional_light_t* sun = alice_get_entity_ptr(scene, sun_handle);

		sun->base.position = (alice_v3f_t){ 1.0f, -1.0f, 0.0f };

		sun->intensity = 10.0f;
		sun->color = ALICE_COLOR_WHITE;
	}

	{
		alice_entity_handle_t camera_handle = alice_new_entity(scene, alice_camera_3d_t);
		alice_camera_3d_t* camera = alice_get_entity_ptr(scene, camera_handle);

		camera->base.position.y = 3.0f;
		camera->base.rotation.x = -45.0f;

		camera->fov = 45.0f;
		camera->near = 0.1f;
		camera->far = 1000.0f;
		camera->exposure = 1.0f;
		camera->gamma = 1.4f;
		camera->active = true;
	}

	alice_serialise_scene(scene, "scenes/physicstest.ascn");
*/


	mu_Context* ui = malloc(sizeof(mu_Context));
	mu_init(ui);

	ui->style->colors[MU_COLOR_WINDOWBG] = mu_color(50, 50, 50, 200);

	ui->text_width = alice_microui_text_width;
	ui->text_height = alice_microui_text_height;
	alice_init_microui_renderer(alice_load_shader("shaders/ui.glsl"),
			alice_load_font("fonts/opensans.ttf", 18.0f));

	alice_init_scripts(scene->script_context);

	bool fullscreen = false;

	while (alice_is_application_running()) {
		alice_reload_changed_resources();

		alice_update_events();

		alice_render_clear();

		alice_application_t* app = alice_get_application();

		if (alice_key_just_pressed(ALICE_KEY_F11)) {
			fullscreen = !fullscreen;
			alice_set_application_fullscreen(0, fullscreen);
		}

		alice_update_scripts(scene->script_context, app->timestep);

		if (scene->physics_engine) {
			alice_update_physics_engine(scene->physics_engine, app->timestep);
		}

		if (scene->renderer) {
			alice_render_scene_3d(scene->renderer, app->width, app->height, scene, alice_null);
		}

		if (scene->renderer_2d) {
			alice_render_scene_2d(scene->renderer_2d, app->width, app->height, scene, alice_null);
		}

		alice_update_microui(ui);

		mu_begin(ui);
		if (mu_begin_window(ui, "Scene Settings", mu_rect(10, 10, 250, 300))) {
			if (scene->renderer) {
				mu_layout_row(ui, 1, (int[]) { -1 }, 0);

				mu_checkbox(ui, "Anti-aliasing", (i32*)&scene->renderer->use_antialiasing);
				mu_checkbox(ui, "Bloom", (i32*)&scene->renderer->use_bloom);

				mu_layout_row(ui, 2, (int[]) { -100, -1 }, 0);
				mu_label(ui, "Bloom threshold");
				mu_slider_ex(ui, &scene->renderer->bloom_threshold, 0.0f, 16.0f, 0.01f, "%g", 0);

				static float blur_iterations = 10.0f;
				mu_label(ui, "Bloom blur iterations");
				if (mu_slider_ex(ui, &blur_iterations,
							2.0f, 100.0f, 2.0f, "%g", 0) == MU_RES_CHANGE) {
					scene->renderer->bloom_blur_iterations = blur_iterations;
				}

				static char buf[256];
				mu_label(ui, "Test");
				mu_textbox(ui, buf, 256);
			}

			mu_end_window(ui);
		}
		mu_end(ui);

		alice_render_microui(ui, app->width, app->height);

		alice_update_application();
	}

	free(ui);
	alice_deinit_microui_renderer();

	alice_free_scene(scene);

	alice_free_application();
	alice_free_resource_manager();
}

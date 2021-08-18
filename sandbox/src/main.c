#include <assert.h>
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
	alice_entity_handle_t selected_entity;
	alice_entity_handle_t old_selected;
} sandbox_t;

sandbox_t sandbox;

static void draw_entity_hierarchy(mu_Context* ui, alice_scene_t* scene, alice_entity_handle_t entity) {
	assert(ui);
	assert(scene);

	alice_entity_t* ptr = alice_get_entity_ptr(scene, entity);

	const char* name = "entity";
	if (ptr->name) { name = ptr->name; }

	int opts = 0;
	if (ptr->child_count == 0) {
		opts |= MU_OPT_LEAF;
	}

	if (sandbox.selected_entity == entity) {
		opts |= MU_OPT_SELECTED;
	}

	int r = mu_begin_treenode_ex(ui, name, opts);

	if (mu_item_hovered(ui) && alice_mouse_button_just_released(ALICE_MOUSE_BUTTON_LEFT)) {
		sandbox.selected_entity = entity;
	}

	if (r) {
		for (u32 i = 0; i < ptr->child_count; i++) {
			draw_entity_hierarchy(ui, scene, ptr->children[i]);
		}

		mu_end_treenode(ui);
	}
}

static void draw_scene_hierarchy(mu_Context* ui, alice_scene_t* scene) {
	assert(ui);
	assert(scene);

	for (u32 i = 0; i < scene->pool_count; i++) {
		alice_entity_pool_t* pool = &scene->pools[i];
		for (u32 j = 0; j < pool->count; j++) {
			alice_entity_t* e = alice_entity_pool_get(pool, j);
			if (e->parent == alice_null_entity_handle) {
				draw_entity_hierarchy(ui, scene, alice_new_entity_handle(j, pool->type_id));
			}
		}
	}
}

static void draw_renderable_properties(mu_Context* ui, alice_scene_t* scene, alice_renderable_3d_t* renderable) {
	mu_checkbox(ui, "Cast shadows", &renderable->cast_shadows);

	if (renderable->model != alice_null) {
		for (u32 i = 0; i < renderable->model->mesh_count; i++) {
			char name[32] = "";
			sprintf(name, "Mesh %d", i);
			if (mu_begin_treenode(ui, name)) {
				alice_mesh_t* mesh = &renderable->model->meshes[i];

				alice_material_t* material = alice_null;
				alice_material_t** material_ptr = alice_null;
				if (i < renderable->material_count) {
					material = renderable->materials[i];
					material_ptr = &renderable->materials[i];
				} else if (renderable->material_count == 1) {
					material = renderable->materials[0];
					material_ptr = &renderable->materials[0];
				}

				static char material_buf[256];
				static alice_material_t* old_material = alice_null;

				if (material && old_material != material) {
					const char* material_path = alice_get_material_resource_filename(material);
					strncpy(material_buf, material_path, 256);

					old_material = material;
				}

				mu_layout_row(ui, 2, (int[]) {-200, -1}, 0);

				mu_label(ui, "Material: ");
				int r = mu_textbox(ui, material_buf, sizeof(material_buf));

				if (r == MU_RES_SUBMIT) {
					*material_ptr = alice_load_material(material_buf);
				}

				mu_end_treenode(ui);
			}
		}
	}
}

void main() {
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

	char scene_filename_buffer[256] = "scenes/physicstest.ascn";
	
	alice_scene_t* scene = alice_new_scene(script_lib_name);

	alice_deserialise_scene(scene, scene_filename_buffer);
	alice_serialise_scene(scene, scene_filename_buffer);

	alice_init_scripts(scene->script_context);

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

	alice_3d_pick_context_t* pick_context = alice_new_3d_pick_context(alice_load_shader("shaders/pick.glsl"));
	sandbox.selected_entity = alice_null_entity_handle;
	sandbox.old_selected = alice_null_entity_handle;

	mu_Context* ui = malloc(sizeof(mu_Context));
	mu_init(ui);

	ui->style->colors[MU_COLOR_WINDOWBG] = mu_color(50, 50, 50, 200);

	ui->text_width = alice_microui_text_width;
	ui->text_height = alice_microui_text_height;
	alice_init_microui_renderer(alice_load_shader("shaders/ui.glsl"),
			alice_load_font("fonts/opensans.ttf", 18.0f));

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

		alice_compute_scene_transforms(scene);

		if (scene->renderer) {
			alice_render_scene_3d(scene->renderer, app->width, app->height, scene, alice_null);
		}

		if (alice_mouse_button_just_released(ALICE_MOUSE_BUTTON_RIGHT)) {
			sandbox.selected_entity = alice_3d_pick(pick_context, scene);
		}

		if (scene->renderer_2d) {
			alice_render_scene_2d(scene->renderer_2d, app->width, app->height, scene, alice_null);
		}

		alice_update_microui(ui);

		mu_begin(ui);
		if (mu_begin_window(ui, "Entity", mu_rect(10, 420, 400, 300))) {
			static char entity_name_buf[256] = "";

			if (sandbox.selected_entity != alice_null_entity_handle) {
				alice_entity_t* ptr = alice_get_entity_ptr(scene, sandbox.selected_entity);

				if (sandbox.old_selected != sandbox.selected_entity) {
					sandbox.old_selected = sandbox.selected_entity;
					if (ptr->name) {
						strcpy(entity_name_buf, ptr->name);
					} else {
						strcpy(entity_name_buf, "unnamed entity");
					}
				}

				mu_layout_row(ui, 2, (int[]) { -200, -1 }, 0);

				mu_label(ui, "Name");
				if (mu_textbox(ui, entity_name_buf, sizeof(entity_name_buf)) == MU_RES_SUBMIT) {
					free(ptr->name);

					ptr->name = alice_copy_string(entity_name_buf);
				}

				mu_layout_row(ui, 4, (int[]) { -200, -132, -66, -1 }, 0);
				mu_label(ui, "Position: ");
				mu_number(ui, &ptr->position.x, 0.01f);
				mu_number(ui, &ptr->position.y, 0.01f);
				mu_number(ui, &ptr->position.z, 0.01f);

				mu_label(ui, "Rotation: ");
				mu_number(ui, &ptr->rotation.x, 0.5f);
				mu_number(ui, &ptr->rotation.y, 0.5f);
				mu_number(ui, &ptr->rotation.z, 0.5f);

				mu_label(ui, "Scale: ");
				mu_number(ui, &ptr->scale.x, 0.01f);
				mu_number(ui, &ptr->scale.y, 0.01f);
				mu_number(ui, &ptr->scale.z, 0.01f);

				mu_layout_row(ui, 1, (int[]) { -1 }, 0);

				u32 selected_entity_type_id = alice_get_entity_handle_type(sandbox.selected_entity);
				if (selected_entity_type_id == alice_get_type_info(alice_renderable_3d_t).id) {
					mu_label(ui, "Type: Renderable 3D");

					alice_renderable_3d_t* renderable = (alice_renderable_3d_t*)ptr;

					draw_renderable_properties(ui, scene, renderable);
				} else if (selected_entity_type_id == alice_get_type_info(alice_rigidbody_3d_t).id) {
					mu_label(ui, "Type: Rigidbody 3D");
				} else if (selected_entity_type_id == alice_get_type_info(alice_camera_3d_t).id) {
					mu_label(ui, "Type: Camera 3D");
				} else if (selected_entity_type_id == alice_get_type_info(alice_camera_2d_t).id) {
					mu_label(ui, "Type: Camera 2D");
				} else if (selected_entity_type_id == alice_get_type_info(alice_point_light_t).id) {
					mu_label(ui, "Type: Point Light");

					alice_point_light_t* light = (alice_point_light_t*)ptr;
		
					mu_layout_row(ui, 2, (int[]) { -200, -1 }, 0);

					mu_label(ui, "Intensity");
					mu_number(ui, &light->intensity, 0.1f);
					mu_label(ui, "Range");
					mu_number(ui, &light->range, 0.1f);
				} else if (selected_entity_type_id == alice_get_type_info(alice_directional_light_t).id) {
					mu_label(ui, "Type: Directional Light");

					alice_directional_light_t* light = (alice_directional_light_t*)ptr;

					mu_checkbox(ui, "Cast shadows", &light->cast_shadows);
				}
			}

			mu_end_window(ui);
		}

		if (mu_begin_window(ui, "Scene", mu_rect(10, 10, 400, 400))) {
			if (scene->renderer) {
				mu_layout_row(ui, 1, (int[]) { -1 }, 0);

				static char fps_buf[256] = "1000";
				static char frame_time_buf[256] = "0.001";
				static double time_until_next_fps_print = 1.0;

				time_until_next_fps_print -= app->timestep;
				if (time_until_next_fps_print <= 0.0) {
					time_until_next_fps_print = 1.0;
					sprintf(fps_buf, "FPS: %g", 1.0f / app->timestep);
					sprintf(frame_time_buf, "Frame Time %g", app->timestep);
				}

				mu_label(ui, fps_buf);
				mu_label(ui, frame_time_buf);

				mu_checkbox(ui, "Anti-aliasing", (i32*)&scene->renderer->use_antialiasing);
				mu_checkbox(ui, "Bloom", (i32*)&scene->renderer->use_bloom);

				mu_layout_row(ui, 2, (int[]) { -200, -1 }, 0);
				mu_label(ui, "Bloom threshold");
				mu_slider_ex(ui, &scene->renderer->bloom_threshold, 0.0f, 16.0f, 0.01f, "%g", 0);

				static float blur_iterations = 10.0f;
				mu_label(ui, "Bloom blur iterations");
				if (mu_slider_ex(ui, &blur_iterations,
							2.0f, 100.0f, 2.0f, "%g", 0) == MU_RES_CHANGE) {
					scene->renderer->bloom_blur_iterations = blur_iterations;
				}

				mu_label(ui, "Scene filename");
				mu_textbox(ui, scene_filename_buffer, 256);

				if (mu_button(ui, "Save")) {
					alice_serialise_scene(scene, scene_filename_buffer);
				}

				if (mu_button(ui, "Load")) {
					alice_free_scene(scene);
					scene = alice_new_scene(script_lib_name);
					alice_deserialise_scene(scene, scene_filename_buffer);

					sandbox.selected_entity = alice_null_entity_handle;

					alice_init_scripts(scene->script_context);
				}

				mu_layout_row(ui, 1, (int[]) { -1 }, 0);
				draw_scene_hierarchy(ui, scene);
			}

			mu_end_window(ui);
		}
		mu_end(ui);

		alice_render_microui(ui, app->width, app->height);

		alice_update_application();
	}

	alice_free_3d_pick_context(pick_context);

	free(ui);
	alice_deinit_microui_renderer();

	alice_free_scene(scene);

	alice_free_application();
	alice_free_resource_manager();
}

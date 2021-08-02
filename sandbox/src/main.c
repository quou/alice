#include <string.h>
#include <stdio.h>

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

typedef struct Sandbox {
	bool show_gui;
	alice_UIToggle* show_gui_toggle;
	alice_Scene* scene;
} Sandbox;

static void on_button_hover(alice_UIContext* context, alice_UIElement* button) {
	alice_log("hover");
}

static void on_button_click(alice_UIContext* context, alice_UIElement* button) {
	alice_log("click");
}

static void on_use_bloom_toggle(alice_UIContext* context, alice_UIElement* element) {
	alice_UIToggle* toggle = (alice_UIToggle*)element;

	Sandbox* sandbox = context->user_pointer;

	sandbox->scene->renderer->use_bloom = toggle->value;
}

static void on_use_antialiasing_toggle(alice_UIContext* context, alice_UIElement* element) {
	alice_UIToggle* toggle = (alice_UIToggle*)element;

	Sandbox* sandbox = context->user_pointer;

	sandbox->scene->renderer->use_antialiasing = toggle->value;
}

static void on_show_gui_toggle(alice_UIContext* context, alice_UIElement* element) {
	alice_UIToggle* toggle = (alice_UIToggle*)element;

	Sandbox* sandbox = context->user_pointer;

	sandbox->show_gui = toggle->value;
}

static void on_test_window_create(alice_UIContext* context, alice_UIWindow* window) {
	window->title = "Scene Settings";
	window->position = (alice_v2f){ 100.0f, 30.0f };
	window->dimentions = (alice_v2f) { 350.0f, 600.0f };

	Sandbox* sandbox = context->user_pointer;

	alice_Scene* scene = sandbox->scene;

	alice_UIToggle* show_gui_toggle = alice_add_ui_toggle(window);
	show_gui_toggle->base.on_click = on_show_gui_toggle;
	show_gui_toggle->label = "Show GUI";
	show_gui_toggle->value = sandbox->show_gui;

	sandbox->show_gui_toggle = show_gui_toggle;

	alice_UIToggle* use_bloom_toggle = alice_add_ui_toggle(window);
	use_bloom_toggle->base.on_click = on_use_bloom_toggle;
	use_bloom_toggle->label = "Bloom";
	use_bloom_toggle->value = scene->renderer->use_bloom;

	alice_UIToggle* use_antialiasing_toggle = alice_add_ui_toggle(window);
	use_antialiasing_toggle->base.on_click = on_use_antialiasing_toggle;
	use_antialiasing_toggle->label = "Antialiasing";
	use_antialiasing_toggle->value = scene->renderer->use_antialiasing;
}

void main() {
	Sandbox sandbox;

	alice_init_resource_manager("res");
	alice_init_application((alice_ApplicationConfig){
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

	alice_Scene* scene = alice_new_scene(script_lib_name);

	sandbox.scene = scene;

	alice_deserialise_scene(scene, "scenes/physicstest.ascn");
	alice_serialise_scene(scene, "scenes/physicstest.ascn");

	scene->renderer->ambient_intensity = 0.2f;
	scene->renderer->ambient_color = 0xadb0ea;

/*	{
		alice_EntityHandle monkey_handle = alice_new_entity(scene, alice_Rigidbody3D);
		alice_Rigidbody3D* monkey = alice_get_entity_ptr(scene, monkey_handle);
		monkey->base.position.y = 10.0f;
		monkey->base.position.z = 3.0f;

		monkey->mass = 1.0f;
		monkey->restitution = 0.3f;

		monkey->velocity = (alice_v3f){0.0f, 0.0f, 0.0f};
		monkey->force = (alice_v3f){0.0f, 0.0f, 0.0f};

		monkey->gravity_scale = 1.0f;

		monkey->box = (alice_BoxCollider) {
			.dimentions = (alice_v3f) {
				.x = 2.0f,
				.y = 2.0f,
				.z = 2.0f
			}
		};

		alice_EntityHandle monkey_visible_handle = alice_new_entity(scene, alice_Renderable3D);
		alice_Renderable3D* monkey_visible = alice_get_entity_ptr(scene, monkey_visible_handle);

		monkey_visible->base.rotation.y = 180.0f;

		monkey_visible->model = alice_load_model("models/monkey.glb");
		alice_renderable_3d_add_material(monkey_visible, "default_material");

		alice_entity_parent_to(scene, monkey_visible_handle, monkey_handle);
	}

	{
		alice_EntityHandle monkey_handle = alice_new_entity(scene, alice_Rigidbody3D);
		alice_Rigidbody3D* monkey = alice_get_entity_ptr(scene, monkey_handle);
		monkey->base.position.y = 5.0f;
		monkey->base.position.z = 3.0f;

		monkey->mass = 1.0f;
		monkey->restitution = 0.3f;

		monkey->velocity = (alice_v3f){0.0f, 0.0f, 0.0f};
		monkey->force = (alice_v3f){0.0f, 0.0f, 0.0f};

		monkey->gravity_scale = 1.0f;

		monkey->box = (alice_BoxCollider) {
			.dimentions = (alice_v3f) {
				.x = 2.0f,
				.y = 2.0f,
				.z = 2.0f
			}
		};

		alice_EntityHandle monkey_visible_handle = alice_new_entity(scene, alice_Renderable3D);
		alice_Renderable3D* monkey_visible = alice_get_entity_ptr(scene, monkey_visible_handle);

		monkey_visible->base.rotation.y = 180.0f;

		monkey_visible->model = alice_load_model("models/monkey.glb");
		alice_renderable_3d_add_material(monkey_visible, "default_material");

		alice_entity_parent_to(scene, monkey_visible_handle, monkey_handle);
	}

	{
		alice_EntityHandle ground_handle = alice_new_entity(scene, alice_Rigidbody3D);
		alice_Rigidbody3D* ground = alice_get_entity_ptr(scene, ground_handle);
		ground->base.position.z = 3.0f;
		ground->base.position.y = -1.0f;

		ground->mass = 0.0f;
		ground->restitution = 0.3f;

		ground->velocity = (alice_v3f){0.0, 0.0f, 0.0f};
		ground->force = (alice_v3f){0.0f, 0.0f, 0.0f};

		ground->gravity_scale = 0.0f;

		ground->box = (alice_BoxCollider) {
			.dimentions = (alice_v3f) {
				.x = 10.0f,
				.y = 0.4f,
				.z = 10.0f
			}
		};

		alice_EntityHandle ground_visible_handle = alice_new_entity(scene, alice_Renderable3D);
		alice_Renderable3D* ground_visible = alice_get_entity_ptr(scene, ground_visible_handle);

		ground_visible->base.scale = (alice_v3f) {
			.x = 10.0f,
			.y = 0.4f,
			.z = 10.0f
		};

		ground_visible->model = alice_load_model("cube");
		alice_renderable_3d_add_material(ground_visible, "default_material");

		alice_entity_parent_to(scene, ground_visible_handle, ground_handle);
	}

	{
		alice_EntityHandle sun_handle = alice_new_entity(scene, alice_DirectionalLight);
		alice_DirectionalLight* sun = alice_get_entity_ptr(scene, sun_handle);

		sun->base.position = (alice_v3f){ 1.0f, -1.0f, 0.0f };

		sun->intensity = 10.0f;
		sun->color = ALICE_COLOR_WHITE;
	}

	{
		alice_EntityHandle camera_handle = alice_new_entity(scene, alice_Camera3D);
		alice_Camera3D* camera = alice_get_entity_ptr(scene, camera_handle);

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

	alice_init_scripts(scene->script_context);

	alice_UIContext* ui = alice_new_ui_context(
			alice_load_shader("shaders/uirect.glsl"),
			alice_load_shader("shaders/gizmo.glsl"),
			alice_load_shader("shaders/text.glsl"),
			alice_load_binary("fonts/opensans.ttf"),
			18.0f);

	ui->user_pointer = &sandbox;

	ui->gizmo_textures[ALICE_GIZMOTEXTURE_POINT_LIGHT] =
		alice_load_texture("textures/icons/light.png", ALICE_TEXTURE_ALIASED);
	ui->gizmo_textures[ALICE_GIZMOTEXTURE_DIRECTIONAL_LIGHT] =
		alice_load_texture("textures/icons/sun.png", ALICE_TEXTURE_ALIASED);

	alice_new_ui_window(ui, on_test_window_create);

	alice_TextRenderer* text_renderer = alice_new_text_renderer(alice_load_binary("fonts/opensans.ttf"),
			32.0f, alice_load_shader("shaders/text.glsl"));

	char fps_buffer[256] = "";
	double time_until_fps_write = 1.0;

	bool fullscreen = false;
	sandbox.show_gui = false;

	while (alice_is_application_running()) {
		alice_reload_changed_resources();

		alice_update_events();

		alice_render_clear();

		alice_Application* app = alice_get_application();

		time_until_fps_write -= app->timestep;
		if (time_until_fps_write <= 0.0) {
			time_until_fps_write = 1.0;
			sprintf(fps_buffer, "fps: %g", 1.0 / app->timestep);
		}

		if (alice_key_just_pressed(ALICE_KEY_F11)) {
			fullscreen = !fullscreen;
			alice_set_application_fullscreen(0, fullscreen);
		}

		if (alice_key_just_pressed(ALICE_KEY_F1)) {
			sandbox.show_gui = !sandbox.show_gui;
			sandbox.show_gui_toggle->value = sandbox.show_gui;
		}

		alice_update_scripts(scene->script_context, app->timestep);

		alice_update_physics_engine(scene->physics_engine, app->timestep);

		alice_render_scene_3d(scene->renderer, app->width, app->height, scene, alice_null);

		if (sandbox.show_gui) {
			alice_set_text_renderer_dimentions(text_renderer, (alice_v2f){app->width, app->height});
			alice_render_text(text_renderer, (alice_v2f){0, 0}, fps_buffer);

			alice_draw_scene_gizmos(ui, scene);
			alice_draw_ui(ui);
		}

		alice_update_application();
	}

	alice_free_ui_context(ui);
	alice_free_text_renderer(text_renderer);

	alice_free_scene(scene);

	alice_free_application();
	alice_free_resource_manager();
}

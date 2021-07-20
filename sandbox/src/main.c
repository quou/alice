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

static void on_button_hover(alice_UIContext* context, alice_UIElement* button) {
	alice_log("hover");
}

static void on_button_click(alice_UIContext* context, alice_UIElement* button) {
	alice_log("click");
}

static void on_test_window_create(alice_UIContext* context, alice_UIWindow* window) {
	window->title = "I'm a window!";
	window->position = (alice_v2f){ 100.0f, 30.0f };
	window->dimentions = (alice_v2f) { 350.0f, 600.0f };

	alice_UILabel* label = alice_add_ui_label(window);
	label->text = "I'm a label!";

	alice_UIButton* button2 = alice_add_ui_button(window);
	button2->text = "I'm a button!";
	button2->base.on_hover = on_button_hover;
	button2->base.on_click = on_button_click;

	alice_UITextInput* input = alice_add_ui_text_input(window);
	input->label = "Text input 1";
	input->buffer = "Some text";
}

void main() {
	alice_init_resource_manager("res");
	alice_init_application((alice_ApplicationConfig){
				.name = "sandbox",
				.splash_image = "splash.png",
				.splash_shader = "shaders/splash.glsl",
				.width = 1024,
				.height = 728,
				.fullscreen = false
			});

	alice_init_default_resources();

	alice_Scene* scene = alice_new_scene("scripts.dll");
	alice_deserialise_scene(scene, "scenes/lottamonkeys.ascn");

	alice_init_scripts(scene->script_context);

	alice_SceneRenderer3D* renderer = alice_new_scene_renderer_3d(
			alice_load_shader("shaders/postprocess.glsl"),
			alice_load_shader("shaders/bright_extract.glsl"),
			alice_load_shader("shaders/blur.glsl"));

	renderer->use_bloom = true;
	renderer->use_antialiasing = true;

	alice_UIContext* ui = alice_new_ui_context(
			alice_load_shader("shaders/uirect.glsl"),
			alice_load_shader("shaders/text.glsl"),
			alice_load_binary("fonts/opensans.ttf"),
			18.0f);

	//alice_new_ui_window(ui, on_test_window_create);

	alice_TextRenderer* text_renderer = alice_new_text_renderer(alice_load_binary("fonts/opensans.ttf"),
			32.0f, alice_load_shader("shaders/text.glsl"));

	char fps_buffer[256] = "";
	double time_until_fps_write = 1.0;

	bool fullscreen = false;

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

		alice_update_scripts(scene->script_context, app->timestep);

		alice_render_scene_3d(renderer, app->width, app->height, scene, alice_null);

		alice_set_text_renderer_dimentions(text_renderer, (alice_v2f){app->width, app->height});
		alice_render_text(text_renderer, (alice_v2f){0, 0}, fps_buffer);

		alice_draw_ui(ui);

		alice_update_application();
	}

	alice_free_ui_context(ui);
	alice_free_text_renderer(text_renderer);

	alice_free_scene_renderer_3d(renderer);

	alice_free_scene(scene);

	alice_free_application();
	alice_free_resource_manager();
}

#include <string.h>
#include <stdio.h>

#include <alice/application.h>
#include <alice/entity.h>
#include <alice/resource.h>
#include <alice/graphics.h>
#include <alice/sceneserialise.h>
#include <alice/ui.h>
#include <alice/input.h>

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

	alice_Scene* scene = alice_new_scene();
	alice_deserialise_scene(scene, "scenes/monkey.ascn");
	alice_serialise_scene(scene, "scenes/monkey.ascn");

	alice_SceneRenderer3D* renderer = alice_new_scene_renderer_3d(
			alice_load_shader("shaders/postprocess.glsl"),
			alice_load_shader("shaders/bright_extract.glsl"),
			alice_load_shader("shaders/blur.glsl"));

	renderer->use_bloom = true;
	renderer->use_antialiasing = true;

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

		alice_set_text_renderer_dimentions(text_renderer, (alice_v2f){app->width, app->height});
		alice_render_text(text_renderer, (alice_v2f){0, 0}, fps_buffer);

		alice_render_scene_3d(renderer, app->width, app->height, scene, alice_null);

		alice_update_application();
	}

	alice_free_text_renderer(text_renderer);

	alice_free_scene_renderer_3d(renderer);

	alice_free_scene(scene);

	alice_free_application();
	alice_free_resource_manager();
}

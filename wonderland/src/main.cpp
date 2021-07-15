#include <string.h>
#include <stdio.h>

extern "C" {
	#include <alice/application.h>
	#include <alice/entity.h>
	#include <alice/resource.h>
	#include <alice/graphics.h>
	#include <alice/sceneserialise.h>
	#include <alice/ui.h>
	#include <alice/input.h>
}

#include "gui.hpp"
#include "imgui/imgui.h"

int main() {
	alice_init_resource_manager("../sandbox/res");
	alice_init_application({
				.name = "wonderland",
				.splash_image = "splash.png",
				.splash_shader = "shaders/splash.glsl",
				.width = 1024,
				.height = 728,
				.fullscreen = false
			});

	init_gui(alice_get_application());

	alice_init_default_resources();

	alice_Scene* scene = alice_new_scene();	
	alice_deserialise_scene(scene, "scenes/cube.ascn");	
	
	alice_SceneRenderer3D* renderer = alice_new_scene_renderer_3d(
			alice_load_shader("shaders/postprocess.glsl"),
			alice_load_shader("shaders/bright_extract.glsl"),
			alice_load_shader("shaders/blur.glsl"));

	alice_RenderTarget* scene_target = alice_new_render_target(128, 128, 1);

	char fps_buffer[256] = "";
	double time_until_fps_write = 1.0;\

	while (alice_is_application_running()) {
		alice_reload_changed_resources();
		
		alice_update_events();

		alice_render_clear();

		gui_begin_frame();

		ImGui::Begin("scene");	
		float width = ImGui::GetContentRegionAvail().x;
		float height = ImGui::GetContentRegionAvail().y;

		alice_render_scene_3d(renderer, width, height, scene, scene_target);

		ImGui::Image((ImTextureID)(u64)scene_target->color_attachments[0],
			ImVec2(width, height), ImVec2(0.0, 1.0), ImVec2(1.0, 0.0));

		ImGui::End();

		gui_end_frame();

		alice_update_application();
	}

	alice_free_render_target(scene_target);

	alice_free_scene_renderer_3d(renderer);

	alice_free_scene(scene);

	quit_gui();

	alice_free_application();
	alice_free_resource_manager();
}


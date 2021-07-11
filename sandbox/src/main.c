#include <alice/application.h>
#include <alice/entity.h>
#include <alice/resource.h>
#include <alice/graphics.h>
#include <alice/sceneserialise.h>

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
	
	alice_EntityHandle cube = alice_new_entity(scene, alice_Renderable3D);
	alice_Renderable3D* cube_ptr = alice_get_entity_ptr(scene, cube);
		
	cube_ptr->base.position.z = 3.0f;
	cube_ptr->base.rotation = (alice_v3f) {
		.x = -32.5f,
		.y = 45.0f,
		.z = 388.5f
	};

	cube_ptr->mesh = alice_load_mesh("cube");
	cube_ptr->material = alice_load_material("materials/pirategold.mat");
	
	alice_EntityHandle sphere = alice_new_entity(scene, alice_Renderable3D);
	alice_Renderable3D* sphere_ptr = alice_get_entity_ptr(scene, sphere);

	sphere_ptr->base.position.x = 1.1f;

	sphere_ptr->mesh = alice_load_mesh("sphere");
	sphere_ptr->material = alice_load_material("materials/rustediron.mat");

	alice_entity_parent_to(scene, sphere, cube);

	{
		alice_EntityHandle e_handle = alice_new_entity(scene, alice_Camera3D);
		alice_Camera3D* e = alice_get_entity_ptr(scene, e_handle);	

		e->fov = 45.0f;
		e->near = 0.1f;
		e->far = 1000.0f;

		e->exposure = 1.0f;
		e->gamma = 1.4f;
	}

	{
		alice_EntityHandle e_handle = alice_new_entity(scene, alice_PointLight);
		alice_PointLight* e = alice_get_entity_ptr(scene, e_handle);	

		e->color = ALICE_COLOR_WHITE;
		e->range = 1.0f;
		e->intensity = 100.0f;
	}

	{
		alice_EntityHandle e_handle = alice_new_entity(scene, alice_PointLight);
		alice_PointLight* e = alice_get_entity_ptr(scene, e_handle);	

		e->base.position.z = 3.0f;
		e->base.position.y = 1.3f;

		e->color = ALICE_COLOR_RED;
		e->range = 1.0f;
		e->intensity = 100.0f;
	}


	{
		alice_EntityHandle e_handle = alice_new_entity(scene, alice_PointLight);
		alice_PointLight* e = alice_get_entity_ptr(scene, e_handle);	

		e->base.position.z = 3.0f;
		e->base.position.y = -2.0f;

		e->color = ALICE_COLOR_BLUE;
		e->range = 1.0f;
		e->intensity = 100.0f;
	}

	alice_SceneRenderer3D* renderer = alice_new_scene_renderer_3d(alice_load_shader("shaders/postprocess.glsl"));

	while (alice_is_application_running()) {
		alice_reload_changed_resources();
		
		alice_update_events();

		alice_render_clear();

		alice_Application* app = alice_get_application();

/*		double timestep = alice_get_timestep();

		cube_ptr->base.rotation.x += 100.0 * timestep;
		cube_ptr->base.rotation.y += 100.0 * timestep;
		cube_ptr->base.rotation.z += 100.0 * timestep;*/

		alice_render_scene_3d(renderer, app->width, app->height, scene, alice_null);

		alice_update_application();
	}

	alice_free_scene_renderer_3d(renderer);

	alice_free_scene(scene);

	alice_free_application();
	alice_free_resource_manager();
}

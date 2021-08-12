#include <assert.h>
#include <string.h>

#include "alice/sceneserialise.h"
#include "alice/dtable.h"
#include "alice/graphics.h"
#include "alice/scripting.h"
#include "alice/physics.h"
#include "alice/debugrenderer.h"

typedef enum alice_serialisable_type_t {
	ALICE_ST_ENTITY,
	ALICE_ST_RENDERABLE3D,
	ALICE_ST_POINTLIGHT,
	ALICE_ST_DIRECTIONALLIGHT,
	ALICE_ST_CAMERA3D,
	ALICE_ST_CAMERA2D,
	ALICE_ST_RIGIDBODY3D,
	ALICE_ST_SPRITE2D
} alice_serialisable_type_t;

static alice_serialisable_type_t alice_determine_entity_type(alice_entity_handle_t handle) {
	u32 type_id = alice_get_entity_handle_type(handle);

	if (type_id == alice_get_type_info(alice_renderable_3d_t).id) {
		return ALICE_ST_RENDERABLE3D;
	} else if (type_id == alice_get_type_info(alice_point_light_t).id) {
		return ALICE_ST_POINTLIGHT;
	} else if (type_id == alice_get_type_info(alice_camera_3d_t).id) {
		return ALICE_ST_CAMERA3D;
	} else if (type_id == alice_get_type_info(alice_camera_2d_t).id) {
		return ALICE_ST_CAMERA2D;
	} else if (type_id == alice_get_type_info(alice_directional_light_t).id) {
		return ALICE_ST_DIRECTIONALLIGHT;
	} else if (type_id == alice_get_type_info(alice_rigidbody_3d_t).id) {
		return ALICE_ST_RIGIDBODY3D;
	} else if (type_id == alice_get_type_info(alice_sprite_2d_t).id) {
		return ALICE_ST_SPRITE2D;
	}

	return ALICE_ST_ENTITY;
}

static void alice_serialise_entity(alice_dtable_t* table, alice_scene_t* scene, alice_entity_handle_t handle) {
	assert(table && scene);

	alice_entity_t* entity = alice_get_entity_ptr(scene, handle);

	alice_serialisable_type_t entity_type = alice_determine_entity_type(handle);

	const char* type_name = "entity";
	switch (entity_type) {
		case ALICE_ST_RENDERABLE3D:
			type_name = "renderable_3d";
			break;
		case ALICE_ST_POINTLIGHT:
			type_name = "point_light";
			break;
		case ALICE_ST_CAMERA3D:
			type_name = "camera_3d";
			break;
		case ALICE_ST_CAMERA2D:
			type_name = "camera_2d";
			break;
		case ALICE_ST_DIRECTIONALLIGHT:
			type_name = "directional_light";
			break;
		case ALICE_ST_RIGIDBODY3D:
			type_name = "rigidbody_3d";
			break;
		case ALICE_ST_SPRITE2D:
			type_name = "sprite_2d";
			break;
	}

	alice_dtable_t entity_table = alice_new_empty_dtable(type_name);

	if (entity->name) {
		alice_dtable_t name_table = alice_new_string_dtable("name", entity->name);
		alice_dtable_add_child(&entity_table, name_table);
	}

	alice_dtable_t position_table = alice_new_empty_dtable("position");
	{
		alice_dtable_t x_table = alice_new_number_dtable("x", entity->position.x);
		alice_dtable_t y_table = alice_new_number_dtable("y", entity->position.y);
		alice_dtable_t z_table = alice_new_number_dtable("z", entity->position.z);

		alice_dtable_add_child(&position_table, x_table);
		alice_dtable_add_child(&position_table, y_table);
		alice_dtable_add_child(&position_table, z_table);
	}
	alice_dtable_add_child(&entity_table, position_table);

	alice_dtable_t rotation_table = alice_new_empty_dtable("rotation");
	{
		alice_dtable_t x_table = alice_new_number_dtable("x", entity->rotation.x);
		alice_dtable_t y_table = alice_new_number_dtable("y", entity->rotation.y);
		alice_dtable_t z_table = alice_new_number_dtable("z", entity->rotation.z);

		alice_dtable_add_child(&rotation_table, x_table);
		alice_dtable_add_child(&rotation_table, y_table);
		alice_dtable_add_child(&rotation_table, z_table);
	}
	alice_dtable_add_child(&entity_table, rotation_table);

	alice_dtable_t scale_table = alice_new_empty_dtable("scale");
	{
		alice_dtable_t x_table = alice_new_number_dtable("x", entity->scale.x);
		alice_dtable_t y_table = alice_new_number_dtable("y", entity->scale.y);
		alice_dtable_t z_table = alice_new_number_dtable("z", entity->scale.z);

		alice_dtable_add_child(&scale_table, x_table);
		alice_dtable_add_child(&scale_table, y_table);
		alice_dtable_add_child(&scale_table, z_table);
	}
	alice_dtable_add_child(&entity_table, scale_table);

	if (entity->script) {
		alice_dtable_t script_table = alice_new_empty_dtable("script");

		if (entity->script->get_instance_size_name) {
			alice_dtable_t get_instance_size_table =
				alice_new_string_dtable("get_instance_size", entity->script->get_instance_size_name);
			alice_dtable_add_child(&script_table, get_instance_size_table);
		}

		if (entity->script->on_init_name) {
			alice_dtable_t on_init_table = alice_new_string_dtable("on_init", entity->script->on_init_name);
			alice_dtable_add_child(&script_table, on_init_table);
		}

		if (entity->script->on_update_name) {
			alice_dtable_t on_update_table = alice_new_string_dtable("on_update", entity->script->on_update_name);
			alice_dtable_add_child(&script_table, on_update_table);
		}

		if (entity->script->on_physics_update_name) {
			alice_dtable_t on_physics_update_table = alice_new_string_dtable("on_physics_update", entity->script->on_physics_update_name);
			alice_dtable_add_child(&script_table, on_physics_update_table);
		}

		if (entity->script->on_free_name) {
			alice_dtable_t on_free_table = alice_new_string_dtable("on_free", entity->script->on_free_name);
			alice_dtable_add_child(&script_table, on_free_table);
		}


		alice_dtable_add_child(&entity_table, script_table);
	}

	switch (entity_type) {
		case ALICE_ST_RENDERABLE3D: {
			alice_renderable_3d_t* renderable = (alice_renderable_3d_t*)entity;

			const char* model_path = alice_get_model_resource_filename(renderable->model);

			alice_dtable_t cast_shadows_table = alice_new_bool_dtable("cast_shadows",
					renderable->cast_shadows);
			alice_dtable_add_child(&entity_table, cast_shadows_table);

			alice_dtable_t model_table = alice_new_string_dtable("model", model_path);
			alice_dtable_add_child(&entity_table, model_table);

			alice_dtable_value_array_t* material_array = alice_new_dtable_value_array();

			for (u32 i = 0; i < renderable->material_count; i++) {
				alice_material_t* material = renderable->materials[i];

				const char* material_path =
					alice_get_material_resource_filename(material);
				if (strcmp(material_path, "default_material") != 0) {
					alice_save_material(material, material_path);
				}

				alice_dtable_value_t material_path_value;
				material_path_value.type = ALICE_DTABLE_STRING;
				material_path_value.as.string = alice_copy_string(material_path);

				alice_dtable_value_array_add(material_array, material_path_value);
			}

			alice_dtable_t materials_table = alice_new_array_dtable("materials", material_array);
			alice_dtable_add_child(&entity_table, materials_table);

			break;
		    }
		case ALICE_ST_POINTLIGHT: {
			alice_point_light_t* light = (alice_point_light_t*)entity;

			alice_rgb_color_t color = alice_rgb_color_from_color(light->color);

			alice_dtable_t color_table = alice_new_empty_dtable("color");
			alice_dtable_t r_table = alice_new_number_dtable("r", color.r);
			alice_dtable_t g_table = alice_new_number_dtable("g", color.g);
			alice_dtable_t b_table = alice_new_number_dtable("b", color.b);

			alice_dtable_add_child(&color_table, r_table);
			alice_dtable_add_child(&color_table, g_table);
			alice_dtable_add_child(&color_table, b_table);

			alice_dtable_add_child(&entity_table, color_table);

			alice_dtable_t range_table = alice_new_number_dtable("range", light->range);
			alice_dtable_add_child(&entity_table, range_table);

			alice_dtable_t intensity_table = alice_new_number_dtable("intensity", light->intensity);
			alice_dtable_add_child(&entity_table, intensity_table);

			break;
		}
		case ALICE_ST_DIRECTIONALLIGHT: {
			alice_directional_light_t* light = (alice_directional_light_t*)entity;

			alice_rgb_color_t color = alice_rgb_color_from_color(light->color);

			alice_dtable_t color_table = alice_new_empty_dtable("color");
			alice_dtable_t r_table = alice_new_number_dtable("r", color.r);
			alice_dtable_t g_table = alice_new_number_dtable("g", color.g);
			alice_dtable_t b_table = alice_new_number_dtable("b", color.b);

			alice_dtable_add_child(&color_table, r_table);
			alice_dtable_add_child(&color_table, g_table);
			alice_dtable_add_child(&color_table, b_table);

			alice_dtable_add_child(&entity_table, color_table);

			alice_dtable_t intensity_table = alice_new_number_dtable("intensity", light->intensity);
			alice_dtable_add_child(&entity_table, intensity_table);

			alice_dtable_t cast_shadows_table = alice_new_bool_dtable("cast_shadows",
					light->cast_shadows);
			alice_dtable_add_child(&entity_table, cast_shadows_table);

			break;
		}
		case ALICE_ST_CAMERA3D: {
			alice_camera_3d_t* camera = (alice_camera_3d_t*)entity;

			alice_dtable_t fov_table = alice_new_number_dtable("fov", camera->fov);
			alice_dtable_add_child(&entity_table, fov_table);

			alice_dtable_t near_table = alice_new_number_dtable("near", camera->near);
			alice_dtable_add_child(&entity_table, near_table);

			alice_dtable_t far_table = alice_new_number_dtable("far", camera->far);
			alice_dtable_add_child(&entity_table, far_table);

			alice_dtable_t exposure_table = alice_new_number_dtable("exposure", camera->exposure);
			alice_dtable_add_child(&entity_table, exposure_table);

			alice_dtable_t gamma_table = alice_new_number_dtable("gamma", camera->gamma);
			alice_dtable_add_child(&entity_table, gamma_table);

			alice_dtable_t active_table = alice_new_bool_dtable("active", camera->active);
			alice_dtable_add_child(&entity_table, active_table);
			break;
		}
		case ALICE_ST_CAMERA2D: {
			alice_camera_2d_t* camera = (alice_camera_2d_t*)entity;

			alice_dtable_t dimentions_table = alice_new_empty_dtable("dimentions");
			{
				alice_dtable_t x_table = alice_new_number_dtable("x", camera->dimentions.x);
				alice_dtable_add_child(&dimentions_table, x_table);

				alice_dtable_t y_table = alice_new_number_dtable("y", camera->dimentions.y);
				alice_dtable_add_child(&dimentions_table, y_table);

				alice_dtable_add_child(&entity_table, dimentions_table);
			}

			alice_dtable_t stretch_table = alice_new_bool_dtable("stretch", camera->stretch);
			alice_dtable_add_child(&entity_table, stretch_table);

			alice_dtable_t active_table = alice_new_bool_dtable("active", camera->active);
			alice_dtable_add_child(&entity_table, active_table);

			break;
		}
		case ALICE_ST_RIGIDBODY3D: {
			alice_rigidbody_3d_t* rigidbody = (alice_rigidbody_3d_t*)entity;

			alice_dtable_t mass_table = alice_new_number_dtable("mass", rigidbody->mass);
			alice_dtable_add_child(&entity_table, mass_table);

			alice_dtable_t static_friction_table = alice_new_number_dtable(
					"dynamic_friction", rigidbody->dynamic_friction);
			alice_dtable_add_child(&entity_table, static_friction_table);

			alice_dtable_t dynamic_friction_table = alice_new_number_dtable(
					"static_friction", rigidbody->static_friction);
			alice_dtable_add_child(&entity_table, dynamic_friction_table);

			alice_dtable_t restitution_table = alice_new_number_dtable("restitution",
					rigidbody->restitution);
			alice_dtable_add_child(&entity_table, restitution_table);

			alice_dtable_t gravity_scale_table = alice_new_number_dtable("gravity_scale",
					rigidbody->gravity_scale);
			alice_dtable_add_child(&entity_table, gravity_scale_table);

			alice_dtable_t box_table = alice_new_empty_dtable("box");

			{
				alice_dtable_t dimentions_table = alice_new_empty_dtable("dimentions");

				alice_dtable_t x_table = alice_new_number_dtable("x", rigidbody->box.dimentions.x);
				alice_dtable_add_child(&dimentions_table, x_table);

				alice_dtable_t y_table = alice_new_number_dtable("y", rigidbody->box.dimentions.y);
				alice_dtable_add_child(&dimentions_table, y_table);

				alice_dtable_t z_table = alice_new_number_dtable("z", rigidbody->box.dimentions.z);
				alice_dtable_add_child(&dimentions_table, z_table);

				alice_dtable_add_child(&box_table, dimentions_table);
			}

			{
				alice_dtable_t position_table = alice_new_empty_dtable("position");

				alice_dtable_t x_table = alice_new_number_dtable("x", rigidbody->box.position.x);
				alice_dtable_add_child(&position_table, x_table);

				alice_dtable_t y_table = alice_new_number_dtable("y", rigidbody->box.position.y);
				alice_dtable_add_child(&position_table, y_table);

				alice_dtable_t z_table = alice_new_number_dtable("z", rigidbody->box.position.z);
				alice_dtable_add_child(&position_table, z_table);

				alice_dtable_add_child(&box_table, position_table);
			}

			alice_dtable_add_child(&entity_table, box_table);

			{
				alice_dtable_t constraints_table = alice_new_empty_dtable("constraints");

				alice_dtable_t x_table = alice_new_bool_dtable("x", rigidbody->constraints.x);
				alice_dtable_add_child(&constraints_table, x_table);

				alice_dtable_t y_table = alice_new_bool_dtable("y", rigidbody->constraints.y);
				alice_dtable_add_child(&constraints_table, y_table);

				alice_dtable_t z_table = alice_new_bool_dtable("z", rigidbody->constraints.z);
				alice_dtable_add_child(&constraints_table, z_table);

				alice_dtable_add_child(&entity_table, constraints_table);
			}

			break;
		}
		case ALICE_ST_SPRITE2D: {
			alice_sprite_2d_t* sprite = (alice_sprite_2d_t*)entity;

			alice_dtable_t image_table = alice_new_empty_dtable("image");

			const char* image_path = alice_get_texture_resource_filename(sprite->image);

			alice_dtable_t path_table = alice_new_string_dtable("path", image_path);
			alice_dtable_add_child(&image_table, path_table);

			alice_dtable_t is_antialiased_table = alice_new_bool_dtable("is_antialiased",
					sprite->image->flags & ALICE_TEXTURE_ANTIALIASED);
			alice_dtable_add_child(&image_table, is_antialiased_table);

			alice_dtable_add_child(&entity_table, image_table);

			break;
		}
	}

	if (entity->child_count > 0) {
		alice_dtable_t children_table = alice_new_empty_dtable("children");

		for (u32 i = 0; i < entity->child_count; i++) {
			alice_serialise_entity(&children_table, scene, entity->children[i]);
		}

		alice_dtable_add_child(&entity_table, children_table);
	}

	alice_dtable_add_child(table, entity_table);
}

void alice_serialise_scene(alice_scene_t* scene, const char* file_path) {
	alice_dtable_t table = alice_new_empty_dtable("scene");

	alice_dtable_t settings_table = alice_new_empty_dtable("settings");
	if (scene->renderer) {
		alice_dtable_t renderer_table = alice_new_empty_dtable("renderer_3d");

		if (scene->renderer->postprocess) {
			alice_dtable_t shader_table = alice_new_string_dtable("postprocess_shader",
					alice_get_shader_resource_filename(scene->renderer->postprocess));
			alice_dtable_add_child(&renderer_table, shader_table);
		}

		if (scene->renderer->extract) {
			alice_dtable_t shader_table = alice_new_string_dtable("bright_extract_shader",
					alice_get_shader_resource_filename(scene->renderer->extract));
			alice_dtable_add_child(&renderer_table, shader_table);
		}

		if (scene->renderer->blur) {
			alice_dtable_t shader_table = alice_new_string_dtable("blur_shader",
					alice_get_shader_resource_filename(scene->renderer->blur));
			alice_dtable_add_child(&renderer_table, shader_table);
		}

		if (scene->renderer->shadowmap->shader) {
			alice_dtable_t shader_table = alice_new_string_dtable("depth_shader",
					alice_get_shader_resource_filename(scene->renderer->shadowmap->shader));
			alice_dtable_add_child(&renderer_table, shader_table);
		}

		alice_dtable_t debug_table = alice_new_bool_dtable("debug", scene->renderer->debug);
		alice_dtable_add_child(&renderer_table, debug_table);

		if (scene->renderer->debug) {
			alice_dtable_t shader_table = alice_new_string_dtable("line_shader",
					alice_get_shader_resource_filename(
						scene->renderer->debug_renderer->line_shader));
			alice_dtable_add_child(&renderer_table, shader_table);
		}

		alice_dtable_t use_bloom_table = alice_new_bool_dtable("use_bloom", scene->renderer->use_bloom);
		alice_dtable_add_child(&renderer_table, use_bloom_table);

		alice_dtable_t bloom_threshold_table = alice_new_number_dtable("bloom_threshold",
				scene->renderer->bloom_threshold);
		alice_dtable_add_child(&renderer_table, bloom_threshold_table);

		alice_dtable_t bloom_blur_iterations_table = alice_new_number_dtable("bloom_blur_iterations",
				scene->renderer->bloom_blur_iterations);
		alice_dtable_add_child(&renderer_table, bloom_blur_iterations_table);

		alice_dtable_t use_antialiasing_table = alice_new_bool_dtable("use_antialiasing",
				scene->renderer->use_antialiasing);
		alice_dtable_add_child(&renderer_table, use_antialiasing_table);

		alice_dtable_t shadowmap_resolution_table =
			alice_new_number_dtable("shadowmap_resolution", scene->renderer->shadowmap->res);
		alice_dtable_add_child(&renderer_table, shadowmap_resolution_table);

		alice_dtable_add_child(&settings_table, renderer_table);
	}

	if (scene->physics_engine) {
		alice_dtable_t physics_table = alice_new_empty_dtable("physics");

		alice_dtable_t gravity_table = alice_new_number_dtable("gravity", scene->physics_engine->gravity);
		alice_dtable_add_child(&physics_table, gravity_table);

		alice_dtable_add_child(&settings_table, physics_table);
	}

	if (scene->renderer_2d) {
		alice_dtable_t renderer_table = alice_new_empty_dtable("renderer_2d");

		const char* sprite_shader_path =
			alice_get_shader_resource_filename(scene->renderer_2d->sprite_shader);

		alice_dtable_t sprite_shader_path_table =
			alice_new_string_dtable("sprite_shader", sprite_shader_path);
		alice_dtable_add_child(&renderer_table, sprite_shader_path_table);

		alice_dtable_add_child(&settings_table, renderer_table);
	}

	alice_dtable_add_child(&table, settings_table);

	alice_dtable_t entities_table = alice_new_empty_dtable("entities");
	for (u32 i = 0; i < scene->pool_count; i++) {
		alice_entity_pool_t* pool = &scene->pools[i];
		for (u32 ii = 0; ii < pool->count; ii++) {
			alice_entity_handle_t handle = alice_new_entity_handle(ii, pool->type_id);

			alice_entity_t* ptr = alice_entity_pool_get(pool, ii);

			if (ptr->parent == alice_null_entity_handle) {
				alice_serialise_entity(&entities_table, scene, handle);
			}
		}
	}

	alice_dtable_add_child(&table, entities_table);

	alice_write_dtable(&table, file_path);

	alice_deinit_dtable(&table);
}

static alice_serialisable_type_t alice_determine_dtable_type(alice_dtable_t* table) {
	assert(table);

	if (strcmp(table->name, "renderable_3d") == 0) {
		return ALICE_ST_RENDERABLE3D;
	} else if (strcmp(table->name, "camera_3d") == 0) {
		return ALICE_ST_CAMERA3D;
	} else if (strcmp(table->name, "camera_2d") == 0) {
		return ALICE_ST_CAMERA2D;
	} else if (strcmp(table->name, "point_light") == 0) {
		return ALICE_ST_POINTLIGHT;
	} else if (strcmp(table->name, "directional_light") == 0) {
		return ALICE_ST_DIRECTIONALLIGHT;
	} else if (strcmp(table->name, "rigidbody_3d") == 0) {
		return ALICE_ST_RIGIDBODY3D;
	} else if (strcmp(table->name, "sprite_2d") == 0) {
		return ALICE_ST_SPRITE2D;
	}

	return ALICE_ST_ENTITY;
}

static alice_entity_handle_t alice_deserialise_entity(alice_dtable_t* table, alice_scene_t* scene) {
	assert(table);
	assert(scene);

	const alice_serialisable_type_t entity_type = alice_determine_dtable_type(table);

	alice_type_info_t create_type = alice_get_type_info(alice_entity_t);

	switch (entity_type) {
		case ALICE_ST_RENDERABLE3D:
			create_type = alice_get_type_info(alice_renderable_3d_t);
			break;
		case ALICE_ST_CAMERA3D:
			create_type = alice_get_type_info(alice_camera_3d_t);
			break;
		case ALICE_ST_CAMERA2D:
			create_type = alice_get_type_info(alice_camera_2d_t);
			break;
		case ALICE_ST_POINTLIGHT:
			create_type = alice_get_type_info(alice_point_light_t);
			break;
		case ALICE_ST_DIRECTIONALLIGHT:
			create_type = alice_get_type_info(alice_directional_light_t);
			break;
		case ALICE_ST_RIGIDBODY3D:
			create_type = alice_get_type_info(alice_rigidbody_3d_t);
			break;
		case ALICE_ST_SPRITE2D:
			create_type = alice_get_type_info(alice_sprite_2d_t);
			break;
		default:
			break;
	}

	const alice_entity_handle_t handle = impl_alice_new_entity(scene, create_type);

	alice_entity_t* entity = alice_get_entity_ptr(scene, handle);

	alice_dtable_t* name_table = alice_dtable_find_child(table, "name");
	if (name_table && name_table->value.type == ALICE_DTABLE_STRING) {
		entity->name = alice_copy_string(name_table->value.as.string);
	}

	alice_dtable_t* position_table = alice_dtable_find_child(table, "position");
	if (position_table) {
		alice_dtable_t* x_table = alice_dtable_find_child(position_table, "x");
		if (x_table && x_table->value.type == ALICE_DTABLE_NUMBER) {
			entity->position.x = (float)x_table->value.as.number;
		}

		alice_dtable_t* y_table = alice_dtable_find_child(position_table, "y");
		if (y_table && y_table->value.type == ALICE_DTABLE_NUMBER) {
			entity->position.y = (float)y_table->value.as.number;
		}

		alice_dtable_t* z_table = alice_dtable_find_child(position_table, "z");
		if (z_table && z_table->value.type == ALICE_DTABLE_NUMBER) {
			entity->position.z = (float)z_table->value.as.number;
		}
	}

	alice_dtable_t* rotation_table = alice_dtable_find_child(table, "rotation");
	if (rotation_table) {
		alice_dtable_t* x_table = alice_dtable_find_child(rotation_table, "x");
		if (x_table && x_table->value.type == ALICE_DTABLE_NUMBER) {
			entity->rotation.x = (float)x_table->value.as.number;
		}

		alice_dtable_t* y_table = alice_dtable_find_child(rotation_table, "y");
		if (y_table && y_table->value.type == ALICE_DTABLE_NUMBER) {
			entity->rotation.y = (float)y_table->value.as.number;
		}

		alice_dtable_t* z_table = alice_dtable_find_child(rotation_table, "z");
		if (z_table && z_table->value.type == ALICE_DTABLE_NUMBER) {
			entity->rotation.z = (float)z_table->value.as.number;
		}
	}

	alice_dtable_t* scale_table = alice_dtable_find_child(table, "scale");
	if (scale_table) {
		alice_dtable_t* x_table = alice_dtable_find_child(scale_table, "x");
		if (x_table && x_table->value.type == ALICE_DTABLE_NUMBER) {
			entity->scale.x = (float)x_table->value.as.number;
		}

		alice_dtable_t* y_table = alice_dtable_find_child(scale_table, "y");
		if (y_table && y_table->value.type == ALICE_DTABLE_NUMBER) {
			entity->scale.y = (float)y_table->value.as.number;
		}

		alice_dtable_t* z_table = alice_dtable_find_child(scale_table, "z");
		if (z_table && z_table->value.type == ALICE_DTABLE_NUMBER) {
			entity->scale.z = (float)z_table->value.as.number;
		}
	}

	alice_dtable_t* script_table = alice_dtable_find_child(table, "script");
	if (script_table) {
		alice_dtable_t* get_instance_size_table = alice_dtable_find_child(script_table, "get_instance_size");
		alice_dtable_t* on_init_table = alice_dtable_find_child(script_table, "on_init");
		alice_dtable_t* on_update_table = alice_dtable_find_child(script_table, "on_update");
		alice_dtable_t* on_physics_update_table = alice_dtable_find_child(script_table, "on_physics_update");
		alice_dtable_t* on_free_table = alice_dtable_find_child(script_table, "on_free");

		const char* get_instance_size_name = alice_null;
		const char* on_init_name = alice_null;
		const char* on_update_name = alice_null;
		const char* on_physics_update_name = alice_null;
		const char* on_free_name = alice_null;

		if (get_instance_size_table && get_instance_size_table->value.type == ALICE_DTABLE_STRING) {
			get_instance_size_name = get_instance_size_table->value.as.string;
		}

		if (on_init_table && on_init_table->value.type == ALICE_DTABLE_STRING) {
			on_init_name = on_init_table->value.as.string;
		}

		if (on_update_table && on_update_table->value.type == ALICE_DTABLE_STRING) {
			on_update_name = on_update_table->value.as.string;
		}

		if (on_physics_update_table && on_physics_update_table->value.type == ALICE_DTABLE_STRING) {
			on_physics_update_name = on_physics_update_table->value.as.string;
		}

		if (on_free_table && on_free_table->value.type == ALICE_DTABLE_STRING) {
			on_free_name = on_free_table->value.as.string;
		}

		entity->script = alice_new_script(scene->script_context, handle,
				get_instance_size_name,
				on_init_name,
				on_update_name,
				on_physics_update_name,
				on_free_name, false);

	}

	switch (entity_type) {
		case ALICE_ST_RENDERABLE3D: {
			alice_renderable_3d_t* renderable = (alice_renderable_3d_t*)entity;

			alice_dtable_t* cast_shadows_table = alice_dtable_find_child(table, "cast_shadows");
			if (cast_shadows_table && cast_shadows_table->value.type == ALICE_DTABLE_BOOL) {
				renderable->cast_shadows = cast_shadows_table->value.as.boolean;
			}

			alice_dtable_t* model_path_table = alice_dtable_find_child(table, "model");
			if (model_path_table && model_path_table->value.type == ALICE_DTABLE_STRING) {
				renderable->model = alice_load_model(model_path_table->value.as.string);
			}

			alice_dtable_t* materials_table = alice_dtable_find_child(table, "materials");
			if (materials_table && materials_table->value.type == ALICE_DTABLE_ARRAY) {
				alice_dtable_value_array_t* array = materials_table->value.as.array;
				for (u32 i = 0; i < array->count; i++) {
					alice_dtable_value_t* value = &array->values[i];

					if (value->type == ALICE_DTABLE_STRING) {
						alice_renderable_3d_add_material(renderable, value->as.string);
					}
				}
			}

			break;
		}
		case ALICE_ST_CAMERA3D: {
			alice_camera_3d_t* camera = (alice_camera_3d_t*)entity;

			alice_dtable_t* fov_table = alice_dtable_find_child(table, "fov");
			if (fov_table && fov_table->value.type == ALICE_DTABLE_NUMBER) {
				camera->fov = (float)fov_table->value.as.number;
			}

			alice_dtable_t* near_table = alice_dtable_find_child(table, "near");
			if (fov_table && near_table->value.type == ALICE_DTABLE_NUMBER) {
				camera->near = (float)near_table->value.as.number;
			}

			alice_dtable_t* far_table = alice_dtable_find_child(table, "far");
			if (far_table && far_table->value.type == ALICE_DTABLE_NUMBER) {
				camera->far = (float)far_table->value.as.number;
			}

			alice_dtable_t* exposure_table = alice_dtable_find_child(table, "exposure");
			if (exposure_table && exposure_table->value.type == ALICE_DTABLE_NUMBER) {
				camera->exposure = (float)exposure_table->value.as.number;
			}

			alice_dtable_t* gamma_table = alice_dtable_find_child(table, "gamma");
			if (gamma_table && gamma_table->value.type == ALICE_DTABLE_NUMBER) {
				camera->gamma = (float)gamma_table->value.as.number;
			}

			alice_dtable_t* active_table = alice_dtable_find_child(table, "active");
			if (active_table && active_table->value.type == ALICE_DTABLE_BOOL) {
				camera->active = active_table->value.as.number;
			}

			break;
		}
		case ALICE_ST_CAMERA2D: {
			alice_camera_2d_t* camera = (alice_camera_2d_t*)entity;

			alice_dtable_t* dimentions_table = alice_dtable_find_child(table, "dimentions");
			if (dimentions_table) {
				alice_dtable_t* x_table = alice_dtable_find_child(dimentions_table, "x");
				if (x_table && x_table->value.type == ALICE_DTABLE_NUMBER) {
					camera->dimentions.x = (float)x_table->value.as.number;
				}

				alice_dtable_t* y_table = alice_dtable_find_child(dimentions_table, "y");
				if (y_table && y_table->value.type == ALICE_DTABLE_NUMBER) {
					camera->dimentions.y = (float)y_table->value.as.number;
				}
			}

			alice_dtable_t* stretch_table = alice_dtable_find_child(table, "stretch");
			if (stretch_table && stretch_table->value.type == ALICE_DTABLE_BOOL) {
				camera->stretch = stretch_table->value.as.boolean;
			}

			alice_dtable_t* active_table = alice_dtable_find_child(table, "active");
			if (active_table && active_table->value.type == ALICE_DTABLE_BOOL) {
				camera->active = active_table->value.as.boolean;
			}
		}
		case ALICE_ST_POINTLIGHT: {
			alice_point_light_t* light = (alice_point_light_t*)entity;

			alice_dtable_t* range_table = alice_dtable_find_child(table, "range");
			if (range_table && range_table->value.type == ALICE_DTABLE_NUMBER) {
				light->range = (float)range_table->value.as.number;
			}

			alice_dtable_t* intensity_table = alice_dtable_find_child(table, "intensity");
			if (intensity_table && intensity_table->value.type == ALICE_DTABLE_NUMBER) {
				light->intensity = (float)intensity_table->value.as.number;
			}

			alice_dtable_t* color_table = alice_dtable_find_child(table, "color");
			if (color_table) {
				alice_rgb_color_t color = (alice_rgb_color_t){1.0, 1.0, 1.0};

				alice_dtable_t* r_table = alice_dtable_find_child(color_table, "r");
				if (r_table && r_table->value.type == ALICE_DTABLE_NUMBER) {
					color.r = (float)r_table->value.as.number;
				}

				alice_dtable_t* g_table = alice_dtable_find_child(color_table, "g");
				if (g_table && g_table->value.type == ALICE_DTABLE_NUMBER) {
					color.g = (float)g_table->value.as.number;
				}

				alice_dtable_t* b_table = alice_dtable_find_child(color_table, "b");
				if (b_table && b_table->value.type == ALICE_DTABLE_NUMBER) {
					color.b = (float)b_table->value.as.number;
				}

				light->color = alice_color_from_rgb_color(color);
			}

			break;
		}
		case ALICE_ST_DIRECTIONALLIGHT: {
			alice_directional_light_t* light = (alice_directional_light_t*)entity;

			alice_dtable_t* intensity_table = alice_dtable_find_child(table, "intensity");
			if (intensity_table && intensity_table->value.type == ALICE_DTABLE_NUMBER) {
				light->intensity = (float)intensity_table->value.as.number;
			}

			alice_dtable_t* cast_shadows_table = alice_dtable_find_child(table, "cast_shadows");
			if (cast_shadows_table && cast_shadows_table->value.type == ALICE_DTABLE_BOOL) {
				light->cast_shadows = cast_shadows_table->value.as.boolean;
			}

			alice_dtable_t* color_table = alice_dtable_find_child(table, "color");
			if (color_table) {
				alice_rgb_color_t color = (alice_rgb_color_t){1.0, 1.0, 1.0};

				alice_dtable_t* r_table = alice_dtable_find_child(color_table, "r");
				if (r_table && r_table->value.type == ALICE_DTABLE_NUMBER) {
					color.r = (float)r_table->value.as.number;
				}

				alice_dtable_t* g_table = alice_dtable_find_child(color_table, "g");
				if (g_table && g_table->value.type == ALICE_DTABLE_NUMBER) {
					color.g = (float)g_table->value.as.number;
				}

				alice_dtable_t* b_table = alice_dtable_find_child(color_table, "b");
				if (b_table && b_table->value.type == ALICE_DTABLE_NUMBER) {
					color.b = (float)b_table->value.as.number;
				}

				light->color = alice_color_from_rgb_color(color);
			}

			break;
		}
		case ALICE_ST_RIGIDBODY3D: {
			alice_rigidbody_3d_t* rigidbody = (alice_rigidbody_3d_t*)entity;

			alice_dtable_t* mass_table = alice_dtable_find_child(table, "mass");
			if (mass_table && mass_table->value.type == ALICE_DTABLE_NUMBER) {
				rigidbody->mass = (float)mass_table->value.as.number;
			}

			alice_dtable_t* restitution_table = alice_dtable_find_child(table, "restitution");
			if (restitution_table && restitution_table->value.type == ALICE_DTABLE_NUMBER) {
				rigidbody->restitution = (float)restitution_table->value.as.number;
			}

			alice_dtable_t* gravity_scale_table = alice_dtable_find_child(table, "gravity_scale");
			if (gravity_scale_table && gravity_scale_table->value.type == ALICE_DTABLE_NUMBER) {
				rigidbody->gravity_scale = (float)gravity_scale_table->value.as.number;
			}

			alice_dtable_t* dynamic_friction_table = alice_dtable_find_child(table, "dynamic_friction");
			if (dynamic_friction_table && dynamic_friction_table->value.type == ALICE_DTABLE_NUMBER) {
				rigidbody->dynamic_friction = (float)dynamic_friction_table->value.as.number;
			}

			alice_dtable_t* static_friction_table = alice_dtable_find_child(table, "static_friction");
			if (static_friction_table && dynamic_friction_table->value.type == ALICE_DTABLE_NUMBER) {
				rigidbody->static_friction = (float)static_friction_table->value.as.number;
			}

			alice_dtable_t* box_table = alice_dtable_find_child(table, "box");
			if (box_table) {
				alice_dtable_t* dimentions_table = alice_dtable_find_child(box_table, "dimentions");
				if (dimentions_table) {
					alice_dtable_t* x_table = alice_dtable_find_child(dimentions_table, "x");
					if (x_table && x_table->value.type == ALICE_DTABLE_NUMBER) {
						rigidbody->box.dimentions.x = (float)x_table->value.as.number;
					}

					alice_dtable_t* y_table = alice_dtable_find_child(dimentions_table, "y");
					if (y_table && y_table->value.type == ALICE_DTABLE_NUMBER) {
						rigidbody->box.dimentions.y = (float)y_table->value.as.number;
					}

					alice_dtable_t* z_table = alice_dtable_find_child(dimentions_table, "z");
					if (z_table && z_table->value.type == ALICE_DTABLE_NUMBER) {
						rigidbody->box.dimentions.z = (float)z_table->value.as.number;
					}
				}

				alice_dtable_t* position_table = alice_dtable_find_child(box_table, "position");
				if (position_table) {
					alice_dtable_t* x_table = alice_dtable_find_child(position_table, "x");
					if (x_table && x_table->value.type == ALICE_DTABLE_NUMBER) {
						rigidbody->box.position.x = (float)x_table->value.as.number;
					}

					alice_dtable_t* y_table = alice_dtable_find_child(position_table, "y");
					if (y_table && y_table->value.type == ALICE_DTABLE_NUMBER) {
						rigidbody->box.position.y = (float)y_table->value.as.number;
					}

					alice_dtable_t* z_table = alice_dtable_find_child(position_table, "z");
					if (z_table && z_table->value.type == ALICE_DTABLE_NUMBER) {
						rigidbody->box.position.z = (float)z_table->value.as.number;
					}
				}
			}

			alice_dtable_t* constraints_table = alice_dtable_find_child(table, "constraints");
			if (constraints_table) {
				alice_dtable_t* x_table = alice_dtable_find_child(constraints_table, "x");
				if (x_table && x_table->value.type == ALICE_DTABLE_BOOL) {
					rigidbody->constraints.x = x_table->value.as.boolean;
				}

				alice_dtable_t* y_table = alice_dtable_find_child(constraints_table, "y");
				if (y_table && y_table->value.type == ALICE_DTABLE_BOOL) {
					rigidbody->constraints.y = y_table->value.as.boolean;
				}

				alice_dtable_t* z_table = alice_dtable_find_child(constraints_table, "z");
				if (z_table && z_table->value.type == ALICE_DTABLE_BOOL) {
					rigidbody->constraints.z = z_table->value.as.boolean;
				}
			}

			break;
		}
		case ALICE_ST_SPRITE2D: {
			alice_sprite_2d_t* sprite = (alice_sprite_2d_t*)entity;

			alice_dtable_t* image_table = alice_dtable_find_child(table, "image");
			if (image_table) {
				const char* image_path = alice_null;
				alice_texture_flags_t flags = 0;

				alice_dtable_t* path_table = alice_dtable_find_child(image_table, "path");
				if (path_table && path_table->value.type == ALICE_DTABLE_STRING) {
					image_path = path_table->value.as.string;
				}

				alice_dtable_t* is_antialiased_table =
					alice_dtable_find_child(image_table, "is_antialiased_table");
				if (is_antialiased_table &&
						is_antialiased_table->value.type == ALICE_DTABLE_BOOL) {
					flags |= ALICE_TEXTURE_ANTIALIASED;
				} else {
					flags |= ALICE_TEXTURE_ALIASED;
				}

				sprite->image = alice_load_texture(image_path, flags);
			}
		}
		default:
			break;
	}

	alice_dtable_t* children_table = alice_dtable_find_child(table, "children");
	if (children_table) {
		for (u32 i = 0; i < children_table->child_count; i++) {
			alice_dtable_t* child_table = &children_table->children[i];
			alice_entity_handle_t child = alice_deserialise_entity(child_table, scene);
			alice_entity_parent_to(scene, child, handle);
		}
	}

	return handle;
}

void alice_deserialise_scene(alice_scene_t* scene, const char* file_path) {
	assert(scene);

	alice_dtable_t* table = alice_read_dtable(alice_load_string(file_path));
	if (!table) { return; }

	alice_dtable_t* settings_table = alice_dtable_find_child(table, "settings");
	if (settings_table) {
		alice_dtable_t* renderer_3d_table = alice_dtable_find_child(settings_table, "renderer_3d");
		if (renderer_3d_table) {
			alice_shader_t* postprocess = alice_null;
			alice_shader_t* blur = alice_null;
			alice_shader_t* extract = alice_null;
			alice_shader_t* debug_shader = alice_null;
			alice_shader_t* depth_shader = alice_null;
			bool debug = false;
			u32 shadowmap_resolution = 1024;

			alice_dtable_t* postprocess_shader_table = alice_dtable_find_child(renderer_3d_table,
					"postprocess_shader");
			if (postprocess_shader_table && postprocess_shader_table->value.type == ALICE_DTABLE_STRING) {
				postprocess = alice_load_shader(postprocess_shader_table->value.as.string);
			}

			alice_dtable_t* bright_extract_shader_table = alice_dtable_find_child(renderer_3d_table,
					"bright_extract_shader");
			if (bright_extract_shader_table &&
					bright_extract_shader_table->value.type == ALICE_DTABLE_STRING) {
				extract = alice_load_shader(bright_extract_shader_table->value.as.string);
			}

			alice_dtable_t* blur_shader_table = alice_dtable_find_child(renderer_3d_table,
					"blur_shader");
			if (blur_shader_table && blur_shader_table->value.type == ALICE_DTABLE_STRING) {
				blur = alice_load_shader(blur_shader_table->value.as.string);
			}

			alice_dtable_t* debug_shader_table = alice_dtable_find_child(renderer_3d_table,
					"line_shader");
			if (debug_shader_table && debug_shader_table->value.type == ALICE_DTABLE_STRING) {
				debug_shader = alice_load_shader(debug_shader_table->value.as.string);
			}

			alice_dtable_t* depth_shader_table = alice_dtable_find_child(renderer_3d_table,
					"depth_shader");
			if (depth_shader_table && depth_shader_table->value.type == ALICE_DTABLE_STRING) {
				depth_shader = alice_load_shader(depth_shader_table->value.as.string);
			}

			alice_dtable_t* debug_table = alice_dtable_find_child(renderer_3d_table, "debug");
			if (debug_table && debug_table->value.type == ALICE_DTABLE_BOOL) {
				debug = debug_table->value.as.boolean;
			}

			alice_dtable_t* shadowmap_resolution_table =
				alice_dtable_find_child(renderer_3d_table, "shadowmap_resolution");
			if (shadowmap_resolution_table && shadowmap_resolution_table->value.type == ALICE_DTABLE_NUMBER) {
				shadowmap_resolution = (u32)shadowmap_resolution_table->value.as.number;
			}

			scene->renderer = alice_new_scene_renderer_3d(postprocess, extract, blur, depth_shader,
					debug, debug_shader, shadowmap_resolution);
			scene->renderer->use_antialiasing = true;
			scene->renderer->use_bloom = true;

			alice_dtable_t* use_bloom_table = alice_dtable_find_child(renderer_3d_table,
					"use_bloom");
			if (use_bloom_table && use_bloom_table->value.type == ALICE_DTABLE_BOOL) {
				scene->renderer->use_bloom = use_bloom_table->value.as.boolean;
			}

			alice_dtable_t* bloom_threshold_table = alice_dtable_find_child(renderer_3d_table, "bloom_threshold");
			if (bloom_threshold_table && bloom_threshold_table->value.type == ALICE_DTABLE_NUMBER) {
				scene->renderer->bloom_threshold = (float)bloom_threshold_table->value.as.number;
			}

			alice_dtable_t* bloom_blur_iterations_table = alice_dtable_find_child(renderer_3d_table,
					"bloom_blur_iterations");
			if (bloom_blur_iterations_table &&
					bloom_blur_iterations_table->value.type == ALICE_DTABLE_NUMBER) {
				scene->renderer->bloom_blur_iterations =
					(u32)bloom_blur_iterations_table->value.as.number;
			}

			alice_dtable_t* use_antialiasing_table = alice_dtable_find_child(renderer_3d_table,
					"use_antialiasing");
			if (use_antialiasing_table && use_antialiasing_table->value.type == ALICE_DTABLE_BOOL) {
				scene->renderer->use_antialiasing = use_antialiasing_table->value.as.boolean;
			}
		}

		alice_dtable_t* physics_table = alice_dtable_find_child(settings_table, "physics");
		if (physics_table) {
			scene->physics_engine = alice_new_physics_engine(scene);

			alice_dtable_t* gravity_table = alice_dtable_find_child(physics_table, "gravity");
			if (gravity_table && gravity_table->value.type == ALICE_DTABLE_NUMBER) {
				scene->physics_engine->gravity = (float)gravity_table->value.as.number;
			}
		}

		alice_dtable_t* renderer_2d_table = alice_dtable_find_child(settings_table, "renderer_2d");
		if (renderer_2d_table) {
			alice_shader_t* sprite_shader = alice_null;

			alice_dtable_t* sprite_shader_path_table =
				alice_dtable_find_child(renderer_2d_table, "sprite_shader");
			if (sprite_shader_path_table &&
					sprite_shader_path_table->value.type == ALICE_DTABLE_STRING) {
				sprite_shader = alice_load_shader(sprite_shader_path_table->value.as.string);
			}

			scene->renderer_2d = alice_new_scene_renderer_2d(sprite_shader);
		}
	}

	alice_dtable_t* entities_table = alice_dtable_find_child(table, "entities");
	for (u32 i = 0; i < entities_table->child_count; i++) {
		alice_dtable_t* child_table = &entities_table->children[i];
		alice_deserialise_entity(child_table, scene);
	}

	alice_free_dtable(table);
}

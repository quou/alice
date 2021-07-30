#include <assert.h>
#include <string.h>

#include "alice/sceneserialise.h"
#include "alice/dtable.h"
#include "alice/graphics.h"
#include "alice/scripting.h"
#include "alice/physics.h"
#include "alice/debugrenderer.h"

typedef enum alice_SerialisableType {
	ALICE_ST_ENTITY,
	ALICE_ST_RENDERABLE3D,
	ALICE_ST_POINTLIGHT,
	ALICE_ST_DIRECTIONALLIGHT,
	ALICE_ST_CAMERA3D,
	ALICE_ST_RIGIDBODY3D
} alice_SerialisableType;

static alice_SerialisableType alice_determine_entity_type(alice_EntityHandle handle) {
	u32 type_id = alice_get_entity_handle_type(handle);

	if (type_id == alice_get_type_info(alice_Renderable3D).id) {
		return ALICE_ST_RENDERABLE3D;
	} else if (type_id == alice_get_type_info(alice_PointLight).id) {
		return ALICE_ST_POINTLIGHT;
	} else if (type_id == alice_get_type_info(alice_Camera3D).id) {
		return ALICE_ST_CAMERA3D;
	} else if (type_id == alice_get_type_info(alice_DirectionalLight).id) {
		return ALICE_ST_DIRECTIONALLIGHT;
	} else if (type_id == alice_get_type_info(alice_Rigidbody3D).id) {
		return ALICE_ST_RIGIDBODY3D;
	}

	return ALICE_ST_ENTITY;
}

static void alice_serialise_entity(alice_DTable* table, alice_Scene* scene, alice_EntityHandle handle) {
	assert(table && scene);

	alice_Entity* entity = alice_get_entity_ptr(scene, handle);

	alice_SerialisableType entity_type = alice_determine_entity_type(handle);

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
		case ALICE_ST_DIRECTIONALLIGHT:
			type_name = "directional_light";
			break;
		case ALICE_ST_RIGIDBODY3D:
			type_name = "rigidbody_3d";
			break;
	}

	alice_DTable entity_table = alice_new_empty_dtable(type_name);

	if (entity->name) {
		alice_DTable name_table = alice_new_string_dtable("name", entity->name);
		alice_dtable_add_child(&entity_table, name_table);
	}

	alice_DTable position_table = alice_new_empty_dtable("position");
	{
		alice_DTable x_table = alice_new_number_dtable("x", entity->position.x);
		alice_DTable y_table = alice_new_number_dtable("y", entity->position.y);
		alice_DTable z_table = alice_new_number_dtable("z", entity->position.z);

		alice_dtable_add_child(&position_table, x_table);
		alice_dtable_add_child(&position_table, y_table);
		alice_dtable_add_child(&position_table, z_table);
	}
	alice_dtable_add_child(&entity_table, position_table);

	alice_DTable rotation_table = alice_new_empty_dtable("rotation");
	{
		alice_DTable x_table = alice_new_number_dtable("x", entity->rotation.x);
		alice_DTable y_table = alice_new_number_dtable("y", entity->rotation.y);
		alice_DTable z_table = alice_new_number_dtable("z", entity->rotation.z);

		alice_dtable_add_child(&rotation_table, x_table);
		alice_dtable_add_child(&rotation_table, y_table);
		alice_dtable_add_child(&rotation_table, z_table);
	}
	alice_dtable_add_child(&entity_table, rotation_table);

	alice_DTable scale_table = alice_new_empty_dtable("scale");
	{
		alice_DTable x_table = alice_new_number_dtable("x", entity->scale.x);
		alice_DTable y_table = alice_new_number_dtable("y", entity->scale.y);
		alice_DTable z_table = alice_new_number_dtable("z", entity->scale.z);

		alice_dtable_add_child(&scale_table, x_table);
		alice_dtable_add_child(&scale_table, y_table);
		alice_dtable_add_child(&scale_table, z_table);
	}
	alice_dtable_add_child(&entity_table, scale_table);

	if (entity->script) {
		alice_DTable script_table = alice_new_empty_dtable("script");

		if (entity->script->get_instance_size_name) {
			alice_DTable get_instance_size_table =
				alice_new_string_dtable("get_instance_size", entity->script->get_instance_size_name);
			alice_dtable_add_child(&script_table, get_instance_size_table);
		}

		if (entity->script->on_init_name) {
			alice_DTable on_init_table = alice_new_string_dtable("on_init", entity->script->on_init_name);
			alice_dtable_add_child(&script_table, on_init_table);
		}

		if (entity->script->on_update_name) {
			alice_DTable on_update_table = alice_new_string_dtable("on_update", entity->script->on_update_name);
			alice_dtable_add_child(&script_table, on_update_table);
		}

		if (entity->script->on_physics_update_name) {
			alice_DTable on_physics_update_table = alice_new_string_dtable("on_physics_update", entity->script->on_physics_update_name);
			alice_dtable_add_child(&script_table, on_physics_update_table);
		}

		if (entity->script->on_free_name) {
			alice_DTable on_free_table = alice_new_string_dtable("on_free", entity->script->on_free_name);
			alice_dtable_add_child(&script_table, on_free_table);
		}


		alice_dtable_add_child(&entity_table, script_table);
	}

	switch (entity_type) {
		case ALICE_ST_RENDERABLE3D: {
			alice_Renderable3D* renderable = (alice_Renderable3D*)entity;

			const char* model_path = alice_get_model_resource_filename(renderable->model);

			alice_DTable model_table = alice_new_string_dtable("model", model_path);
			alice_dtable_add_child(&entity_table, model_table);

			alice_DTableValueArray* material_array = alice_new_dtable_value_array();

			for (u32 i = 0; i < renderable->material_count; i++) {
				alice_Material* material = renderable->materials[i];

				const char* material_path =
					alice_get_material_resource_filename(material);
				if (strcmp(material_path, "default_material") != 0) {
					alice_save_material(material, material_path);
				}

				alice_DTableValue material_path_value;
				material_path_value.type = ALICE_DTABLE_STRING;
				material_path_value.as.string = alice_copy_string(material_path);

				alice_dtable_value_array_add(material_array, material_path_value);
			}

			alice_DTable materials_table = alice_new_array_dtable("materials", material_array);
			alice_dtable_add_child(&entity_table, materials_table);

			break;
		    }
		case ALICE_ST_POINTLIGHT: {
			alice_PointLight* light = (alice_PointLight*)entity;

			alice_RGBColor color = alice_rgb_color_from_color(light->color);

			alice_DTable color_table = alice_new_empty_dtable("color");
			alice_DTable r_table = alice_new_number_dtable("r", color.r);
			alice_DTable g_table = alice_new_number_dtable("g", color.g);
			alice_DTable b_table = alice_new_number_dtable("b", color.b);

			alice_dtable_add_child(&color_table, r_table);
			alice_dtable_add_child(&color_table, g_table);
			alice_dtable_add_child(&color_table, b_table);

			alice_dtable_add_child(&entity_table, color_table);

			alice_DTable range_table = alice_new_number_dtable("range", light->range);
			alice_dtable_add_child(&entity_table, range_table);

			alice_DTable intensity_table = alice_new_number_dtable("intensity", light->intensity);
			alice_dtable_add_child(&entity_table, intensity_table);

			break;
		}
		case ALICE_ST_DIRECTIONALLIGHT: {
			alice_DirectionalLight* light = (alice_DirectionalLight*)entity;

			alice_RGBColor color = alice_rgb_color_from_color(light->color);

			alice_DTable color_table = alice_new_empty_dtable("color");
			alice_DTable r_table = alice_new_number_dtable("r", color.r);
			alice_DTable g_table = alice_new_number_dtable("g", color.g);
			alice_DTable b_table = alice_new_number_dtable("b", color.b);

			alice_dtable_add_child(&color_table, r_table);
			alice_dtable_add_child(&color_table, g_table);
			alice_dtable_add_child(&color_table, b_table);

			alice_dtable_add_child(&entity_table, color_table);

			alice_DTable intensity_table = alice_new_number_dtable("intensity", light->intensity);
			alice_dtable_add_child(&entity_table, intensity_table);

			break;
		}
		case ALICE_ST_CAMERA3D: {
			alice_Camera3D* camera = (alice_Camera3D*)entity;

			alice_DTable fov_table = alice_new_number_dtable("fov", camera->fov);
			alice_dtable_add_child(&entity_table, fov_table);

			alice_DTable near_table = alice_new_number_dtable("near", camera->near);
			alice_dtable_add_child(&entity_table, near_table);

			alice_DTable far_table = alice_new_number_dtable("far", camera->far);
			alice_dtable_add_child(&entity_table, far_table);

			alice_DTable exposure_table = alice_new_number_dtable("exposure", camera->exposure);
			alice_dtable_add_child(&entity_table, exposure_table);

			alice_DTable gamma_table = alice_new_number_dtable("gamma", camera->gamma);
			alice_dtable_add_child(&entity_table, gamma_table);

			alice_DTable active_table = alice_new_bool_dtable("active", camera->active);
			alice_dtable_add_child(&entity_table, active_table);
			break;
		}
		case ALICE_ST_RIGIDBODY3D: {
			alice_Rigidbody3D* rigidbody = (alice_Rigidbody3D*)entity;

			alice_DTable mass_table = alice_new_number_dtable("mass", rigidbody->mass);
			alice_dtable_add_child(&entity_table, mass_table);

			alice_DTable static_friction_table = alice_new_number_dtable(
					"dynamic_friction", rigidbody->dynamic_friction);
			alice_dtable_add_child(&entity_table, static_friction_table);

			alice_DTable dynamic_friction_table = alice_new_number_dtable(
					"static_friction", rigidbody->static_friction);
			alice_dtable_add_child(&entity_table, dynamic_friction_table);

			alice_DTable restitution_table = alice_new_number_dtable("restitution",
					rigidbody->restitution);
			alice_dtable_add_child(&entity_table, restitution_table);

			alice_DTable gravity_scale_table = alice_new_number_dtable("gravity_scale",
					rigidbody->gravity_scale);
			alice_dtable_add_child(&entity_table, gravity_scale_table);

			alice_DTable box_table = alice_new_empty_dtable("box");

			{
				alice_DTable dimentions_table = alice_new_empty_dtable("dimentions");

				alice_DTable x_table = alice_new_number_dtable("x", rigidbody->box.dimentions.x);
				alice_dtable_add_child(&dimentions_table, x_table);

				alice_DTable y_table = alice_new_number_dtable("y", rigidbody->box.dimentions.y);
				alice_dtable_add_child(&dimentions_table, y_table);

				alice_DTable z_table = alice_new_number_dtable("z", rigidbody->box.dimentions.z);
				alice_dtable_add_child(&dimentions_table, z_table);

				alice_dtable_add_child(&box_table, dimentions_table);
			}

			{
				alice_DTable position_table = alice_new_empty_dtable("position");

				alice_DTable x_table = alice_new_number_dtable("x", rigidbody->box.position.x);
				alice_dtable_add_child(&position_table, x_table);

				alice_DTable y_table = alice_new_number_dtable("y", rigidbody->box.position.y);
				alice_dtable_add_child(&position_table, y_table);

				alice_DTable z_table = alice_new_number_dtable("z", rigidbody->box.position.z);
				alice_dtable_add_child(&position_table, z_table);

				alice_dtable_add_child(&box_table, position_table);
			}

			alice_dtable_add_child(&entity_table, box_table);

			break;
		}
	}

	if (entity->child_count > 0) {
		alice_DTable children_table = alice_new_empty_dtable("children");

		for (u32 i = 0; i < entity->child_count; i++) {
			alice_serialise_entity(&children_table, scene, entity->children[i]);
		}

		alice_dtable_add_child(&entity_table, children_table);
	}

	alice_dtable_add_child(table, entity_table);
}

void alice_serialise_scene(alice_Scene* scene, const char* file_path) {
	alice_DTable table = alice_new_empty_dtable("scene");

	alice_DTable settings_table = alice_new_empty_dtable("settings");
	if (scene->renderer) {
		if (scene->renderer->postprocess) {
			alice_DTable shader_table = alice_new_string_dtable("postprocess_shader",
					alice_get_shader_resource_filename(scene->renderer->postprocess));
			alice_dtable_add_child(&settings_table, shader_table);
		}

		if (scene->renderer->extract) {
			alice_DTable shader_table = alice_new_string_dtable("bright_extract_shader",
					alice_get_shader_resource_filename(scene->renderer->extract));
			alice_dtable_add_child(&settings_table, shader_table);
		}

		if (scene->renderer->blur) {
			alice_DTable shader_table = alice_new_string_dtable("blur_shader",
					alice_get_shader_resource_filename(scene->renderer->blur));
			alice_dtable_add_child(&settings_table, shader_table);
		}

		if (scene->renderer->shadowmap->shader) {
			alice_DTable shader_table = alice_new_string_dtable("depth_shader",
					alice_get_shader_resource_filename(scene->renderer->shadowmap->shader));
			alice_dtable_add_child(&settings_table, shader_table);
		}

		alice_DTable debug_table = alice_new_bool_dtable("debug", scene->renderer->debug);
		alice_dtable_add_child(&settings_table, debug_table);

		if (scene->renderer->debug) {
			alice_DTable shader_table = alice_new_string_dtable("line_shader",
					alice_get_shader_resource_filename(
						scene->renderer->debug_renderer->line_shader));
			alice_dtable_add_child(&settings_table, shader_table);
		}

		alice_DTable use_bloom_table = alice_new_bool_dtable("use_bloom", scene->renderer->use_bloom);
		alice_dtable_add_child(&settings_table, use_bloom_table);

		alice_DTable bloom_threshold_table = alice_new_number_dtable("bloom_threshold",
				scene->renderer->bloom_threshold);
		alice_dtable_add_child(&settings_table, bloom_threshold_table);

		alice_DTable bloom_blur_iterations_table = alice_new_number_dtable("bloom_blur_iterations",
				scene->renderer->bloom_blur_iterations);
		alice_dtable_add_child(&settings_table, bloom_blur_iterations_table);

		alice_DTable use_antialiasing_table = alice_new_bool_dtable("use_antialiasing",
				scene->renderer->use_antialiasing);
		alice_dtable_add_child(&settings_table, use_antialiasing_table);
	}

	if (scene->physics_engine) {
		alice_DTable gravity_table = alice_new_number_dtable("gravity", scene->physics_engine->gravity);
		alice_dtable_add_child(&settings_table, gravity_table);
	}

	alice_dtable_add_child(&table, settings_table);

	alice_DTable entities_table = alice_new_empty_dtable("entities");
	for (u32 i = 0; i < scene->pool_count; i++) {
		alice_EntityPool* pool = &scene->pools[i];
		for (u32 ii = 0; ii < pool->count; ii++) {
			alice_EntityHandle handle = alice_new_entity_handle(ii, pool->type_id);

			alice_Entity* ptr = alice_entity_pool_get(pool, ii);

			if (ptr->parent == alice_null_entity_handle) {
				alice_serialise_entity(&entities_table, scene, handle);
			}
		}
	}

	alice_dtable_add_child(&table, entities_table);

	alice_write_dtable(&table, file_path);

	alice_deinit_dtable(&table);
}

static alice_SerialisableType alice_determine_dtable_type(alice_DTable* table) {
	assert(table);

	if (strcmp(table->name, "renderable_3d") == 0) {
		return ALICE_ST_RENDERABLE3D;
	} else if (strcmp(table->name, "camera_3d") == 0) {
		return ALICE_ST_CAMERA3D;
	} else if (strcmp(table->name, "point_light") == 0) {
		return ALICE_ST_POINTLIGHT;
	} else if (strcmp(table->name, "directional_light") == 0) {
		return ALICE_ST_DIRECTIONALLIGHT;
	} else if (strcmp(table->name, "rigidbody_3d") == 0) {
		return ALICE_ST_RIGIDBODY3D;
	}

	return ALICE_ST_ENTITY;
}

static alice_EntityHandle alice_deserialise_entity(alice_DTable* table, alice_Scene* scene) {
	assert(table);
	assert(scene);

	const alice_SerialisableType entity_type = alice_determine_dtable_type(table);

	alice_TypeInfo create_type = alice_get_type_info(alice_Entity);

	switch (entity_type) {
		case ALICE_ST_RENDERABLE3D:
			create_type = alice_get_type_info(alice_Renderable3D);
			break;
		case ALICE_ST_CAMERA3D:
			create_type = alice_get_type_info(alice_Camera3D);
			break;
		case ALICE_ST_POINTLIGHT:
			create_type = alice_get_type_info(alice_PointLight);
			break;
		case ALICE_ST_DIRECTIONALLIGHT:
			create_type = alice_get_type_info(alice_DirectionalLight);
			break;
		case ALICE_ST_RIGIDBODY3D:
			create_type = alice_get_type_info(alice_Rigidbody3D);
			break;
		default:
			break;
	}

	const alice_EntityHandle handle = impl_alice_new_entity(scene, create_type);

	alice_Entity* entity = alice_get_entity_ptr(scene, handle);

	alice_DTable* name_table = alice_dtable_find_child(table, "name");
	if (name_table && name_table->value.type == ALICE_DTABLE_STRING) {
		entity->name = alice_copy_string(name_table->value.as.string);
	}

	alice_DTable* position_table = alice_dtable_find_child(table, "position");
	if (position_table) {
		alice_DTable* x_table = alice_dtable_find_child(position_table, "x");
		if (x_table && x_table->value.type == ALICE_DTABLE_NUMBER) {
			entity->position.x = x_table->value.as.number;
		}

		alice_DTable* y_table = alice_dtable_find_child(position_table, "y");
		if (y_table && y_table->value.type == ALICE_DTABLE_NUMBER) {
			entity->position.y = y_table->value.as.number;
		}

		alice_DTable* z_table = alice_dtable_find_child(position_table, "z");
		if (z_table && z_table->value.type == ALICE_DTABLE_NUMBER) {
			entity->position.z = z_table->value.as.number;
		}
	}

	alice_DTable* rotation_table = alice_dtable_find_child(table, "rotation");
	if (rotation_table) {
		alice_DTable* x_table = alice_dtable_find_child(rotation_table, "x");
		if (x_table && x_table->value.type == ALICE_DTABLE_NUMBER) {
			entity->rotation.x = x_table->value.as.number;
		}

		alice_DTable* y_table = alice_dtable_find_child(rotation_table, "y");
		if (y_table && y_table->value.type == ALICE_DTABLE_NUMBER) {
			entity->rotation.y = y_table->value.as.number;
		}

		alice_DTable* z_table = alice_dtable_find_child(rotation_table, "z");
		if (z_table && z_table->value.type == ALICE_DTABLE_NUMBER) {
			entity->rotation.z = z_table->value.as.number;
		}
	}

	alice_DTable* scale_table = alice_dtable_find_child(table, "scale");
	if (scale_table) {
		alice_DTable* x_table = alice_dtable_find_child(scale_table, "x");
		if (x_table && x_table->value.type == ALICE_DTABLE_NUMBER) {
			entity->scale.x = x_table->value.as.number;
		}

		alice_DTable* y_table = alice_dtable_find_child(scale_table, "y");
		if (y_table && y_table->value.type == ALICE_DTABLE_NUMBER) {
			entity->scale.y = y_table->value.as.number;
		}

		alice_DTable* z_table = alice_dtable_find_child(scale_table, "z");
		if (z_table && z_table->value.type == ALICE_DTABLE_NUMBER) {
			entity->scale.z = z_table->value.as.number;
		}
	}

	alice_DTable* script_table = alice_dtable_find_child(table, "script");
	if (script_table) {
		alice_DTable* get_instance_size_table = alice_dtable_find_child(script_table, "get_instance_size");
		alice_DTable* on_init_table = alice_dtable_find_child(script_table, "on_init");
		alice_DTable* on_update_table = alice_dtable_find_child(script_table, "on_update");
		alice_DTable* on_physics_update_table = alice_dtable_find_child(script_table, "on_physics_update");
		alice_DTable* on_free_table = alice_dtable_find_child(script_table, "on_free");

		const char* get_instance_size_name = alice_null;
		const char* on_init_name = alice_null;
		const char* on_update_name = alice_null;
		const char* on_physics_update_name = alice_null;
		const char* on_free_name = alice_null;

		if (get_instance_size_table && get_instance_size_table->value.type == ALICE_DTABLE_STRING) {
			get_instance_size_name = alice_copy_string(get_instance_size_table->value.as.string);
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
			alice_Renderable3D* renderable = (alice_Renderable3D*)entity;

			alice_DTable* model_path_table = alice_dtable_find_child(table, "model");
			if (model_path_table && model_path_table->value.type == ALICE_DTABLE_STRING) {
				renderable->model = alice_load_model(model_path_table->value.as.string);
			}

			alice_DTable* materials_table = alice_dtable_find_child(table, "materials");
			if (materials_table && materials_table->value.type == ALICE_DTABLE_ARRAY) {
				alice_DTableValueArray* array = materials_table->value.as.array;
				for (u32 i = 0; i < array->count; i++) {
					alice_DTableValue* value = &array->values[i];

					if (value->type == ALICE_DTABLE_STRING) {
						alice_renderable_3d_add_material(renderable, value->as.string);
					}
				}
			}

			break;
		}
		case ALICE_ST_CAMERA3D: {
			alice_Camera3D* camera = (alice_Camera3D*)entity;

			alice_DTable* fov_table = alice_dtable_find_child(table, "fov");
			if (fov_table && fov_table->value.type == ALICE_DTABLE_NUMBER) {
				camera->fov = fov_table->value.as.number;
			}

			alice_DTable* near_table = alice_dtable_find_child(table, "near");
			if (fov_table && near_table->value.type == ALICE_DTABLE_NUMBER) {
				camera->near = near_table->value.as.number;
			}

			alice_DTable* far_table = alice_dtable_find_child(table, "far");
			if (far_table && far_table->value.type == ALICE_DTABLE_NUMBER) {
				camera->far = far_table->value.as.number;
			}

			alice_DTable* exposure_table = alice_dtable_find_child(table, "exposure");
			if (exposure_table && exposure_table->value.type == ALICE_DTABLE_NUMBER) {
				camera->exposure = exposure_table->value.as.number;
			}

			alice_DTable* gamma_table = alice_dtable_find_child(table, "gamma");
			if (gamma_table && gamma_table->value.type == ALICE_DTABLE_NUMBER) {
				camera->gamma = gamma_table->value.as.number;
			}

			alice_DTable* active_table = alice_dtable_find_child(table, "active");
			if (active_table && active_table->value.type == ALICE_DTABLE_BOOL) {
				camera->active = active_table->value.as.number;
			}

			break;
		}
		case ALICE_ST_POINTLIGHT: {
			alice_PointLight* light = (alice_PointLight*)entity;

			alice_DTable* range_table = alice_dtable_find_child(table, "range");
			if (range_table && range_table->value.type == ALICE_DTABLE_NUMBER) {
				light->range = range_table->value.as.number;
			}

			alice_DTable* intensity_table = alice_dtable_find_child(table, "intensity");
			if (intensity_table && intensity_table->value.type == ALICE_DTABLE_NUMBER) {
				light->intensity = intensity_table->value.as.number;
			}


			alice_DTable* color_table = alice_dtable_find_child(table, "color");
			if (color_table) {
				alice_RGBColor color = (alice_RGBColor){1.0, 1.0, 1.0};

				alice_DTable* r_table = alice_dtable_find_child(color_table, "r");
				if (r_table && r_table->value.type == ALICE_DTABLE_NUMBER) {
					color.r = r_table->value.as.number;
				}

				alice_DTable* g_table = alice_dtable_find_child(color_table, "g");
				if (g_table && g_table->value.type == ALICE_DTABLE_NUMBER) {
					color.g = g_table->value.as.number;
				}

				alice_DTable* b_table = alice_dtable_find_child(color_table, "b");
				if (b_table && b_table->value.type == ALICE_DTABLE_NUMBER) {
					color.b = b_table->value.as.number;
				}

				light->color = alice_color_from_rgb_color(color);
			}

			break;
		}
		case ALICE_ST_DIRECTIONALLIGHT: {
			alice_DirectionalLight* light = (alice_DirectionalLight*)entity;

			alice_DTable* intensity_table = alice_dtable_find_child(table, "intensity");
			if (intensity_table && intensity_table->value.type == ALICE_DTABLE_NUMBER) {
				light->intensity = intensity_table->value.as.number;
			}

			alice_DTable* color_table = alice_dtable_find_child(table, "color");
			if (color_table) {
				alice_RGBColor color = (alice_RGBColor){1.0, 1.0, 1.0};

				alice_DTable* r_table = alice_dtable_find_child(color_table, "r");
				if (r_table && r_table->value.type == ALICE_DTABLE_NUMBER) {
					color.r = r_table->value.as.number;
				}

				alice_DTable* g_table = alice_dtable_find_child(color_table, "g");
				if (g_table && g_table->value.type == ALICE_DTABLE_NUMBER) {
					color.g = g_table->value.as.number;
				}

				alice_DTable* b_table = alice_dtable_find_child(color_table, "b");
				if (b_table && b_table->value.type == ALICE_DTABLE_NUMBER) {
					color.b = b_table->value.as.number;
				}

				light->color = alice_color_from_rgb_color(color);
			}

			break;
		}
		case ALICE_ST_RIGIDBODY3D: {
			alice_Rigidbody3D* rigidbody = (alice_Rigidbody3D*)entity;

			alice_DTable* mass_table = alice_dtable_find_child(table, "mass");
			if (mass_table && mass_table->value.type == ALICE_DTABLE_NUMBER) {
				rigidbody->mass = mass_table->value.as.number;
			}

			alice_DTable* restitution_table = alice_dtable_find_child(table, "restitution");
			if (restitution_table && restitution_table->value.type == ALICE_DTABLE_NUMBER) {
				rigidbody->restitution = restitution_table->value.as.number;
			}

			alice_DTable* gravity_scale_table = alice_dtable_find_child(table, "gravity_scale");
			if (gravity_scale_table && gravity_scale_table->value.type == ALICE_DTABLE_NUMBER) {
				rigidbody->gravity_scale = gravity_scale_table->value.as.number;
			}

			alice_DTable* dynamic_friction_table = alice_dtable_find_child(table, "dynamic_friction");
			if (dynamic_friction_table && dynamic_friction_table->value.type == ALICE_DTABLE_NUMBER) {
				rigidbody->dynamic_friction = dynamic_friction_table->value.as.number;
			}

			alice_DTable* static_friction_table = alice_dtable_find_child(table, "static_friction");
			if (static_friction_table && dynamic_friction_table->value.type == ALICE_DTABLE_NUMBER) {
				rigidbody->static_friction = static_friction_table->value.as.number;
			}

			alice_DTable* box_table = alice_dtable_find_child(table, "box");
			if (box_table) {
				alice_DTable* dimentions_table = alice_dtable_find_child(box_table, "dimentions");
				if (dimentions_table) {
					alice_DTable* x_table = alice_dtable_find_child(dimentions_table, "x");
					if (x_table && x_table->value.type == ALICE_DTABLE_NUMBER) {
						rigidbody->box.dimentions.x = x_table->value.as.number;
					}

					alice_DTable* y_table = alice_dtable_find_child(dimentions_table, "y");
					if (y_table && y_table->value.type == ALICE_DTABLE_NUMBER) {
						rigidbody->box.dimentions.y = y_table->value.as.number;
					}

					alice_DTable* z_table = alice_dtable_find_child(dimentions_table, "z");
					if (z_table && z_table->value.type == ALICE_DTABLE_NUMBER) {
						rigidbody->box.dimentions.z = z_table->value.as.number;
					}
				}

				alice_DTable* position_table = alice_dtable_find_child(box_table, "position");
				if (position_table) {
					alice_DTable* x_table = alice_dtable_find_child(position_table, "x");
					if (x_table && x_table->value.type == ALICE_DTABLE_NUMBER) {
						rigidbody->box.position.x = x_table->value.as.number;
					}

					alice_DTable* y_table = alice_dtable_find_child(position_table, "y");
					if (y_table && y_table->value.type == ALICE_DTABLE_NUMBER) {
						rigidbody->box.position.y = y_table->value.as.number;
					}

					alice_DTable* z_table = alice_dtable_find_child(position_table, "z");
					if (z_table && z_table->value.type == ALICE_DTABLE_NUMBER) {
						rigidbody->box.position.z = z_table->value.as.number;
					}
				}
			}

			break;
		}
		default:
			break;
	}

	alice_DTable* children_table = alice_dtable_find_child(table, "children");
	if (children_table) {
		for (u32 i = 0; i < children_table->child_count; i++) {
			alice_DTable* child_table = &children_table->children[i];
			alice_EntityHandle child = alice_deserialise_entity(child_table, scene);
			alice_entity_parent_to(scene, child, handle);
		}
	}

	return handle;
}

void alice_deserialise_scene(alice_Scene* scene, const char* file_path) {
	assert(scene);

	alice_DTable* table = alice_read_dtable(alice_load_string(file_path));
	if (!table) { return; }

	alice_DTable* settings_table = alice_dtable_find_child(table, "settings");
	if (settings_table) {
		alice_Shader* postprocess = alice_null;
		alice_Shader* blur = alice_null;
		alice_Shader* extract = alice_null;
		alice_Shader* debug_shader = alice_null;
		alice_Shader* depth_shader = alice_null;
		bool debug = false;

		alice_DTable* postprocess_shader_table = alice_dtable_find_child(settings_table,
				"postprocess_shader");
		if (postprocess_shader_table && postprocess_shader_table->value.type == ALICE_DTABLE_STRING) {
			postprocess = alice_load_shader(postprocess_shader_table->value.as.string);
		}

		alice_DTable* bright_extract_shader_table = alice_dtable_find_child(settings_table,
				"bright_extract_shader");
		if (bright_extract_shader_table &&
				bright_extract_shader_table->value.type == ALICE_DTABLE_STRING) {
			extract = alice_load_shader(bright_extract_shader_table->value.as.string);
		}

		alice_DTable* blur_shader_table = alice_dtable_find_child(settings_table,
				"blur_shader");
		if (blur_shader_table && blur_shader_table->value.type == ALICE_DTABLE_STRING) {
			blur = alice_load_shader(blur_shader_table->value.as.string);
		}

		alice_DTable* debug_shader_table = alice_dtable_find_child(settings_table,
				"line_shader");
		if (debug_shader_table && debug_shader_table->value.type == ALICE_DTABLE_STRING) {
			debug_shader = alice_load_shader(debug_shader_table->value.as.string);
		}

		alice_DTable* depth_shader_table = alice_dtable_find_child(settings_table,
				"depth_shader");
		if (depth_shader_table && depth_shader_table->value.type == ALICE_DTABLE_STRING) {
			depth_shader = alice_load_shader(depth_shader_table->value.as.string);
		}

		alice_DTable* debug_table = alice_dtable_find_child(settings_table, "debug");
		if (debug_table && debug_table->value.type == ALICE_DTABLE_BOOL) {
			debug = debug_table->value.as.boolean;
		}

		scene->renderer = alice_new_scene_renderer_3d(postprocess, extract, blur, depth_shader,
				debug, debug_shader);
		scene->renderer->use_antialiasing = true;
		scene->renderer->use_bloom = true;

		alice_DTable* use_bloom_table = alice_dtable_find_child(settings_table,
				"use_bloom");
		if (use_bloom_table && use_bloom_table->value.type == ALICE_DTABLE_BOOL) {
			scene->renderer->use_bloom = use_bloom_table->value.as.boolean;
		}

		alice_DTable* bloom_threshold_table = alice_dtable_find_child(settings_table, "bloom_threshold");
		if (bloom_threshold_table && bloom_threshold_table->value.type == ALICE_DTABLE_NUMBER) {
			scene->renderer->bloom_threshold = bloom_threshold_table->value.as.number;
		}

		alice_DTable* bloom_blur_iterations_table = alice_dtable_find_child(settings_table,
				"bloom_blur_iterations");
		if (bloom_blur_iterations_table &&
				bloom_blur_iterations_table->value.type == ALICE_DTABLE_NUMBER) {
			scene->renderer->bloom_blur_iterations =
				(u32)bloom_blur_iterations_table->value.as.number;
		}

		alice_DTable* use_antialiasing_table = alice_dtable_find_child(settings_table,
				"use_antialiasing");
		if (use_antialiasing_table && use_antialiasing_table->value.type == ALICE_DTABLE_BOOL) {
			scene->renderer->use_antialiasing = use_antialiasing_table->value.as.boolean;
		}

		scene->physics_engine = alice_new_physics_engine(scene);

		alice_DTable* gravity_table = alice_dtable_find_child(settings_table, "gravity");
		if (gravity_table && gravity_table->value.type == ALICE_DTABLE_NUMBER) {
			scene->physics_engine->gravity = gravity_table->value.as.number;
		}
	}

	alice_DTable* entities_table = alice_dtable_find_child(table, "entities");
	for (u32 i = 0; i < entities_table->child_count; i++) {
		alice_DTable* child_table = &entities_table->children[i];
		alice_deserialise_entity(child_table, scene);
	}

	alice_free_dtable(table);
}

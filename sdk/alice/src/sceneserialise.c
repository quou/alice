#include "alice/sceneserialise.h"
#include "alice/dtable.h"

static void alice_serialise_entity(alice_DTable* table, alice_Scene* scene, alice_Entity* entity) {
	assert(table && scene && entity);

	if (entity->name) {
		alice_DTable name_table = alice_new_string_dtable("name", entity->name);
		alice_dtable_add_child(table, name_table);
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

	alice_DTable rotation_table = alice_new_empty_dtable("rotation");
	{
		alice_DTable x_table = alice_new_number_dtable("x", entity->rotation.x);
		alice_DTable y_table = alice_new_number_dtable("y", entity->rotation.y);
		alice_DTable z_table = alice_new_number_dtable("z", entity->rotation.z);

		alice_dtable_add_child(&rotation_table, x_table);
		alice_dtable_add_child(&rotation_table, y_table);
		alice_dtable_add_child(&rotation_table, z_table);
	}

	alice_DTable scale_table = alice_new_empty_dtable("scale");
	{
		alice_DTable x_table = alice_new_number_dtable("x", entity->scale.x);
		alice_DTable y_table = alice_new_number_dtable("y", entity->scale.y);
		alice_DTable z_table = alice_new_number_dtable("z", entity->scale.z);

		alice_dtable_add_child(&scale_table, x_table);
		alice_dtable_add_child(&scale_table, y_table);
		alice_dtable_add_child(&scale_table, z_table);
	}

	alice_dtable_add_child(table, position_table);
}

void alice_serialise_scene(alice_Scene* scene, const char* file_path) {
	alice_DTable table = alice_new_empty_dtable("scene");

	for (alice_scene_iter(scene, iter, alice_Entity)) {
		alice_DTable entity_table = alice_new_empty_dtable("entity");

		alice_serialise_entity(&entity_table, scene, iter.current_ptr);

		alice_dtable_add_child(&table, entity_table);
	}

	alice_write_dtable(&table, file_path);	

	alice_deinit_dtable(&table);
}

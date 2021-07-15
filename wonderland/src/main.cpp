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

struct Editor {
	alice_EntityHandle selection_context;
	alice_Scene* scene;
};

Editor editor;

static void draw_entity_hierarchy(alice_EntityHandle handle, alice_Entity* ptr) {
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;

	if (editor.selection_context == handle) {
		flags |= ImGuiTreeNodeFlags_Selected;
	}

	if (ptr->child_count == 0) {
		flags |= ImGuiTreeNodeFlags_Leaf;
	}

	bool open = ImGui::TreeNodeEx((void*)(uint64_t)handle, flags, "%s",
		ptr->name ? ptr->name : "entity");

	if (ImGui::IsItemClicked()) {
		editor.selection_context = handle;
	}

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
		ImGui::Text("reparent");

		ImGui::SetDragDropPayload("reparent entity", &handle, sizeof(handle));

		ImGui::EndDragDropSource();
	}

	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("reparent entity")) {
			alice_EntityHandle dragged_entity = *static_cast<alice_EntityHandle*>(payload->Data);

			alice_Entity* dragged_ptr = (alice_Entity*)alice_get_entity_ptr(editor.scene, dragged_entity);

			if (dragged_entity != handle) {
				alice_EntityHandle parent = dragged_ptr->parent;
				if (parent != alice_null_entity_handle) {
					alice_entity_remove_child(editor.scene, parent, dragged_entity);
				}
				alice_entity_parent_to(editor.scene, handle, dragged_entity);
			}
		}
	}


	if (open) {
		for (u32 i = 0; i < ptr->child_count; i++) {
			draw_entity_hierarchy(ptr->children[i],
					(alice_Entity*)alice_get_entity_ptr(editor.scene, ptr->children[i]));
		}

		ImGui::TreePop();
	}
}

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

	editor.selection_context = alice_null_entity_handle;

	editor.scene = alice_new_scene();
	alice_deserialise_scene(editor.scene, "scenes/cube.ascn");

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

		if (ImGui::Begin("scene")) {
			float width = ImGui::GetContentRegionAvail().x;
			float height = ImGui::GetContentRegionAvail().y;

			alice_render_scene_3d(renderer, width, height, editor.scene, scene_target);

			ImGui::Image((ImTextureID)(u64)scene_target->color_attachments[0],
				ImVec2(width, height), ImVec2(0.0, 1.0), ImVec2(1.0, 0.0));
		}
		ImGui::End();

		if (ImGui::Begin("hierarchy")) {
			bool open = ImGui::TreeNode("root");

			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("reparent entity")) {
					alice_EntityHandle dragged_entity = *static_cast<alice_EntityHandle*>(payload->Data);

					alice_entity_unparent(editor.scene, dragged_entity);
				}
			}

			if (open) {
				for (u32 i = 0; i < editor.scene->pool_count; i++) {
					alice_EntityPool* pool = &editor.scene->pools[i];
					for (u32 ii = 0; ii < pool->count; ii++) {
						alice_EntityHandle handle = alice_new_entity_handle(ii, pool->type_id);

						alice_Entity* ptr = (alice_Entity*)alice_entity_pool_get(pool, ii);

						if (ptr->parent == alice_null_entity_handle) {
							draw_entity_hierarchy(handle, ptr);
						}
					}
				}
				ImGui::TreePop();
			}

			if (!ImGui::IsAnyItemHovered() &&
				ImGui::IsWindowHovered() &&
				ImGui::IsMouseClicked(ImGuiPopupFlags_MouseButtonLeft)) {
				editor.selection_context = alice_null_entity_handle;
			}
		}
		ImGui::End();

		gui_end_frame();

		alice_update_application();
	}

	alice_free_render_target(scene_target);

	alice_free_scene_renderer_3d(renderer);

	alice_free_scene(editor.scene);

	quit_gui();

	alice_free_application();
	alice_free_resource_manager();
}


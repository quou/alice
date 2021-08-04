#include <assert.h>
#include <stdlib.h>

#include "alice/debugrenderer.h"

alice_debug_renderer_t* alice_new_debug_renderer(alice_shader_t* line_shader) {
	assert(line_shader);

	alice_debug_renderer_t* new = malloc(sizeof(alice_debug_renderer_t));

	new->line_shader = line_shader;

	alice_vertex_buffer_t* buffer = alice_new_vertex_buffer(
			ALICE_VERTEXBUFFER_DRAW_LINES | ALICE_VERTEXBUFFER_DYNAMIC_DRAW);

	new->camera = alice_null;
	new->scene = alice_null;

	new->vertices_per_line = 2;
	new->elements_per_vertex = 3;
	new->indices_per_line = 2;

	alice_bind_vertex_buffer_for_edit(buffer);
	alice_push_vertices(buffer, alice_null,
			new->vertices_per_line * new->elements_per_vertex);
	alice_push_indices(buffer, alice_null, new->indices_per_line);
	alice_configure_vertex_buffer(buffer, 0, 3, 0, 0); /* vec3 position */
	alice_bind_vertex_buffer_for_edit(alice_null);

	new->line_vb = buffer;

	return new;
}

void alice_free_debug_renderer(alice_debug_renderer_t* renderer) {
	assert(renderer);

	alice_free_vertex_buffer(renderer->line_vb);

	free(renderer);
}

void alice_debug_renderer_draw_line(alice_debug_renderer_t* renderer, alice_v3f_t start, alice_v3f_t end) {
	assert(renderer);

	float verts[] = {
		start.x, start.y, start.z,
		end.x, end.y, end.z
	};

	u32 indices[] = {
		0, 1
	};

	alice_bind_vertex_buffer_for_edit(renderer->line_vb);
	alice_update_vertices(renderer->line_vb, verts, 0,
		renderer->vertices_per_line * renderer->elements_per_vertex);
	alice_update_indices(renderer->line_vb, indices, 0,
			renderer->indices_per_line);

	alice_bind_shader(renderer->line_shader);

	if (renderer->scene && renderer->camera) {
		alice_shader_set_m4f(renderer->line_shader, "camera",
				alice_get_camera_3d_matrix(renderer->scene, renderer->camera));
	}

	alice_bind_vertex_buffer_for_draw(renderer->line_vb);
	alice_draw_vertex_buffer(renderer->line_vb);
	alice_bind_vertex_buffer_for_draw(alice_null);

	alice_bind_shader(alice_null);
}

void alice_debug_renderer_draw_aabb(alice_debug_renderer_t* renderer, alice_aabb_t aabb) {
	assert(renderer);

	alice_debug_renderer_draw_line(renderer,
			(alice_v3f_t) { aabb.min.x, aabb.min.y, aabb.min.z },
			(alice_v3f_t) { aabb.min.x, aabb.max.y, aabb.min.z });

	alice_debug_renderer_draw_line(renderer,
			(alice_v3f_t) { aabb.max.x, aabb.min.y, aabb.min.z },
			(alice_v3f_t) { aabb.max.x, aabb.max.y, aabb.min.z });

	alice_debug_renderer_draw_line(renderer,
			(alice_v3f_t) { aabb.min.x, aabb.min.y, aabb.max.z },
			(alice_v3f_t) { aabb.min.x, aabb.max.y, aabb.max.z });

	alice_debug_renderer_draw_line(renderer,
			(alice_v3f_t) { aabb.max.x, aabb.min.y, aabb.max.z },
			(alice_v3f_t) { aabb.max.x, aabb.max.y, aabb.max.z });

	alice_debug_renderer_draw_line(renderer,
			(alice_v3f_t) { aabb.max.x, aabb.min.y, aabb.min.z },
			(alice_v3f_t) { aabb.min.x, aabb.min.y, aabb.min.z });

	alice_debug_renderer_draw_line(renderer,
			(alice_v3f_t) { aabb.min.x, aabb.min.y, aabb.max.z },
			(alice_v3f_t) { aabb.min.x, aabb.min.y, aabb.min.z });

	alice_debug_renderer_draw_line(renderer,
			(alice_v3f_t) { aabb.min.x, aabb.max.y, aabb.max.z },
			(alice_v3f_t) { aabb.max.x, aabb.max.y, aabb.max.z });

	alice_debug_renderer_draw_line(renderer,
			(alice_v3f_t) { aabb.max.x, aabb.max.y, aabb.min.z },
			(alice_v3f_t) { aabb.max.x, aabb.max.y, aabb.max.z });

	alice_debug_renderer_draw_line(renderer,
			(alice_v3f_t) { aabb.min.x, aabb.min.y, aabb.max.z },
			(alice_v3f_t) { aabb.max.x, aabb.min.y, aabb.max.z });

	alice_debug_renderer_draw_line(renderer,
			(alice_v3f_t) { aabb.max.x, aabb.min.y, aabb.min.z },
			(alice_v3f_t) { aabb.max.x, aabb.min.y, aabb.max.z });

	alice_debug_renderer_draw_line(renderer,
			(alice_v3f_t) { aabb.min.x, aabb.max.y, aabb.min.z },
			(alice_v3f_t) { aabb.min.x, aabb.max.y, aabb.max.z });

	alice_debug_renderer_draw_line(renderer,
			(alice_v3f_t) { aabb.min.x, aabb.max.y, aabb.min.z },
			(alice_v3f_t) { aabb.max.x, aabb.max.y, aabb.min.z });
}

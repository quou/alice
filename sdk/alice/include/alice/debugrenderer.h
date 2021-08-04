#pragma once

#include "alice/core.h"
#include "alice/graphics.h"
#include "alice/physics.h"

typedef struct alice_debug_renderer_t {
	alice_shader_t* line_shader;

	alice_vertex_buffer_t* line_vb;

	alice_camera_3d_t* camera;
	alice_scene_t* scene;

	u32 vertices_per_line;
	u32 elements_per_vertex;
	u32 indices_per_line;
} alice_debug_renderer_t;

ALICE_API alice_debug_renderer_t* alice_new_debug_renderer(alice_shader_t* line_shader);
ALICE_API void alice_free_debug_renderer(alice_debug_renderer_t* renderer);

ALICE_API void alice_debug_renderer_draw_line(alice_debug_renderer_t* renderer, alice_v3f_t start, alice_v3f_t end);
ALICE_API void alice_debug_renderer_draw_aabb(alice_debug_renderer_t* renderer, alice_aabb_t aabb);

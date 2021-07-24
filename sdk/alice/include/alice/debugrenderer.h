#pragma once

#include "alice/core.h"
#include "alice/graphics.h"
#include "alice/physics.h"

typedef struct alice_DebugRenderer {
	alice_Shader* line_shader;

	alice_VertexBuffer* line_vb;

	alice_Camera3D* camera;
	alice_Scene* scene;

	u32 vertices_per_line;
	u32 elements_per_vertex;
	u32 indices_per_line;
} alice_DebugRenderer;

ALICE_API alice_DebugRenderer* alice_new_debug_renderer(alice_Shader* line_shader);
ALICE_API void alice_free_debug_renderer(alice_DebugRenderer* renderer);

ALICE_API void alice_debug_renderer_draw_line(alice_DebugRenderer* renderer, alice_v3f start, alice_v3f end);
ALICE_API void alice_debug_renderer_draw_aabb(alice_DebugRenderer* renderer, alice_AABB aabb);

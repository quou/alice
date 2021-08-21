#pragma once

#include "alice/core.h"
#include "alice/resource.h"
#include "alice/graphics.h"

typedef struct mu_Context mu_Context;

typedef struct alice_opengl_state_backup_t {
	bool blend;
	bool cull_face;
	bool depth_test;
	bool scissor_test;
} alice_opengl_state_backup_t;

typedef struct alice_ui_renderer_t {
	alice_opengl_state_backup_t backup;

	u32 quad_count;

	alice_vertex_buffer_t* vb;
	alice_shader_t* shader;
	alice_texture_t* atlas;

	alice_font_t* font;

	alice_m4f_t camera;
	u32 width, height;
} alice_ui_renderer_t;

ALICE_API alice_ui_renderer_t* alice_new_ui_renderer(alice_shader_t* shader, alice_font_t* font);
ALICE_API void alice_free_ui_renderer(alice_ui_renderer_t* renderer);
ALICE_API void alice_ui_renderer_push_quad(alice_ui_renderer_t* renderer, alice_rect_t dst, alice_rect_t src,
		alice_color_t color, float transparency, u32 mode);
ALICE_API void alice_begin_ui_renderer(alice_ui_renderer_t* renderer, u32 width, u32 height);
ALICE_API void alice_end_ui_renderer(alice_ui_renderer_t* renderer);
ALICE_API void alice_flush_ui_renderer(alice_ui_renderer_t* renderer);
ALICE_API void alice_set_ui_renderer_clip(alice_ui_renderer_t* renderer, alice_rect_t rect);
ALICE_API void alice_ui_renderer_draw_text(alice_ui_renderer_t* renderer, const char* text,
		alice_v2f_t position, alice_color_t color, float transparency);
ALICE_API void alice_ui_renderer_draw_rect(alice_ui_renderer_t* renderer, alice_rect_t rect,
		alice_color_t color, float transparency);
ALICE_API void alice_ui_renderer_draw_icon(alice_ui_renderer_t* renderer, u32 id, alice_rect_t rect,
		alice_color_t color, float transparency);
ALICE_API float alice_ui_text_width(alice_ui_renderer_t* rendere, const char* text);
ALICE_API float alice_ui_tect_height(alice_ui_renderer_t* renderer);

ALICE_API void alice_init_microui_renderer(alice_shader_t* shader, alice_font_t* font);
ALICE_API void alice_deinit_microui_renderer();
ALICE_API i32 alice_microui_text_width(mu_Font font, const char* test, i32 len);
ALICE_API i32 alice_microui_text_height(mu_Font font);
ALICE_API void alice_update_microui(mu_Context* context);
ALICE_API void alice_render_microui(mu_Context* context, u32 width, u32 height);

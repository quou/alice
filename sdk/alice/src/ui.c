#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <stb_truetype.h>
#include <glad/glad.h>
#include <microui.h>
#include <atlas.h>

#include "alice/ui.h"
#include "alice/application.h"
#include "alice/input.h"

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

	alice_m4f_t camera;
	u32 width, height;
} alice_ui_renderer_t;

static alice_ui_renderer_t renderer;

void alice_init_microui_renderer(alice_shader_t* shader) {
	renderer.shader = shader;
	renderer.quad_count = 0;

	renderer.atlas = alice_new_texture_from_memory_uncompressed(
				atlas_texture, sizeof(atlas_texture), 
				ATLAS_WIDTH, ATLAS_HEIGHT, 1,
				ALICE_TEXTURE_ANTIALIASED);

	alice_vertex_buffer_t* buffer = alice_new_vertex_buffer(
			ALICE_VERTEXBUFFER_DRAW_TRIANGLES | ALICE_VERTEXBUFFER_DYNAMIC_DRAW);

	alice_bind_vertex_buffer_for_edit(buffer);
	alice_push_vertices(buffer, alice_null, (9 * 4) * 10000);
	alice_push_indices(buffer, alice_null, 6 * 10000);
	alice_configure_vertex_buffer(buffer, 0, 2, 9, 0); /* vec2 position */
	alice_configure_vertex_buffer(buffer, 1, 2, 9, 2); /* vec2 uv */
	alice_configure_vertex_buffer(buffer, 2, 4, 9, 4); /* vec4 color */
	alice_configure_vertex_buffer(buffer, 3, 1, 9, 8); /* float use_texture */
	alice_bind_vertex_buffer_for_edit(alice_null);

	renderer.vb = buffer;
}

void alice_deinit_microui_renderer() {
	alice_free_vertex_buffer(renderer.vb);
	alice_free_texture(renderer.atlas);
}

static void alice_microui_renderer_push_quad(mu_Rect dst, mu_Rect src, mu_Color color, bool is_texture) {
	alice_bind_vertex_buffer_for_edit(renderer.vb);

	const float tx = src.x / (float)ATLAS_WIDTH;
	const float ty = src.y / (float)ATLAS_HEIGHT;
	const float tw = src.w / (float)ATLAS_WIDTH;
	const float th = src.h / (float)ATLAS_HEIGHT;

	alice_rgb_color_t col;
	col.r = (float)color.r / 255.0f;
	col.g = (float)color.g / 255.0f;
	col.b = (float)color.b / 255.0f;

	float verts[] = {
		dst.x, 		dst.y,		tx, 		ty, col.r, col.g, col.b, (float)color.a / 255.0f, (float)is_texture,
		dst.x + dst.w, 	dst.y,		tx + tw, 	ty, col.r, col.g, col.b, (float)color.a / 255.0f, (float)is_texture,
		dst.x + dst.w, 	dst.y + dst.h,	tx + tw, 	ty + th, col.r, col.g, col.b, (float)color.a / 255.0f, (float)is_texture,
		dst.x, 		dst.y + dst.h,	tx,		ty + th, col.r, col.g, col.b, (float)color.a / 255.0f, (float)is_texture
	};

	const u32 index_offset = renderer.quad_count * 4;

	u32 indices[] = {
		index_offset + 3, index_offset + 2, index_offset + 1,
		index_offset + 3, index_offset + 1, index_offset + 0
	};

	alice_update_vertices(renderer.vb, verts, renderer.quad_count * 9 * 4, 9 * 4);
	alice_update_indices(renderer.vb, indices, renderer.quad_count * 6, 6);

	alice_bind_vertex_buffer_for_edit(alice_null);

	renderer.quad_count++;
}

static void alice_flush_microui_renderer() {
	alice_bind_vertex_buffer_for_edit(alice_null);
	alice_bind_shader(renderer.shader);	

	alice_bind_texture(renderer.atlas, 0);
	alice_shader_set_int(renderer.shader, "atlas", 0);

	alice_shader_set_m4f(renderer.shader, "camera", renderer.camera);

	alice_bind_vertex_buffer_for_draw(renderer.vb);
	alice_draw_vertex_buffer_custom_count(renderer.vb, renderer.quad_count * 6);
	alice_bind_vertex_buffer_for_draw(alice_null);

	alice_bind_texture(alice_null, 0);
	alice_bind_shader(alice_null);

	renderer.quad_count = 0;
}

static void alice_begin_microui_render(u32 width, u32 height) {
	renderer.backup.blend = glIsEnabled(GL_BLEND);
	renderer.backup.cull_face = glIsEnabled(GL_CULL_FACE);
	renderer.backup.depth_test = glIsEnabled(GL_DEPTH_TEST);
	renderer.backup.scissor_test = glIsEnabled(GL_SCISSOR_TEST);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_SCISSOR_TEST);

	renderer.quad_count = 0;

	renderer.width = width;
	renderer.height = height;
	renderer.camera = alice_m4f_ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);

	glScissor(0.0f, 0.0f, (float)width, (float)height);
}

static void alice_end_microui_render() {
	alice_flush_microui_renderer();

	if (renderer.backup.blend) {        glEnable(GL_BLEND); }        else { glDisable(GL_BLEND); }
	if (renderer.backup.cull_face) {    glEnable(GL_CULL_FACE); }    else { glDisable(GL_CULL_FACE); }
	if (renderer.backup.depth_test) {   glEnable(GL_DEPTH_TEST); }   else { glDisable(GL_DEPTH_TEST); }
	if (renderer.backup.scissor_test) { glEnable(GL_SCISSOR_TEST); } else { glDisable(GL_SCISSOR_TEST); }
}

static void alice_set_microui_renderer_clip(mu_Rect rect) {
	alice_flush_microui_renderer();
	glScissor(rect.x, renderer.height - (rect.y + rect.h), rect.w, rect.h);
}

static void alice_render_microui_text(const char* text, mu_Vec2 pos, mu_Color color) {
	mu_Rect dst = { pos.x, pos.y, 0, 0 };
	for (const char* c = text; *c; c++) {
		if ((*c & 0xc0) == 0x80) { continue; }
		i32 chr = mu_min((u8)*c, 127);
		mu_Rect src = atlas[ATLAS_FONT + chr];
		dst.w = src.w;
		dst.h = src.h;
		alice_microui_renderer_push_quad(dst, src, color, true);
		dst.x += dst.w;
	}
}

static void alice_render_microui_rect(mu_Rect rect, mu_Color color) {
	alice_microui_renderer_push_quad(rect, atlas[ATLAS_WHITE], color, false);
}

static void alice_render_microui_icon(i32 id, mu_Rect rect, mu_Color color) {
	mu_Rect src = atlas[id];
	i32 x = rect.x + (rect.w - src.w) / 2;
	i32 y = rect.y + (rect.h - src.h) / 2;
	alice_microui_renderer_push_quad(mu_rect(x, y, src.w, src.h), src, color, true);
}

i32 alice_microui_text_width(mu_Font font, const char* text, i32 len) {
	i32 result = 0;

	for (const char* c = text; *c && len--; c++) {
		if ((*c & 0xc0) == 0x80) { continue; }
		i32 chr = mu_min((u8)*c, 127);
		result += atlas[ATLAS_FONT + chr].w;
	}

	return result;
}

i32 alice_microui_text_height(mu_Font font) {
	return 18;
}

void alice_update_microui(mu_Context* context) {
	assert(context);

	alice_v2i_t mouse_position = alice_get_mouse_position();

	if (alice_mouse_moved()) {
		mu_input_mousemove(context, mouse_position.x, mouse_position.y);
	}

	if (alice_scrolled()) {
		mu_input_scroll(context, 0, alice_get_scroll_offset().y * -30.0f);
	}

	const char* text_input = alice_get_text_input();
	if (*text_input) {
		mu_input_text(context, text_input);
	}

	/* This is a really bad way of doing this.
	 * TODO: figure out a better way. */
	if (alice_mouse_button_just_pressed(ALICE_MOUSE_BUTTON_LEFT)) {
		mu_input_mousedown(context, mouse_position.x, mouse_position.y, MU_MOUSE_LEFT);
	} else if (alice_mouse_button_just_released(ALICE_MOUSE_BUTTON_LEFT)) {
		mu_input_mouseup(context, mouse_position.x, mouse_position.y, MU_MOUSE_LEFT);
	}

	if (alice_mouse_button_just_pressed(ALICE_MOUSE_BUTTON_RIGHT)) {
		mu_input_mousedown(context, mouse_position.x, mouse_position.y, MU_MOUSE_RIGHT);
	} else if (alice_mouse_button_just_released(ALICE_MOUSE_BUTTON_RIGHT)) {
		mu_input_mouseup(context, mouse_position.x, mouse_position.y, MU_MOUSE_RIGHT);
	}

	if (alice_mouse_button_just_pressed(ALICE_MOUSE_BUTTON_MIDDLE)) {
		mu_input_mousedown(context, mouse_position.x, mouse_position.y, MU_MOUSE_MIDDLE);
	} else if (alice_mouse_button_just_released(ALICE_MOUSE_BUTTON_MIDDLE)) {
		mu_input_mouseup(context, mouse_position.x, mouse_position.y, MU_MOUSE_MIDDLE);
	}

	if (alice_key_just_pressed(ALICE_KEY_LEFT_CONTROL) ||
			alice_key_just_pressed(ALICE_KEY_RIGHT_CONTROL)) {
		mu_input_keydown(context, MU_KEY_CTRL);
	} else if (alice_key_just_released(ALICE_KEY_LEFT_CONTROL) ||
			alice_key_just_released(ALICE_KEY_RIGHT_CONTROL)) {
		mu_input_keyup(context, MU_KEY_CTRL);
	}

	if (alice_key_just_pressed(ALICE_KEY_LEFT_SHIFT) ||
			alice_key_just_pressed(ALICE_KEY_RIGHT_SHIFT)) {
		mu_input_keydown(context, MU_KEY_SHIFT);
	} else if (alice_key_just_released(ALICE_KEY_LEFT_SHIFT) ||
			alice_key_just_released(ALICE_KEY_RIGHT_SHIFT)) {
		mu_input_keyup(context, MU_KEY_SHIFT);
	}

	if (alice_key_just_pressed(ALICE_KEY_LEFT_ALT) ||
			alice_key_just_pressed(ALICE_KEY_RIGHT_ALT)) {
		mu_input_keydown(context, MU_KEY_ALT);
	} else if (alice_key_just_released(ALICE_KEY_LEFT_ALT) ||
			alice_key_just_released(ALICE_KEY_RIGHT_ALT)) {
		mu_input_keyup(context, MU_KEY_ALT);
	}

	if (alice_key_just_pressed(ALICE_KEY_ENTER)) {
		mu_input_keydown(context, MU_KEY_RETURN);
	} else if (alice_key_just_released(ALICE_KEY_ENTER)) {
		mu_input_keyup(context, MU_KEY_RETURN);
	}

	if (alice_key_just_pressed(ALICE_KEY_BACKSPACE)) {
		mu_input_keydown(context, MU_KEY_BACKSPACE);
	} else if (alice_key_just_released(ALICE_KEY_BACKSPACE)) {
		mu_input_keyup(context, MU_KEY_BACKSPACE);
	}
}

void alice_render_microui(mu_Context* context, u32 width, u32 height) {
	assert(context);

	alice_begin_microui_render(width, height);
	
	mu_Command* cmd = alice_null;
	while (mu_next_command(context, &cmd)) {
		switch (cmd->type) {
		case MU_COMMAND_TEXT:
			alice_render_microui_text(cmd->text.str, cmd->text.pos, cmd->text.color); break;
		case MU_COMMAND_RECT:
			alice_render_microui_rect(cmd->rect.rect, cmd->rect.color); break;
		case MU_COMMAND_ICON:
			alice_render_microui_icon(cmd->icon.id, cmd->icon.rect, cmd->icon.color); break;
		case MU_COMMAND_CLIP:
			alice_set_microui_renderer_clip(cmd->clip.rect); break;
		}
	}

	alice_end_microui_render();
}

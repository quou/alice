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

const u32 ui_renderer_max_quads = 800;

static alice_ui_renderer_t* mu_renderer;

alice_ui_renderer_t* alice_new_ui_renderer(alice_shader_t* shader, alice_font_t* font) {
	alice_ui_renderer_t* renderer = malloc(sizeof(alice_ui_renderer_t));

	renderer->shader = shader;
	renderer->quad_count = 0;

	alice_vertex_buffer_t* buffer = alice_new_vertex_buffer(
			ALICE_VERTEXBUFFER_DRAW_TRIANGLES | ALICE_VERTEXBUFFER_DYNAMIC_DRAW);

	renderer->font = font;

	alice_bind_vertex_buffer_for_edit(buffer);
	alice_push_vertices(buffer, alice_null, (9 * 4) * ui_renderer_max_quads);
	alice_push_indices(buffer, alice_null, 6 * ui_renderer_max_quads);
	alice_configure_vertex_buffer(buffer, 0, 2, 9, 0); /* vec2 position */
	alice_configure_vertex_buffer(buffer, 1, 2, 9, 2); /* vec2 uv */
	alice_configure_vertex_buffer(buffer, 2, 4, 9, 4); /* vec4 color */
	/* Layout 3 specifies a mode for the rendering.
	 * 	0 = rectangle
	 * 	1 = icon (using atlas)
	 * 	2 = text (using supplied font) */
	alice_configure_vertex_buffer(buffer, 3, 1, 9, 8); /* float mode */
	alice_bind_vertex_buffer_for_edit(alice_null);

	renderer->vb = buffer;
}

void alice_free_ui_renderer(alice_ui_renderer_t* renderer) {
	alice_free_vertex_buffer(renderer->vb);

	free(renderer);
}

void alice_ui_renderer_push_quad(alice_ui_renderer_t* renderer, alice_rect_t dst, alice_rect_t src, 
		alice_color_t color, float transparency, u32 mode) {
	alice_bind_vertex_buffer_for_edit(renderer->vb);

	float tx = src.x;
	float ty = src.y;
	float tw = src.w;
	float th = src.h;

	if (mode == 1) {
		tx /= (float)ATLAS_WIDTH;
		ty /= (float)ATLAS_HEIGHT;
		tw /= (float)ATLAS_WIDTH;
		th /= (float)ATLAS_HEIGHT;
	}

	alice_rgb_color_t col = alice_rgb_color_from_color(color);

	float verts[] = {
		dst.x, 		dst.y,		tx, 		ty, 	col.r, col.g, col.b, transparency, (float)mode,
		dst.x + dst.w, 	dst.y,		tx + tw, 	ty, 	col.r, col.g, col.b, transparency, (float)mode,
		dst.x + dst.w, 	dst.y + dst.h,	tx + tw, 	ty + th, col.r, col.g, col.b, transparency, (float)mode,
		dst.x, 		dst.y + dst.h,	tx,		ty + th, col.r, col.g, col.b, transparency, (float)mode
	};

	const u32 index_offset = renderer->quad_count * 4;

	u32 indices[] = {
		index_offset + 3, index_offset + 2, index_offset + 1,
		index_offset + 3, index_offset + 1, index_offset + 0
	};

	alice_update_vertices(renderer->vb, verts, renderer->quad_count * 9 * 4, 9 * 4);
	alice_update_indices(renderer->vb, indices, renderer->quad_count * 6, 6);

	alice_bind_vertex_buffer_for_edit(alice_null);

	renderer->quad_count++;

	if (renderer->quad_count >= ui_renderer_max_quads) {
		alice_flush_ui_renderer(renderer);
	}
}

void alice_begin_ui_renderer(alice_ui_renderer_t* renderer, u32 width, u32 height) {
	renderer->backup.blend = glIsEnabled(GL_BLEND);
	renderer->backup.cull_face = glIsEnabled(GL_CULL_FACE);
	renderer->backup.depth_test = glIsEnabled(GL_DEPTH_TEST);
	renderer->backup.scissor_test = glIsEnabled(GL_SCISSOR_TEST);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_SCISSOR_TEST);

	renderer->quad_count = 0;

	renderer->width = width;
	renderer->height = height;
	renderer->camera = alice_m4f_ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);

	glScissor(0.0f, 0.0f, (float)width, (float)height);
}

void alice_end_ui_renderer(alice_ui_renderer_t* renderer) {
	alice_flush_ui_renderer(renderer);

	if (renderer->backup.blend) {        glEnable(GL_BLEND); }        else { glDisable(GL_BLEND); }
	if (renderer->backup.cull_face) {    glEnable(GL_CULL_FACE); }    else { glDisable(GL_CULL_FACE); }
	if (renderer->backup.depth_test) {   glEnable(GL_DEPTH_TEST); }   else { glDisable(GL_DEPTH_TEST); }
	if (renderer->backup.scissor_test) { glEnable(GL_SCISSOR_TEST); } else { glDisable(GL_SCISSOR_TEST); }
}

void alice_flush_ui_renderer(alice_ui_renderer_t* renderer) {
	alice_bind_vertex_buffer_for_edit(alice_null);
	alice_bind_shader(renderer->shader);	

	alice_bind_texture(renderer->icon_texture, 0);
	alice_shader_set_int(renderer->shader, "atlas", 0);

	alice_bind_texture(renderer->font->bitmap, 1);
	alice_shader_set_int(renderer->shader, "font", 1);

	alice_shader_set_m4f(renderer->shader, "camera", renderer->camera);

	alice_bind_vertex_buffer_for_draw(renderer->vb);
	alice_draw_vertex_buffer_custom_count(renderer->vb, renderer->quad_count * 6);
	alice_bind_vertex_buffer_for_draw(alice_null);

	alice_bind_texture(alice_null, 0);
	alice_bind_shader(alice_null);

	renderer->quad_count = 0;
}

void alice_ui_renderer_draw_text(alice_ui_renderer_t* renderer, const char* text,
		alice_v2f_t position, alice_color_t color, float transparency) {
	alice_rect_t dst = { position.x, position.y, 0, 0 };

	float xpos = position.x, ypos = position.y + 12;

	for (const char* c = text; *c; c++) {
		stbtt_aligned_quad quad;
		stbtt_GetPackedQuad((stbtt_packedchar*)renderer->font->char_data,
				renderer->font->bitmap->width, renderer->font->bitmap->height,
				*c - 32, &xpos, &ypos, &quad, 0);

		alice_rect_t src = { 
			quad.s0, quad.t0,
			quad.s1 - quad.s0, quad.t1 - quad.t0
		};

		alice_rect_t dst = {
			quad.x0, quad.y0,
			quad.x1 - quad.x0, quad.y1 - quad.y0
		};

		alice_ui_renderer_push_quad(renderer, dst, src, color, transparency, 2);
	}
}

void alice_ui_renderer_draw_rect(alice_ui_renderer_t* renderer, alice_rect_t rect,
		alice_color_t color, float transparency) {
	alice_rect_t dst = {
		rect.x, rect.y, rect.w, rect.h
	};

	mu_Rect mu_src = atlas[ATLAS_WHITE];

	alice_rect_t src = {
		mu_src.x, mu_src.y, mu_src.w, mu_src.h
	};

	alice_ui_renderer_push_quad(renderer, dst, src, color, transparency, 0);
}

void alice_ui_renderer_draw_icon(alice_ui_renderer_t* renderer, u32 id, alice_rect_t rect,
		alice_color_t color, float transparency) {
	alice_rect_t src = renderer->icon_rects[id];

	i32 x = rect.x + (rect.w - src.w) / 2;
	i32 y = rect.y + (rect.h - src.h) / 2;

	alice_ui_renderer_push_quad(renderer, (alice_rect_t) {x, y, src.w, src.h}, src, color, transparency, 1);
}

float alice_ui_text_width(alice_ui_renderer_t* renderer, const char* text) {
	float result = 0.0f;

	float xpos = 0.0f, ypos = 0.0f;
	for (const char* c = text; *c; c++) {
		stbtt_aligned_quad quad;
		stbtt_GetPackedQuad((stbtt_packedchar*)renderer->font->char_data,
				renderer->font->bitmap->width, renderer->font->bitmap->height,
				*c - 32, &xpos, &ypos, &quad, 0);

		result += quad.x1 - quad.x0;

		const char* next = c + 1;
		if (*next != '\0') {
			stbtt_aligned_quad next_quad;
			stbtt_GetPackedQuad((stbtt_packedchar*)renderer->font->char_data,
					renderer->font->bitmap->width, renderer->font->bitmap->height,
					*c - 32, &xpos, &ypos, &next_quad, 0);
			result += next_quad.x0 - quad.x1;
		}
	}

	return result;
}

float alice_ui_tect_height(alice_ui_renderer_t* renderer) {
	return renderer->font->size;
}

void alice_set_ui_renderer_clip(alice_ui_renderer_t* renderer, alice_rect_t rect) {
	alice_flush_ui_renderer(renderer);
	glScissor(rect.x, renderer->height - (rect.y + rect.h), rect.w, rect.h);
}

void alice_init_microui_renderer(alice_shader_t* shader, alice_font_t* font) {
	mu_renderer = alice_new_ui_renderer(shader, font);

	mu_renderer->icon_texture = alice_new_texture_from_memory_uncompressed(
				atlas_texture, sizeof(atlas_texture), 
				ATLAS_WIDTH, ATLAS_HEIGHT, 1,
				ALICE_TEXTURE_ANTIALIASED);

	for (u32 i = MU_ICON_CLOSE; i <= MU_ICON_MAX; i++) {
		mu_Rect rect = atlas[i];
		
		mu_renderer->icon_rects[i] = (alice_rect_t) {rect.x, rect.y, rect.w, rect.h };
	}
}

void alice_deinit_microui_renderer() {
	alice_free_texture(mu_renderer->icon_texture);

	alice_free_ui_renderer(mu_renderer);
}

static void alice_microui_renderer_push_quad(alice_rect_t dst, alice_rect_t src, mu_Color color, u32 mode) {
	alice_rgb_color_t rgb = { (float)color.r / 255.0f, (float)color.g / 255.0f, (float)color.b / 255.0f };

	alice_color_t c = alice_color_from_rgb_color(rgb);

	alice_ui_renderer_push_quad(mu_renderer, dst, src, c, (float)color.a / 255.0f, mode);
}

static void alice_flush_microui_renderer() {
	alice_flush_ui_renderer(mu_renderer);
}

static void alice_begin_microui_render(u32 width, u32 height) {
	alice_begin_ui_renderer(mu_renderer, width, height);
}

static void alice_end_microui_render() {
	alice_end_ui_renderer(mu_renderer);
}

static void alice_set_microui_renderer_clip(mu_Rect rect) {
	alice_rect_t r = { rect.x, rect.y, rect.w, rect.h };

	alice_set_ui_renderer_clip(mu_renderer, r);
}

static void alice_render_microui_text(const char* text, mu_Vec2 pos, mu_Color color) {
	alice_v2f_t p = { pos.x, pos.y };

	alice_rgb_color_t rgb = { (float)color.r / 255.0f, (float)color.g / 255.0f, (float)color.b / 255.0f };

	alice_color_t c = alice_color_from_rgb_color(rgb);

	alice_ui_renderer_draw_text(mu_renderer, text, p, c, (float)color.a / 255.0f);
}

static void alice_render_microui_rect(mu_Rect rect, mu_Color color) {
	alice_rect_t r = { rect.x, rect.y, rect.w, rect.h };

	alice_rgb_color_t rgb = { (float)color.r / 255.0f, (float)color.g / 255.0f, (float)color.b / 255.0f };

	alice_color_t c = alice_color_from_rgb_color(rgb);

	alice_ui_renderer_draw_rect(mu_renderer, r, c, (float)color.a / 255.0f);
}

static void alice_render_microui_icon(i32 id, mu_Rect rect, mu_Color color) {
	alice_rect_t r = { rect.x, rect.y, rect.w, rect.h };

	alice_rgb_color_t rgb = { (float)color.r / 255.0f, (float)color.g / 255.0f, (float)color.b / 255.0f };

	alice_color_t c = alice_color_from_rgb_color(rgb);

	alice_ui_renderer_draw_icon(mu_renderer, id, r, c, (float)color.a / 255.0f);
}

i32 alice_microui_text_width(mu_Font font, const char* text, i32 len) {
	return (i32)alice_ui_text_width(mu_renderer, text);
}

i32 alice_microui_text_height(mu_Font font) {
	return (i32)alice_ui_tect_height(mu_renderer); 
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

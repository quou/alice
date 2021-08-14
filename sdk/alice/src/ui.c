#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <stb_truetype.h>
#include <glad/glad.h>

#include "alice/ui.h"
#include "alice/application.h"
#include "alice/input.h"

struct alice_text_renderer_t {
	alice_vertex_buffer_t* vb;
	alice_texture_t* font_bitmap;
	alice_shader_t* shader;

	float font_size;

	stbtt_packedchar char_data[96];
	stbtt_fontinfo font_info;

	alice_color_t color;

	alice_v2f_t dimentions;
};

static const u32 alice_font_bitmap_size = 512;

alice_text_renderer_t* alice_new_text_renderer(alice_resource_t* font_data,
		float font_size, alice_shader_t* font_shader) {
	alice_text_renderer_t* new = malloc(sizeof(alice_text_renderer_t));

	u8* ttf_bitmap = malloc(alice_font_bitmap_size * alice_font_bitmap_size);

	stbtt_pack_context ctx;
	stbtt_PackBegin(&ctx, ttf_bitmap, alice_font_bitmap_size, alice_font_bitmap_size, 0, 1, alice_null);
	stbtt_PackFontRange(&ctx, font_data->payload, 0, font_size, 32, 96, new->char_data);
	stbtt_PackEnd(&ctx);

	stbtt_InitFont(&new->font_info, font_data->payload, 0);

	new->font_bitmap = alice_new_texture_from_memory_uncompressed(ttf_bitmap,
			alice_font_bitmap_size * alice_font_bitmap_size,
			alice_font_bitmap_size, alice_font_bitmap_size, 1,
			ALICE_TEXTURE_ANTIALIASED);
	new->shader = font_shader;
	new->font_size = font_size;

	new->color = ALICE_COLOR_WHITE;

	free(ttf_bitmap);

	alice_vertex_buffer_t* buffer = alice_new_vertex_buffer(
			ALICE_VERTEXBUFFER_DRAW_TRIANGLES | ALICE_VERTEXBUFFER_DYNAMIC_DRAW);

	alice_bind_vertex_buffer_for_edit(buffer);
	alice_push_vertices(buffer, alice_null, (4 * 4) * 10000);
	alice_push_indices(buffer, alice_null, 6 * 10000);
	alice_configure_vertex_buffer(buffer, 0, 2, 4, 0);
	alice_configure_vertex_buffer(buffer, 1, 2, 4, 2);
	alice_bind_vertex_buffer_for_edit(alice_null);

	new->vb = buffer;

	return new;
}

void alice_free_text_renderer(alice_text_renderer_t* renderer) {
	assert(renderer);

	alice_free_vertex_buffer(renderer->vb);
	alice_free_texture(renderer->font_bitmap);

	free(renderer);
}

void alice_set_text_renderer_dimentions(alice_text_renderer_t* renderer, alice_v2f_t dimentions) {
	assert(renderer);

	renderer->dimentions = dimentions;
}

void alice_render_text(alice_text_renderer_t* renderer, alice_v2f_t position, const char* string) {
	assert(renderer);

	u32 quad_count = 0;

	alice_bind_vertex_buffer_for_edit(renderer->vb);

	float xpos = 0.0, ypos = renderer->font_size;

	while (*string != '\0') {
		if (*string >= 32) {
			stbtt_aligned_quad quad;
			stbtt_GetPackedQuad(renderer->char_data,
					alice_font_bitmap_size, alice_font_bitmap_size,
					*string - 32, &xpos, &ypos, &quad, 0);

			float verts[] = {
				position.x + quad.x0, position.y + quad.y0, quad.s0, quad.t0,
				position.x + quad.x1, position.y + quad.y0, quad.s1, quad.t0,
				position.x + quad.x1, position.y + quad.y1, quad.s1, quad.t1,
				position.x + quad.x0, position.y + quad.y1, quad.s0, quad.t1
			};

			const u32 index_offset = quad_count * 4;

			u32 indices[] = {
				index_offset + 3, index_offset + 2, index_offset + 1,
				index_offset + 3, index_offset + 1, index_offset + 0
			};

			alice_update_vertices(renderer->vb, verts, quad_count * 4 * 4, 4 * 4);
			alice_update_indices(renderer->vb, indices, quad_count * 6, 6);

			quad_count++;
		} else if (*string == '\n') {
			ypos += renderer->font_size;
			xpos = 0.0;
		}

		string++;
	}

	alice_bind_vertex_buffer_for_edit(alice_null);
	alice_bind_shader(renderer->shader);

	alice_m4f_t projection = alice_m4f_ortho(0.0, renderer->dimentions.x,
			renderer->dimentions.y, 0.0, -1.0, 1.0);

	alice_shader_set_color(renderer->shader, "color_mod", renderer->color);
	alice_shader_set_m4f(renderer->shader, "camera", projection);

	alice_bind_texture(renderer->font_bitmap, 0);
	alice_shader_set_int(renderer->shader, "font", 0);

	alice_bind_vertex_buffer_for_draw(renderer->vb);
	alice_draw_vertex_buffer_custom_count(renderer->vb, quad_count * 6);
	alice_bind_vertex_buffer_for_draw(alice_null);

	alice_bind_texture(alice_null, 0);
	alice_bind_shader(alice_null);
}

void alice_set_text_renderer_color(alice_text_renderer_t* renderer, alice_color_t color) {
	assert(renderer);

	renderer->color = color;
}

alice_v2f_t alice_calculate_text_dimentions(alice_text_renderer_t* renderer, const char* text) {
	assert(renderer);

	float width = 0.0f;
	float height = renderer->font_size;

	float temp_width = 0.0f;

	float xpos = 0.0, ypos = renderer->font_size;

	const char* string = text;

	while (*string != '\0') {
		if (*string >= 32) {
			stbtt_aligned_quad quad;
			stbtt_GetPackedQuad(renderer->char_data,
					alice_font_bitmap_size, alice_font_bitmap_size,
					*string - 32, &xpos, &ypos, &quad, 0);

			temp_width += quad.x1 - quad.x0;

			const char* next = string + 1;
			if (*next != '\0') {
				stbtt_aligned_quad next_quad;
				stbtt_GetPackedQuad(renderer->char_data,
						alice_font_bitmap_size, alice_font_bitmap_size,
						*string - 32, &xpos, &ypos, &next_quad, 0);
				temp_width += next_quad.x0 - quad.x1;
			}
		} else if (*string == '\n') {
			height += renderer->font_size;
			if (temp_width > width) {
				width = temp_width;
				temp_width = 0.0f;
			}
		}

		string++;
	}

	if (temp_width > width) {
		width = temp_width;
	}

	return (alice_v2f_t){
		.x = width,
		.y = height
	};
}

bool alice_mouse_over_ui_rect(alice_ui_rect_t rect) {
	alice_v2i_t mouse_pos = alice_get_mouse_position();

	return
			mouse_pos.x > rect.x && mouse_pos.x < rect.x + rect.w &&
			mouse_pos.y > rect.y && mouse_pos.y < rect.y + rect.h;
}

alice_ui_renderer_t* alice_new_ui_renderer(alice_shader_t* rect_shader) {
	assert(rect_shader);

	alice_ui_renderer_t* new = malloc(sizeof(alice_ui_renderer_t));

	new->max_quads = 10000;
	new->quad_count = 0;

	new->shader = rect_shader;
	new->projection = alice_m4f_identity();

	new->vertices_per_quad = 5 * 4;
	new->indices_per_quad = 6;

	alice_vertex_buffer_t* buffer = alice_new_vertex_buffer(
			ALICE_VERTEXBUFFER_DRAW_TRIANGLES | ALICE_VERTEXBUFFER_DYNAMIC_DRAW);

	alice_bind_vertex_buffer_for_edit(buffer);
	alice_push_vertices(buffer, alice_null, new->vertices_per_quad * new->max_quads);
	alice_push_indices(buffer, alice_null, new->indices_per_quad * new->max_quads);
	alice_configure_vertex_buffer(buffer, 0, 2, 5, 0); /* vec2 position */
	alice_configure_vertex_buffer(buffer, 1, 3, 5, 2); /* vec3 color */
	alice_bind_vertex_buffer_for_edit(alice_null);

	new->vb = buffer;

	return new;
}

void alice_free_ui_renderer(alice_ui_renderer_t* renderer) {
	assert(renderer);

	alice_free_vertex_buffer(renderer->vb);

	free(renderer);
}

void alice_set_ui_renderer_dimentions(alice_ui_renderer_t* renderer, alice_v2f_t dimentions) {
	assert(renderer);

	renderer->projection = alice_m4f_ortho(0.0f, dimentions.x, dimentions.y, 0.0f, -1.0f, 1.0f);
}

void alice_ui_renderer_begin_batch(alice_ui_renderer_t* renderer) {
	assert(renderer);

	renderer->quad_count = 0;
}

void alice_ui_renderer_end_batch(alice_ui_renderer_t* renderer) {
	assert(renderer);

	alice_bind_shader(renderer->shader);

	alice_shader_set_m4f(renderer->shader, "camera", renderer->projection);

	alice_bind_vertex_buffer_for_draw(renderer->vb);
	alice_draw_vertex_buffer_custom_count(renderer->vb, renderer->quad_count * 6);
	alice_bind_vertex_buffer_for_draw(alice_null);

	alice_bind_shader(alice_null);
}

void alice_draw_ui_rect(alice_ui_renderer_t* renderer, alice_ui_rect_t rect, alice_color_t color) {
	assert(renderer);

	if (renderer->quad_count >= renderer->max_quads) {
		alice_ui_renderer_end_batch(renderer);
		alice_ui_renderer_begin_batch(renderer);
	}

	alice_rgb_color_t rgb = alice_rgb_color_from_color(color);

	float verts[] = {
		rect.x + rect.w,	rect.y + rect.h,	rgb.r, rgb.g, rgb.b,
		rect.x + rect.w,	rect.y,			rgb.r, rgb.g, rgb.b,
		rect.x,			rect.y,			rgb.r, rgb.g, rgb.b,
		rect.x,			rect.y + rect.h,	rgb.r, rgb.g, rgb.b,
	};

	const u32 index_offset = renderer->quad_count * 4;

	u32 indices[] = {
		index_offset + 0, index_offset + 1, index_offset + 3,
		index_offset + 1, index_offset + 2, index_offset + 3
	};

	alice_bind_vertex_buffer_for_edit(renderer->vb);
	alice_update_vertices(renderer->vb, verts,
			renderer->quad_count * renderer->vertices_per_quad,
			renderer->vertices_per_quad);
	alice_update_indices(renderer->vb, indices,
			renderer->quad_count * renderer->indices_per_quad,
			renderer->indices_per_quad);
	alice_bind_vertex_buffer_for_edit(alice_null);

	renderer->quad_count++;
}

alice_ui_context_t* alice_new_ui_context(alice_shader_t* rect_shader,
		alice_shader_t* gizmo_shader,
		alice_shader_t* text_shader,
		alice_resource_t* font_data, float font_size) {
	alice_ui_context_t* new = malloc(sizeof(alice_ui_context_t));

	new->text_renderer = alice_new_text_renderer(font_data, font_size, text_shader);
	new->renderer = alice_new_ui_renderer(rect_shader);

	alice_apply_default_ui_config(new);

	new->window_count = 0;
	new->window_capacity = 0;
	new->windows = alice_null;

	new->hovered_element = alice_null;
	new->active_input = alice_null;

	new->text_size = font_size;

	for (u32 i = 0; i < ALICE_GIZMOTEXTURE_COUNT; i++) {
		new->gizmo_textures[i] = alice_null;
	}
	new->gizmo_shader = gizmo_shader;

	float vertices[] = {
		 0.5f,  0.5f, 1.0f, 1.0f,
		 0.5f, -0.5f, 1.0f, 0.0f,
 		-0.5f, -0.5f, 0.0f, 0.0f,
		-0.5f,  0.5f, 0.0f, 1.0f
	};
	unsigned int indices[] = {
		0, 1, 3,
		1, 2, 3
	};

	alice_vertex_buffer_t* buffer = alice_new_vertex_buffer(
			ALICE_VERTEXBUFFER_DRAW_TRIANGLES | ALICE_VERTEXBUFFER_STATIC_DRAW);
	alice_bind_vertex_buffer_for_edit(buffer);
	alice_push_vertices(buffer, vertices, sizeof(vertices) / sizeof(float));
	alice_push_indices(buffer, indices, 6);
	alice_configure_vertex_buffer(buffer, 0, 2, 4, 0); /* vec2 position */
	alice_configure_vertex_buffer(buffer, 1, 2, 4, 2); /* vec2 uv */
	alice_bind_vertex_buffer_for_edit(alice_null);

	new->gizmo_quad = buffer;

	return new;
}

void alice_free_ui_context(alice_ui_context_t* context) {
	assert(context);

	for (u32 i = 0; i < context->window_count; i++) {
		alice_deinit_ui_window(&context->windows[i]);
	}

	if (context->window_capacity > 0) {
		free(context->windows);
	}

	alice_free_vertex_buffer(context->gizmo_quad);
	alice_free_text_renderer(context->text_renderer);
	alice_free_ui_renderer(context->renderer);

	free(context);
}

void alice_apply_default_ui_config(alice_ui_context_t* context) {
	assert(context);

	context->ui_cfg[ALICE_UICFG_PADDING] = 5.0f;
	context->ui_cfg[ALICE_UICFG_OUTLINE_WIDTH] = 1.0f;
	context->ui_cfg[ALICE_UICFG_COLUMN_SIZE] = 200.0f;
	context->ui_cfg[ALICE_UICFG_DEFAULT_WINDOW_MIN_WIDTH] = 200.0f;
	context->ui_cfg[ALICE_UICFG_DEFAULT_WINDOW_MIN_HEIGHT] = 30.0f;

	context->ui_colors[ALICE_UICOLOR_BACKGROUND] = 0xffffff;
	context->ui_colors[ALICE_UICOLOR_OUTLINE] = 0x000000;
	context->ui_colors[ALICE_UICOLOR_TEXT] = 0x000000;
	context->ui_colors[ALICE_UICOLOR_ACTIVE] = 0x7f7f8e;
	context->ui_colors[ALICE_UICOLOR_HOVERED] = 0xdbdbdb;
	context->ui_colors[ALICE_UICOLOR_ACCENT1] = 0x52528c;
	context->ui_colors[ALICE_UICOLOR_ACCENT2] = 0x6b6bbc;
	context->ui_colors[ALICE_UICOLOR_INACTIVE] = 0xb7b7b7;
}

alice_v2f_t alice_calculate_ui_element_dimentions(alice_ui_context_t* context, alice_ui_element_t* element) {
	assert(element);

	const float padding = context->ui_cfg[ALICE_UICFG_PADDING];
	const float column_size = context->ui_cfg[ALICE_UICFG_COLUMN_SIZE];

	switch (element->type) {
		case ALICE_UIELEMENT_BUTTON: {
			alice_ui_button_t* button = (alice_ui_button_t*)element;

			alice_v2f_t text_dimentions =
				alice_calculate_text_dimentions(context->text_renderer, button->text);

			return (alice_v2f_t) {
				.x = text_dimentions.x + padding * 2,
				.y = text_dimentions.y + padding * 2
			};
		}
		case ALICE_UIELEMENT_LABEL: {
			alice_ui_label_t* label = (alice_ui_label_t*)element;
			return alice_calculate_text_dimentions(context->text_renderer, label->text);
		}
		case ALICE_UIELEMENT_TEXTINPUT: {
			alice_ui_text_input_t* input = (alice_ui_text_input_t*)element;

			const alice_v2f_t label_dimentions =
				alice_calculate_text_dimentions(context->text_renderer, input->label);

			return (alice_v2f_t) {
				.x = (label_dimentions.x + input->base.window->dimentions.x) - (padding * 2.0f),
				.y = label_dimentions.y + (padding * 4.0f)
			};
		}
		case ALICE_UIELEMENT_TOGGLE: {
			alice_ui_toggle_t* toggle = (alice_ui_toggle_t*)element;

			const alice_v2f_t label_dimentions =
				alice_calculate_text_dimentions(context->text_renderer, toggle->label);

			return (alice_v2f_t) {
				.x = column_size - (padding * 6.0f),
				.y = label_dimentions.y + (padding * 4.0f)
			};
		}
	}

	return (alice_v2f_t) { 0.0f, 0.0f };
}

typedef struct alice_ui_text_queue_element_t {
	const char* text;
	alice_v2f_t position;
} alice_ui_text_queue_element_t;

typedef struct alice_ui_text_queue_t {
	alice_ui_text_queue_element_t* elements;
	u32 count;
	u32 capacity;
} alice_ui_text_queue_t;

void alice_init_text_queue(alice_ui_text_queue_t* queue) {
	assert(queue);

	queue->elements = alice_null;
	queue->count = 0;
	queue->capacity = 0;
}

void alice_deinit_text_queue(alice_ui_text_queue_t* queue) {
	assert(queue);

	if (queue->capacity > 0) {
		free(queue->elements);
	}

	queue->elements = alice_null;
	queue->count = 0;
	queue->capacity = 0;
}

void alice_text_queue_add(alice_ui_text_queue_t* queue, const char* text, alice_v2f_t position) {
	assert(queue);

	if (queue->count >= queue->capacity) {
		queue->capacity = alice_grow_capacity(queue->capacity);
		queue->elements = realloc(queue->elements, queue->capacity * sizeof(alice_ui_text_queue_t));
	}

	alice_ui_text_queue_element_t* element = &queue->elements[queue->count++];
	element->text = text;
	element->position = position;
}

static bool alice_draw_ui_element(alice_ui_context_t* context, alice_ui_window_t* window,
		alice_ui_element_t* element, alice_ui_text_queue_t* text_queue) {
	assert(context);
	assert(window);
	assert(element);

	const float padding = context->ui_cfg[ALICE_UICFG_PADDING];
	const float outline_thickness = context->ui_cfg[ALICE_UICFG_OUTLINE_WIDTH];
	const float column_size = context->ui_cfg[ALICE_UICFG_COLUMN_SIZE];

	const alice_v2f_t element_dimensions = alice_calculate_ui_element_dimentions(context, element);

	const alice_v2f_t element_position = (alice_v2f_t) {
		.x = window->position.x + element->position.x,
		.y = window->position.y + element->position.y
	};

	const alice_ui_rect_t element_rect = (alice_ui_rect_t) {
		.x = element_position.x,
		.y = element_position.y,
		.w = element_dimensions.x,
		.h = element_dimensions.y
	};

	const alice_ui_rect_t element_outline_rect = (alice_ui_rect_t) {
		.x = element_position.x - outline_thickness,
		.y = element_position.y - outline_thickness,
		.w = element_dimensions.x + (outline_thickness * 2.0f),
		.h = element_dimensions.y + (outline_thickness * 2.0f)
	};

	const alice_color_t outline_color = context->ui_colors[ALICE_UICOLOR_OUTLINE];
	const alice_color_t background_color = context->ui_colors[ALICE_UICOLOR_BACKGROUND];
	const alice_color_t hovered_color = context->ui_colors[ALICE_UICOLOR_HOVERED];
	const alice_color_t active_color = context->ui_colors[ALICE_UICOLOR_ACTIVE];

	bool element_hovered = false;
	bool element_held = false;
	bool element_clicked = false;

	if (alice_mouse_over_ui_rect(element_rect)) {
		element_hovered = true;

		if (context->hovered_element != element && element->on_hover) {
			element->on_hover(context, element);
		}

		if (alice_mouse_button_pressed(ALICE_MOUSE_BUTTON_LEFT)) {
			element_held = true;
		}

		if (window->interactable && alice_mouse_button_just_released(ALICE_MOUSE_BUTTON_LEFT)) {
			if (element->type == ALICE_UIELEMENT_TEXTINPUT ||
					element->type == ALICE_UIELEMENT_TOGGLE) {
				context->active_input = element;
			} else {
				context->active_input = alice_null;
			}

			element_clicked = true;
		}

		context->hovered_element = element;
	} else if (context->hovered_element == element) {
		context->hovered_element = alice_null;
	}

	switch (element->type) {
		case ALICE_UIELEMENT_BUTTON: {
			alice_ui_button_t* button = (alice_ui_button_t*)element;

			alice_color_t color = background_color;
			if (element_held) {
				color = active_color;
			} else if (element_hovered) {
				color = hovered_color;
			}

			alice_draw_ui_rect(context->renderer, element_outline_rect, outline_color);
			alice_draw_ui_rect(context->renderer, element_rect, color);

			const alice_v2f_t text_position = (alice_v2f_t) {
				.x = element_position.x + padding,
				.y = element_position.y
			};

			alice_text_queue_add(text_queue, button->text, text_position);

			break;
		}
		case ALICE_UIELEMENT_TOGGLE: {
			alice_ui_toggle_t* toggle = (alice_ui_toggle_t*)element;

			const alice_ui_rect_t box_rect = (alice_ui_rect_t){
				.x = window->position.x + padding + column_size,
				.y = element_position.y + padding,
				.w = padding * 6.0f,
				.h = element_dimensions.y - (padding * 2.0f)
			};

			const alice_ui_rect_t box_outline_rect = (alice_ui_rect_t){
				.x = box_rect.x - outline_thickness,
				.y = box_rect.y - outline_thickness,
				.w = box_rect.w + (outline_thickness * 2.0f),
				.h = box_rect.h + (outline_thickness * 2.0f)
			};

			alice_draw_ui_rect(context->renderer, box_outline_rect, outline_color);
			alice_draw_ui_rect(context->renderer, box_rect, background_color);

			if (element_clicked) {
				toggle->value = !toggle->value;
			}

			if (toggle->value) {
				const alice_ui_rect_t box_check_rect = (alice_ui_rect_t) {
					.x = box_rect.x + padding,
					.y = box_rect.y + padding,
					.w = box_rect.w - padding * 2.0f,
					.h = box_rect.h - padding * 2.0f
				};

				alice_draw_ui_rect(context->renderer, box_check_rect, active_color);
			}

			const alice_v2f_t label_position = (alice_v2f_t){
				.x = element_position.x,
				.y = element_position.y + padding
			};

			const alice_v2f_t buffer_position = (alice_v2f_t){
				.x = box_rect.x + padding,
				.y = box_rect.y
			};

			alice_text_queue_add(text_queue, toggle->label, label_position);

			break;
		}
		case ALICE_UIELEMENT_LABEL: {
			alice_ui_label_t* label = (alice_ui_label_t*)element;

			alice_text_queue_add(text_queue, label->text, element_position);

			break;
		}
		case ALICE_UIELEMENT_TEXTINPUT: {
			alice_ui_text_input_t* input = (alice_ui_text_input_t*)element;

			const alice_ui_rect_t box_rect = (alice_ui_rect_t) {
				.x = (window->position.x + window->dimentions.x) -
					(padding + column_size),
				.y = element_position.y + padding,
				.w = column_size,
				.h = element_dimensions.y - (padding * 2.0f)
			};

			const alice_ui_rect_t box_outline_rect = (alice_ui_rect_t) {
				.x = box_rect.x - outline_thickness,
				.y = box_rect.y - outline_thickness,
				.w = box_rect.w + (outline_thickness * 2.0f),
				.h = box_rect.h + (outline_thickness * 2.0f)
			};

			alice_draw_ui_rect(context->renderer, box_outline_rect, outline_color);
			alice_draw_ui_rect(context->renderer, box_rect, background_color);

			const alice_v2f_t label_position = (alice_v2f_t) {
				.x = element_position.x,
				.y = element_position.y + padding
			};

			const alice_v2f_t buffer_position = (alice_v2f_t) {
				.x = box_rect.x + padding,
				.y = box_rect.y
			};

			alice_text_queue_add(text_queue, input->label, label_position);
			//alice_text_queue_add(&text_queue, input->buffer, buffer_position);

			break;
		}
	}

	if (element_clicked && element->on_click) {
		element->on_click(context, element);
	}

	return element_hovered;
}

i32 alice_window_z_sort_compare(const void* av, const void* bv) {
	const alice_ui_window_t* a = av;
	const alice_ui_window_t* b = bv;

	return b->z_index - a->z_index;
}

void alice_draw_ui(alice_ui_context_t* context) {
	assert(context);

	const alice_application_t* app = alice_get_application();

	alice_set_text_renderer_dimentions(context->text_renderer,
			(alice_v2f_t){(float)app->width, (float)app->height});
	alice_set_ui_renderer_dimentions(context->renderer, (alice_v2f_t){(float)app->width, (float)app->height});

	alice_set_text_renderer_color(context->text_renderer, context->ui_colors[ALICE_UICOLOR_TEXT]);

	const float padding = context->ui_cfg[ALICE_UICFG_PADDING];
	const float outline_thickness = context->ui_cfg[ALICE_UICFG_OUTLINE_WIDTH];
	const float column_size = context->ui_cfg[ALICE_UICFG_COLUMN_SIZE];

	const alice_color_t outline_color = context->ui_colors[ALICE_UICOLOR_OUTLINE];

	alice_ui_text_queue_t text_queue;
	alice_init_text_queue(&text_queue);

	if (context->z_order_changed) {
		qsort(context->windows, context->window_count,
				sizeof(alice_ui_window_t),
				alice_window_z_sort_compare);
		context->z_order_changed = false;
	}

	for (u32 i = 0; i < context->window_count; i++) {
		alice_ui_window_t* window = &context->windows[i];

		alice_ui_renderer_begin_batch(context->renderer);

		if (!window->visible) { continue; }

		alice_ui_rect_t window_rect = (alice_ui_rect_t) {
			.x = window->position.x,
			.y = window->position.y,
			.w = window->dimentions.x,
			.h = window->dimentions.y
		};

		const alice_v2f_t title_dimension = alice_calculate_text_dimentions(context->text_renderer, window->title);

		const float title_height = (padding) + title_dimension.y;

		alice_ui_rect_t title_rect = (alice_ui_rect_t) {
			.x = window->position.x + outline_thickness,
			.y = window->position.y - title_height + outline_thickness,
			.w = window->dimentions.x - (outline_thickness * 2.0f),
			.h = title_height
		};

		if (
				(alice_mouse_over_ui_rect(window_rect) ||
				alice_mouse_over_ui_rect(title_rect)) &&
				(alice_mouse_button_just_pressed(ALICE_MOUSE_BUTTON_LEFT) ||
				 alice_mouse_button_just_pressed(ALICE_MOUSE_BUTTON_RIGHT))) {
			for (u32 j = 0; j < context->window_count; j++) {
				context->windows[j].z_index++;
			}

			context->z_order_changed = true;
			window->z_index = 0;
		}

		window->interactable = window->z_index == 0;

		if (window->position.x + window->dimentions.x > (float)app->width) {
			window->position.x = (float)app->width - window->dimentions.x;
		}

		if (window->position.y + window->dimentions.y > (float)app->height) {
			window->position.y = (float)app->height - window->dimentions.y;
		}

		if (window->position.x < 0.0f) {
			window->position.x = 0.0f;
		}

		if (window->position.y - title_height < 0.0f) {
			window->position.y = title_height;
		}

		if (alice_mouse_over_ui_rect(title_rect)
				&& alice_mouse_button_just_pressed(ALICE_MOUSE_BUTTON_LEFT)) {
			const alice_v2i_t mouse_pos = alice_get_mouse_position();

			window->being_dragged = true;
			window->drag_offset = (alice_v2f_t) {
				.x = mouse_pos.x - window->position.x,
				.y = mouse_pos.y - window->position.y
			};
		}

		if (!window->interactable) {
			window->being_dragged = false;
		}

		if (window->being_dragged) {
			const alice_v2i_t mouse_pos = alice_get_mouse_position();

			window->position = (alice_v2f_t) {
				.x = mouse_pos.x - window->drag_offset.x,
				.y = mouse_pos.y - window->drag_offset.y
			};

			if (alice_mouse_button_just_released(ALICE_MOUSE_BUTTON_LEFT)) {
				window->being_dragged = false;
			}
		}

		alice_ui_rect_t title_outline_rect = (alice_ui_rect_t) {
			.x = window->position.x,
			.y = window->position.y - title_height,
			.w = window->dimentions.x,
			.h = title_height + (outline_thickness * 2.0f)
		};

		alice_color_t window_background_color = context->ui_colors[ALICE_UICOLOR_INACTIVE];
		if (window->interactable) {
			window_background_color = context->ui_colors[ALICE_UICOLOR_BACKGROUND];
		}

		alice_draw_ui_rect(context->renderer, window_rect, window_background_color);
		alice_draw_ui_rect(context->renderer, title_outline_rect, outline_color);
		alice_draw_ui_rect(context->renderer, title_rect, window_background_color);

		if (window->can_close) {
			const alice_ui_rect_t close_button_rect = (alice_ui_rect_t) {
				.x = title_rect.x + title_rect.w - (title_height - (padding + 2.0f)),
				.y = title_rect.y + padding,
				.w = title_height - (padding * 2.0f),
				.h = title_height - (padding * 2.0f)
			};

			const alice_ui_rect_t close_button_outline_rect = (alice_ui_rect_t) {
				.x = close_button_rect.x - outline_thickness,
				.y = close_button_rect.y - outline_thickness,
				.w = close_button_rect.w + outline_thickness * 2.0f,
				.h = close_button_rect.h + outline_thickness * 2.0f
			};

			alice_color_t close_button_color = context->ui_colors[ALICE_UICOLOR_BACKGROUND];

			if (alice_mouse_over_ui_rect(close_button_rect)) {
				close_button_color = context->ui_colors[ALICE_UICOLOR_HOVERED];

				if (alice_mouse_button_just_released(ALICE_MOUSE_BUTTON_LEFT)) {
					window->visible = false;
					continue;
				}
			}

			alice_draw_ui_rect(context->renderer, close_button_outline_rect, outline_color);
			alice_draw_ui_rect(context->renderer, close_button_rect, close_button_color);

		}

		if (window->can_resize) {
			const alice_ui_rect_t resize_handle_rect = (alice_ui_rect_t) {
				.x = window->position.x + window->dimentions.x - padding * 5.0f,
				.y = window->position.y + window->dimentions.y - padding * 5.0f,
				.w = padding * 4.0f,
				.h = padding * 4.0f
			};

			const alice_ui_rect_t resize_handle_outline_rect = (alice_ui_rect_t) {
				.x = resize_handle_rect.x - outline_thickness,
				.y = resize_handle_rect.y - outline_thickness,
				.w = resize_handle_rect.w + outline_thickness * 2.0f,
				.h = resize_handle_rect.h + outline_thickness * 2.0f
			};

			alice_color_t resize_handle_color = context->ui_colors[ALICE_UICOLOR_BACKGROUND];
			if (alice_mouse_over_ui_rect(resize_handle_rect)) {
				resize_handle_color = context->ui_colors[ALICE_UICOLOR_HOVERED];

				if (alice_mouse_button_just_pressed(ALICE_MOUSE_BUTTON_LEFT)) {
					window->being_resized = true;

					const alice_v2i_t mouse_pos = alice_get_mouse_position();

					window->resize_offset = (alice_v2f_t){
						.x = resize_handle_rect.x - mouse_pos.x,
						.y = resize_handle_rect.y - mouse_pos.y
					};
				}
			}

			if (window->being_resized) {
				const alice_v2i_t mouse_pos = alice_get_mouse_position();

				window->dimentions.x = mouse_pos.x - window->resize_offset.x - window->position.x;
				window->dimentions.y = mouse_pos.y - window->resize_offset.y - window->position.y;

				if (window->dimentions.x < window->min_dimentions.x) {
					window->dimentions.x = window->min_dimentions.x;
				}

				if (window->dimentions.y < window->min_dimentions.y) {
					window->dimentions.y = window->min_dimentions.y;
				}

				if (alice_mouse_button_just_released(ALICE_MOUSE_BUTTON_LEFT)) {
					window->being_resized = false;
				}
			}

			alice_draw_ui_rect(context->renderer, resize_handle_outline_rect, outline_color);
			alice_draw_ui_rect(context->renderer, resize_handle_rect, resize_handle_color);
		}

		alice_v2f_t window_label_position = (alice_v2f_t) {
			.x = window->position.x + padding,
			.y = window->position.y - title_height
		};

		alice_text_queue_add(&text_queue, window->title, window_label_position);

		bool any_element_hovered = false;

		for (u32 i = 0; i < window->element_count; i++) {
			alice_ui_element_t* element = window->elements[i];

			if (alice_draw_ui_element(context, window, element, &text_queue)) {
				any_element_hovered = true;
			}
		}

		if (!any_element_hovered && alice_mouse_button_just_released(ALICE_MOUSE_BUTTON_LEFT)) {
			context->active_input = alice_null;
		}

		alice_ui_renderer_end_batch(context->renderer);

		for (u32 i = 0; i < text_queue.count; i++) {
			alice_ui_text_queue_element_t* e = &text_queue.elements[i];

			alice_render_text(context->text_renderer, e->position, e->text);
		}

		alice_deinit_text_queue(&text_queue);
	}
}

static void alice_draw_gizmo(alice_ui_context_t* context, alice_scene_t* scene, alice_entity_t* entity, alice_m4f_t* view) {
	alice_m4f_t scale = alice_m4f_scale(alice_m4f_identity(), (alice_v3f_t){1.0f, 1.0f, 1.0f});

	alice_m4f_t transform = alice_get_entity_transform(scene, entity);
	transform.elements[0][0] = view->elements[0][0];
	transform.elements[0][1] = view->elements[1][0];
	transform.elements[0][2] = view->elements[2][0];
	transform.elements[1][0] = view->elements[0][1];
	transform.elements[1][1] = view->elements[1][1];
	transform.elements[1][2] = view->elements[2][1];
	transform.elements[2][0] = view->elements[0][2];
	transform.elements[2][1] = view->elements[1][2];
	transform.elements[2][2] = view->elements[2][2];

	transform = alice_m4f_multiply(transform, scale);

	alice_shader_set_m4f(context->gizmo_shader, "transform", transform);

	alice_bind_vertex_buffer_for_draw(context->gizmo_quad);
	alice_draw_vertex_buffer(context->gizmo_quad);
	alice_bind_vertex_buffer_for_draw(alice_null);
}

void alice_draw_scene_gizmos(alice_ui_context_t* context, alice_scene_t* scene) {
	assert(context);

	glCullFace(GL_FRONT);

	alice_camera_3d_t* camera = alice_get_scene_camera_3d(scene);
	if (!camera) { return; }

	alice_m4f_t view = alice_get_camera_3d_view(scene, camera);

	alice_bind_shader(context->gizmo_shader);
	alice_shader_set_m4f(context->gizmo_shader, "camera", alice_get_camera_3d_matrix(scene, camera));

	alice_texture_t* point_light_texture = context->gizmo_textures[ALICE_GIZMOTEXTURE_POINT_LIGHT];
	if (point_light_texture) {
		alice_bind_texture(point_light_texture, 0);
		alice_shader_set_int(context->gizmo_shader, "image", 0);

		for (alice_entity_iter(scene, iter, alice_point_light_t)) {
			alice_point_light_t* light = iter.current_ptr;

			alice_draw_gizmo(context, scene, (alice_entity_t*)light, &view);
		}
	}

	alice_texture_t* sun_texture = context->gizmo_textures[ALICE_GIZMOTEXTURE_DIRECTIONAL_LIGHT];
	if (sun_texture) {
		alice_bind_texture(sun_texture, 0);
		alice_shader_set_int(context->gizmo_shader, "image", 0);

		for (alice_entity_iter(scene, iter, alice_directional_light_t)) {
			alice_directional_light_t* light = iter.current_ptr;

			alice_draw_gizmo(context, scene, (alice_entity_t*)light, &view);
		}
	}

	glCullFace(GL_BACK);
}

alice_ui_window_t* alice_new_ui_window(alice_ui_context_t* context,
		alice_ui_window_create_f create_function) {
	assert(context);

	if (context->window_count >= context->window_capacity) {
		context->window_capacity = alice_grow_capacity(context->window_capacity);
		context->windows = realloc(context->windows, context->window_capacity * sizeof(alice_ui_window_t));
	}

	alice_ui_window_t* new = &context->windows[context->window_count++];

	new->context = context;

	alice_init_ui_window(new, context->window_count);

	if (create_function) {
		create_function(context, new);
	}

	return new;
}

void alice_destroy_ui_window(alice_ui_context_t* context, alice_ui_window_t* window) {
	assert(context);
	assert(window);

	i32 index = -1;

	for (u32 i = 0; i < context->window_count; i++) {
		if (context->windows[i].id == window->id) {
			index = i;
			break;
		}
	}

	if (index == -1) {
		alice_log_warning("Cannot remove an element that a Window doesn't contain");
		return;
	}

	alice_deinit_ui_window(&context->windows[index]);

	for (u32 i = index; i < window->element_count - 1; i++) {
		window->elements[i] = window->elements[i + 1];
	}

	window->element_count--;
}

void alice_free_ui_element(alice_ui_element_t* element) {
	assert(element);

	switch (element->type) {
		case ALICE_UIELEMENT_TEXTINPUT: {
			alice_ui_text_input_t* input = (alice_ui_text_input_t*)element;
			free(input->buffer);
			break;
		}
		default:
			break;
	}

	free(element);
}

void alice_init_ui_window(alice_ui_window_t* window, u32 id) {
	assert(window);
	assert(window->context);

	window->id = id;

	window->element_count = 0;
	window->element_capacity = 0;
	window->elements = alice_null;

	window->visible = true;
	window->can_close = true;
	window->can_resize = true;
	window->interactable = true;

	window->z_index = 0;

	window->being_dragged = false;
	window->being_resized = false;

	window->last_element = alice_null;

	window->min_dimentions = (alice_v2f_t) {
		.x = window->context->ui_cfg[ALICE_UICFG_DEFAULT_WINDOW_MIN_WIDTH],
		.y = window->context->ui_cfg[ALICE_UICFG_DEFAULT_WINDOW_MIN_HEIGHT]
	};
}

void alice_deinit_ui_window(alice_ui_window_t* window) {
	assert(window);

	for (u32 i = 0; i < window->element_count; i++) {
		alice_free_ui_element(window->elements[i]);
	}

	free(window->elements);

	window->id = 0;

	window->last_element = alice_null;

	window->element_count = 0;
	window->element_capacity = 0;
	window->elements = alice_null;
}

void alice_clear_ui_window(alice_ui_window_t* window) {
	assert(window);

	for (u32 i = 0; i < window->element_count; i++) {
		alice_free_ui_element(window->elements[i]);
	}

	window->element_count = 0;
}

void alice_ui_window_remove(alice_ui_window_t* window, alice_ui_element_t* element) {
	assert(window);
	assert(element);

	i32 index = -1;

	for (u32 i = 0; i < window->element_count; i++) {
		if (window->elements[i] == element) {
			index = i;
			break;
		}
	}

	if (index == -1) {
		alice_log_warning("Cannot remove an element that a Window doesn't contain");
		return;
	}

	alice_free_ui_element(window->elements[index]);

	for (u32 i = index; i < window->element_count - 1; i++) {
		window->elements[i] = window->elements[i + 1];
	}

	window->element_count--;
}

static alice_ui_element_t* alice_add_ui_element(alice_ui_window_t* window, alice_ui_element_t* element) {
	assert(window);
	assert(element);

	if (window->element_count >= window->element_capacity) {
		window->element_capacity = alice_grow_capacity(window->element_capacity);
		window->elements = realloc(window->elements, window->element_capacity * sizeof(alice_ui_element_t*));
	}

	window->elements[window->element_count++] = element;

	return element;
}

static alice_ui_element_t* alice_alloc_ui_element(alice_ui_element_type_t type) {
	switch (type) {
		case ALICE_UIELEMENT_BUTTON:
			return malloc(sizeof(alice_ui_button_t));
		case ALICE_UIELEMENT_LABEL:
			return malloc(sizeof(alice_ui_label_t));
		case ALICE_UIELEMENT_TEXTINPUT:
			return malloc(sizeof(alice_ui_text_input_t));
		case ALICE_UIELEMENT_TOGGLE:
			return malloc(sizeof(alice_ui_toggle_t));
		default: /* Unreachable */
			return alice_null;
			break;
	}
}

static alice_ui_element_t* alice_new_ui_element(alice_ui_window_t* window, alice_ui_element_type_t type) {
	alice_ui_element_t* element = alice_alloc_ui_element(type);

	const float padding = window->context->ui_cfg[ALICE_UICFG_PADDING];

	if (window->last_element) {
		alice_v2f_t last_dimensions = alice_calculate_ui_element_dimentions(window->context, window->last_element);

		element->position = (alice_v2f_t){
			.x = padding,
			.y = window->last_element->position.y + last_dimensions.y + padding
		};
	} else {
		element->position = (alice_v2f_t){
			.x = padding,
			.y = padding
		};
	}

	switch (type) {
		case ALICE_UIELEMENT_TEXTINPUT: {
			alice_ui_text_input_t* input = (alice_ui_text_input_t*)element;
			input->buffer = calloc(256, 256);
		}
		default:
			break;
	};

	element->window = window;

	element->on_hover = alice_null;
	element->on_click = alice_null;

	element->type = type;

	window->last_element = element;

	return alice_add_ui_element(window, element);
}

alice_ui_button_t* alice_add_ui_button(alice_ui_window_t* window) {
	assert(window);

	return (alice_ui_button_t*)alice_new_ui_element(window, ALICE_UIELEMENT_BUTTON);
}

alice_ui_label_t* alice_add_ui_label(alice_ui_window_t* window) {
	assert(window);

	return (alice_ui_label_t*)alice_new_ui_element(window, ALICE_UIELEMENT_LABEL);
}

alice_ui_text_input_t* alice_add_ui_text_input(alice_ui_window_t* window) {
	assert(window);

	return (alice_ui_text_input_t*)alice_new_ui_element(window, ALICE_UIELEMENT_TEXTINPUT);
}

alice_ui_toggle_t* alice_add_ui_toggle(alice_ui_window_t* window) {
	assert(window);

	return (alice_ui_toggle_t*)alice_new_ui_element(window, ALICE_UIELEMENT_TOGGLE);
}

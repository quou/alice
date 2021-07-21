#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <stb_truetype.h>
#include <glad/glad.h>

#include "alice/ui.h"
#include "alice/application.h"
#include "alice/input.h"

struct alice_TextRenderer {
	alice_VertexBuffer* vb;
	alice_Texture* font_bitmap;
	alice_Shader* shader;

	float font_size;

	stbtt_packedchar char_data[96];
	stbtt_fontinfo font_info;

	alice_Color color;

	alice_v2f dimentions;
};

static const u32 alice_font_bitmap_size = 512;

alice_TextRenderer* alice_new_text_renderer(alice_Resource* font_data,
		float font_size, alice_Shader* font_shader) {
	alice_TextRenderer* new = malloc(sizeof(alice_TextRenderer));

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

	alice_VertexBuffer* buffer = alice_new_vertex_buffer(
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

void alice_free_text_renderer(alice_TextRenderer* renderer) {
	assert(renderer);

	alice_free_vertex_buffer(renderer->vb);
	alice_free_texture(renderer->font_bitmap);

	free(renderer);
}

void alice_set_text_renderer_dimentions(alice_TextRenderer* renderer, alice_v2f dimentions) {
	assert(renderer);

	renderer->dimentions = dimentions;
}

void alice_render_text(alice_TextRenderer* renderer, alice_v2f position, const char* string) {
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

	alice_m4f projection = alice_m4f_ortho(0.0, renderer->dimentions.x,
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

void alice_set_text_renderer_color(alice_TextRenderer* renderer, alice_Color color) {
	assert(renderer);

	renderer->color = color;
}

alice_v2f alice_calculate_text_dimentions(alice_TextRenderer* renderer, const char* text) {
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

	return (alice_v2f){
		.x = width,
		.y = height
	};
}

bool alice_mouse_over_ui_rect(alice_UIRect rect) {
	alice_v2i mouse_pos = alice_get_mouse_position();

	return
			mouse_pos.x > rect.x && mouse_pos.x < rect.x + rect.w &&
			mouse_pos.y > rect.y && mouse_pos.y < rect.y + rect.h;
}

alice_UIRenderer* alice_new_ui_renderer(alice_Shader* rect_shader) {
	assert(rect_shader);

	alice_UIRenderer* new = malloc(sizeof(alice_UIRenderer));

	new->max_quads = 10000;
	new->quad_count = 0;

	new->shader = rect_shader;
	new->projection = alice_m4f_identity();

	alice_VertexBuffer* buffer = alice_new_vertex_buffer(
			ALICE_VERTEXBUFFER_DRAW_TRIANGLES | ALICE_VERTEXBUFFER_DYNAMIC_DRAW);

	alice_bind_vertex_buffer_for_edit(buffer);
	alice_push_vertices(buffer, alice_null, (5 * 4) * new->max_quads);
	alice_push_indices(buffer, alice_null, 6 * new->max_quads);
	alice_configure_vertex_buffer(buffer, 0, 2, 5, 0); /* vec2 position */
	alice_configure_vertex_buffer(buffer, 1, 3, 5, 2); /* vec3 color */
	alice_bind_vertex_buffer_for_edit(alice_null);

	new->vb = buffer;

	return new;
}

void alice_free_ui_renderer(alice_UIRenderer* renderer) {
	assert(renderer);

	alice_free_vertex_buffer(renderer->vb);

	free(renderer);
}

void alice_set_ui_renderer_dimentions(alice_UIRenderer* renderer, alice_v2f dimentions) {
	assert(renderer);

	renderer->projection = alice_m4f_ortho(0.0f, dimentions.x, dimentions.y, 0.0f, -1.0f, 1.0f);
}

void alice_ui_renderer_begin_batch(alice_UIRenderer* renderer) {
	assert(renderer);

	renderer->quad_count = 0;
}

void alice_ui_renderer_end_batch(alice_UIRenderer* renderer) {
	assert(renderer);

	alice_bind_shader(renderer->shader);

	alice_shader_set_m4f(renderer->shader, "camera", renderer->projection);

	alice_bind_vertex_buffer_for_draw(renderer->vb);
	alice_draw_vertex_buffer_custom_count(renderer->vb, renderer->quad_count * 6);
	alice_bind_vertex_buffer_for_draw(alice_null);

	alice_bind_shader(alice_null);
}

void alice_draw_ui_rect(alice_UIRenderer* renderer, alice_UIRect rect, alice_Color color) {
	assert(renderer);

	if (renderer->quad_count >= renderer->max_quads) {
		alice_ui_renderer_end_batch(renderer);
		alice_ui_renderer_begin_batch(renderer);
	}

	alice_RGBColor rgb = alice_rgb_color_from_color(color);

	float verts[] = {
		rect.x + rect.w,	rect.y + rect.h,	rgb.r, rgb.g, rgb.b,
		rect.x + rect.w,	rect.y,			rgb.r, rgb.g, rgb.b,
		rect.x,			rect.y,			rgb.r, rgb.g, rgb.b,
		rect.x,			rect.y + rect.h,	rgb.r, rgb.g, rgb.b
	};

	const u32 index_offset = renderer->quad_count * 4;

	u32 indices[] = {
		index_offset + 0, index_offset + 1, index_offset + 3,
		index_offset + 1, index_offset + 2, index_offset + 3
	};

	alice_bind_vertex_buffer_for_edit(renderer->vb);
	alice_update_vertices(renderer->vb, verts, renderer->quad_count * 5 * 4, 5 * 4);
	alice_update_indices(renderer->vb, indices, renderer->quad_count * 6, 6);
	alice_bind_vertex_buffer_for_edit(alice_null);

	renderer->quad_count++;
}


alice_UIContext* alice_new_ui_context(alice_Shader* rect_shader, alice_Shader* text_shader,
		alice_Resource* font_data, float font_size) {
	alice_UIContext* new = malloc(sizeof(alice_UIContext));

	new->text_renderer = alice_new_text_renderer(font_data, font_size, text_shader);
	new->renderer = alice_new_ui_renderer(rect_shader);

	alice_apply_default_ui_config(new);

	new->window_count = 0;
	new->window_capacity = 0;
	new->windows = alice_null;

	new->hovered_element = alice_null;
	new->active_input = alice_null;

	new->text_size = font_size;

	return new;
}

void alice_free_ui_context(alice_UIContext* context) {
	assert(context);

	for (u32 i = 0; i < context->window_count; i++) {
		alice_deinit_ui_window(&context->windows[i]);
	}

	alice_free_text_renderer(context->text_renderer);
	alice_free_ui_renderer(context->renderer);
}

void alice_apply_default_ui_config(alice_UIContext* context) {
	assert(context);

	context->ui_cfg[ALICE_UICFG_PADDING] = 5.0f;
	context->ui_cfg[ALICE_UICFG_OUTLINE_WIDTH] = 1.0f;
	context->ui_cfg[ALICE_UICFG_COLUMN_SIZE] = 200.0f;

	context->ui_colors[ALICE_UICOLOR_BACKGROUND] = 0xffffff;
	context->ui_colors[ALICE_UICOLOR_OUTLINE] = 0x000000;
	context->ui_colors[ALICE_UICOLOR_TEXT] = 0x000000;
	context->ui_colors[ALICE_UICOLOR_ACTIVE] = 0x7f7f8e;
	context->ui_colors[ALICE_UICOLOR_HOVERED] = 0xdbdbdb;
	context->ui_colors[ALICE_UICOLOR_ACCENT1] = 0x52528c;
	context->ui_colors[ALICE_UICOLOR_ACCENT2] = 0x6b6bbc;
}

alice_v2f alice_calculate_ui_element_dimentions(alice_UIContext* context, alice_UIElement* element) {
	assert(element);

	const float padding = context->ui_cfg[ALICE_UICFG_PADDING];

	switch (element->type) {
		case ALICE_UIELEMENT_BUTTON: {
			alice_UIButton* button = (alice_UIButton*)element;

			alice_v2f text_dimentions =
				alice_calculate_text_dimentions(context->text_renderer, button->text);

			return (alice_v2f) {
				.x = text_dimentions.x + padding * 2,
				.y = text_dimentions.y + padding * 2
			};
		}
		case ALICE_UIELEMENT_LABEL: {
			alice_UILabel* label = (alice_UILabel*)element;
			return alice_calculate_text_dimentions(context->text_renderer, label->text);
		}
		case ALICE_UIELEMENT_TEXTINPUT: {
			alice_UITextInput* input = (alice_UITextInput*)element;

			const alice_v2f label_dimentions =
				alice_calculate_text_dimentions(context->text_renderer, input->label);

			return (alice_v2f) {
				.x = (label_dimentions.x + input->base.window->dimentions.x) - (padding * 2.0f),
				.y = label_dimentions.y + (padding * 4.0f)
			};
		}
	}

	return (alice_v2f) { 0.0f, 0.0f };
}

typedef struct alice_UITextQueueElement {
	const char* text;
	alice_v2f position;
} alice_UITextQueueElement;

typedef struct alice_UITextQueue {
	alice_UITextQueueElement* elements;
	u32 count;
	u32 capacity;
} alice_UITextQueue;

void alice_init_text_queue(alice_UITextQueue* queue) {
	assert(queue);

	queue->elements = alice_null;
	queue->count = 0;
	queue->capacity = 0;
}

void alice_deinit_text_queue(alice_UITextQueue* queue) {
	assert(queue);

	if (queue->capacity > 0) {
		free(queue->elements);
	}

	queue->elements = alice_null;
	queue->count = 0;
	queue->capacity = 0;
}

void alice_text_queue_add(alice_UITextQueue* queue, const char* text, alice_v2f position) {
	assert(queue);

	if (queue->count >= queue->capacity) {
		queue->capacity = alice_grow_capacity(queue->capacity);
		queue->elements = realloc(queue->elements, queue->capacity * sizeof(alice_UITextQueue));
	}

	alice_UITextQueueElement* element = &queue->elements[queue->count++];
	element->text = text;
	element->position = position;
}

void alice_draw_ui(alice_UIContext* context) {
	assert(context);

	const alice_Application* app = alice_get_application();

	alice_set_text_renderer_dimentions(context->text_renderer, (alice_v2f){app->width, app->height});
	alice_set_ui_renderer_dimentions(context->renderer, (alice_v2f){app->width, app->height});

	alice_set_text_renderer_color(context->text_renderer, context->ui_colors[ALICE_UICOLOR_TEXT]);

	const float padding = context->ui_cfg[ALICE_UICFG_PADDING];
	const float outline_thickness = context->ui_cfg[ALICE_UICFG_OUTLINE_WIDTH];
	const float column_size = context->ui_cfg[ALICE_UICFG_COLUMN_SIZE];

	alice_UITextQueue text_queue;
	alice_init_text_queue(&text_queue);

	/* Draw rectangles */
	alice_ui_renderer_begin_batch(context->renderer);

	for (u32 i = 0; i < context->window_count; i++) {
		alice_UIWindow* window = &context->windows[i];

		alice_UIRect window_rect = (alice_UIRect) {
			.x = window->position.x,
			.y = window->position.y,
			.w = window->dimentions.x,
			.h = window->dimentions.y
		};

		const alice_v2f title_dimension = alice_calculate_text_dimentions(context->text_renderer, window->title);

		const float title_height = (padding) + title_dimension.y;

		alice_UIRect title_rect = (alice_UIRect) {
			.x = window->position.x + outline_thickness,
			.y = window->position.y - title_height + outline_thickness,
			.w = window->dimentions.x - (outline_thickness * 2.0f),
			.h = title_height
		};

		if (alice_mouse_over_ui_rect(title_rect)
				&& alice_mouse_button_just_pressed(ALICE_MOUSE_BUTTON_LEFT)) {
			const alice_v2i mouse_pos = alice_get_mouse_position();

			window->being_dragged = true;
			window->drag_offset = (alice_v2f) {
				.x = mouse_pos.x - window->position.x,
				.y = mouse_pos.y - window->position.y
			};
		}

		if (window->being_dragged) {
			const alice_v2i mouse_pos = alice_get_mouse_position();

			window->position = (alice_v2f) {
				.x = mouse_pos.x - window->drag_offset.x,
				.y = mouse_pos.y - window->drag_offset.y
			};

			if (alice_mouse_button_just_released(ALICE_MOUSE_BUTTON_LEFT)) {
				window->being_dragged = false;
			}
		}

		alice_UIRect title_outline_rect = (alice_UIRect) {
			.x = window->position.x,
			.y = window->position.y - title_height,
			.w = window->dimentions.x,
			.h = title_height + (outline_thickness * 2.0f)
		};
		alice_draw_ui_rect(context->renderer, window_rect, context->ui_colors[ALICE_UICOLOR_BACKGROUND]);
		alice_draw_ui_rect(context->renderer, title_outline_rect, context->ui_colors[ALICE_UICOLOR_OUTLINE]);
		alice_draw_ui_rect(context->renderer, title_rect, context->ui_colors[ALICE_UICOLOR_BACKGROUND]);

		alice_v2f window_label_position = (alice_v2f) {
			.x = window->position.x + padding,
			.y = window->position.y - title_height
		};

		alice_text_queue_add(&text_queue, window->title, window_label_position);

		bool any_element_hovered = false;

		for (u32 i = 0; i < window->element_count; i++) {
			alice_UIElement* element = window->elements[i];

			const alice_v2f element_dimensions = alice_calculate_ui_element_dimentions(context, element);

			const alice_v2f element_position = (alice_v2f) {
				.x = window->position.x + element->position.x,
				.y = window->position.y + element->position.y
			};

			const alice_UIRect element_rect = (alice_UIRect) {
				.x = element_position.x,
				.y = element_position.y,
				.w = element_dimensions.x,
				.h = element_dimensions.y
			};

			const alice_UIRect element_outline_rect = (alice_UIRect) {
				.x = element_position.x - outline_thickness,
				.y = element_position.y - outline_thickness,
				.w = element_dimensions.x + (outline_thickness * 2.0f),
				.h = element_dimensions.y + (outline_thickness * 2.0f)
			};

			const alice_Color outline_color = context->ui_colors[ALICE_UICOLOR_OUTLINE];
			const alice_Color background_color = context->ui_colors[ALICE_UICOLOR_BACKGROUND];
			const alice_Color hovered_color = context->ui_colors[ALICE_UICOLOR_HOVERED];
			const alice_Color active_color = context->ui_colors[ALICE_UICOLOR_ACTIVE];

			bool element_hovered = false;
			bool element_held = false;

			if (alice_mouse_over_ui_rect(element_rect)) {
				element_held = true;
				any_element_hovered = true;

				if (context->hovered_element != element && element->on_hover) {
					element->on_hover(context, element);
				}

				if (alice_mouse_button_pressed(ALICE_MOUSE_BUTTON_LEFT)) {
					element_held = true;
				}

				if (alice_mouse_button_just_released(ALICE_MOUSE_BUTTON_LEFT)) {
					if (element->type == ALICE_UIELEMENT_TEXTINPUT) {
						context->active_input = element;
					} else {
						context->active_input = alice_null;
					}

					if (element->on_click) {
						element->on_click(context, element);
					}
				}

				context->hovered_element = element;
			} else if (context->hovered_element == element) {
				context->hovered_element = alice_null;
			}

			switch (element->type) {
				case ALICE_UIELEMENT_BUTTON: {
					alice_UIButton* button = (alice_UIButton*)element;

					alice_Color color = background_color;
					if (element_held) {
						color = active_color;
					} else if (element_hovered) {
						color = hovered_color;
					}

					alice_draw_ui_rect(context->renderer, element_outline_rect, outline_color);
					alice_draw_ui_rect(context->renderer, element_rect, color);

					const alice_v2f text_position = (alice_v2f) {
						.x = element_position.x + padding,
						.y = element_position.y
					};

					alice_text_queue_add(&text_queue, button->text, text_position);

					break;
				}
				case ALICE_UIELEMENT_LABEL: {
					alice_UILabel* label = (alice_UILabel*)element;

					alice_text_queue_add(&text_queue, label->text, element_position);

					break;
				}
				case ALICE_UIELEMENT_TEXTINPUT: {
					alice_UITextInput* input = (alice_UITextInput*)element;

					const alice_UIRect box_rect = (alice_UIRect) {
						.x = (window->position.x + window->dimentions.x) -
							(padding + column_size),
						.y = element_position.y + padding,
						.w = column_size,
						.h = element_dimensions.y - (padding * 2.0f)
					};

					const alice_UIRect box_outline_rect = (alice_UIRect) {
						.x = box_rect.x - outline_thickness,
						.y = box_rect.y - outline_thickness,
						.w = box_rect.w + (outline_thickness * 2.0f),
						.h = box_rect.h + (outline_thickness * 2.0f)
					};

					alice_draw_ui_rect(context->renderer, box_outline_rect, outline_color);
					alice_draw_ui_rect(context->renderer, box_rect, background_color);

					const alice_v2f label_position = (alice_v2f) {
						.x = element_position.x,
						.y = element_position.y + padding
					};

					const alice_v2f buffer_position = (alice_v2f) {
						.x = box_rect.x + padding,
						.y = box_rect.y
					};

					alice_text_queue_add(&text_queue, input->label, label_position);
					//alice_text_queue_add(&text_queue, input->buffer, buffer_position);

					break;
				}
			}
		}

		if (!any_element_hovered && alice_mouse_button_just_released(ALICE_MOUSE_BUTTON_LEFT)) {
			context->active_input = alice_null;
		}
	}

	alice_ui_renderer_end_batch(context->renderer);

	for (u32 i = 0; i < text_queue.count; i++) {
		alice_UITextQueueElement* e = &text_queue.elements[i];

		alice_render_text(context->text_renderer, e->position, e->text);
	}

	alice_deinit_text_queue(&text_queue);
}

alice_UIWindow* alice_new_ui_window(alice_UIContext* context,
		alice_WindowCreateFunction create_function) {
	assert(context);

	if (context->window_count >= context->window_capacity) {
		context->window_capacity = alice_grow_capacity(context->window_capacity);
		context->windows = realloc(context->windows, context->window_capacity * sizeof(alice_UIWindow));
	}

	alice_UIWindow* new = &context->windows[context->window_count++];

	alice_init_ui_window(new, context->window_count);

	new->context = context;

	if (create_function) {
		create_function(context, new);
	}

	return new;
}

void alice_destroy_ui_window(alice_UIContext* context, alice_UIWindow* window) {
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

void alice_free_ui_element(alice_UIElement* element) {
	assert(element);

	switch (element->type) {
		case ALICE_UIELEMENT_TEXTINPUT: {
			alice_UITextInput* input = (alice_UITextInput*)element;
			free(input->buffer);
			break;
		}
		default:
			break;
	}

	free(element);
}

void alice_init_ui_window(alice_UIWindow* window, u32 id) {
	assert(window);

	window->id = id;

	window->element_count = 0;
	window->element_capacity = 0;
	window->elements = alice_null;

	window->being_dragged = false;

	window->last_element = alice_null;
}

void alice_deinit_ui_window(alice_UIWindow* window) {
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

void alice_clear_ui_window(alice_UIWindow* window) {
	assert(window);

	for (u32 i = 0; i < window->element_count; i++) {
		alice_free_ui_element(window->elements[i]);
	}

	window->element_count = 0;
}

void alice_ui_window_remove(alice_UIWindow* window, alice_UIElement* element) {
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

static alice_UIElement* alice_add_ui_element(alice_UIWindow* window, alice_UIElement* element) {
	assert(window);
	assert(element);

	if (window->element_count >= window->element_capacity) {
		window->element_capacity = alice_grow_capacity(window->element_capacity);
		window->elements = realloc(window->elements, window->element_capacity * sizeof(alice_UIElement*));
	}

	window->elements[window->element_count++] = element;

	return element;
}

static alice_UIElement* alice_alloc_ui_element(alice_UIElementType type) {
	switch (type) {
		case ALICE_UIELEMENT_BUTTON:
			return malloc(sizeof(alice_UIButton));
		case ALICE_UIELEMENT_LABEL:
			return malloc(sizeof(alice_UILabel));
		case ALICE_UIELEMENT_TEXTINPUT:
			return malloc(sizeof(alice_UITextInput));
		default: /* Unreachable */
			return alice_null;
			break;
	}
}

static alice_UIElement* alice_new_ui_element(alice_UIWindow* window, alice_UIElementType type) {
	alice_UIElement* element = alice_alloc_ui_element(type);

	const float padding = window->context->ui_cfg[ALICE_UICFG_PADDING];

	if (window->last_element) {
		alice_v2f last_dimensions = alice_calculate_ui_element_dimentions(window->context, window->last_element);

		element->position = (alice_v2f){
			.x = padding,
			.y = window->last_element->position.y + last_dimensions.y + padding
		};
	} else {
		element->position = (alice_v2f){
			.x = padding,
			.y = padding
		};
	}

	switch (type) {
		case ALICE_UIELEMENT_TEXTINPUT: {
			alice_UITextInput* input = (alice_UITextInput*)element;
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

alice_UIButton* alice_add_ui_button(alice_UIWindow* window) {
	assert(window);

	return (alice_UIButton*)alice_new_ui_element(window, ALICE_UIELEMENT_BUTTON);
}

alice_UILabel* alice_add_ui_label(alice_UIWindow* window) {
	assert(window);

	return (alice_UILabel*)alice_new_ui_element(window, ALICE_UIELEMENT_LABEL);
}

alice_UITextInput* alice_add_ui_text_input(alice_UIWindow* window) {
	assert(window);

	return (alice_UITextInput*)alice_new_ui_element(window, ALICE_UIELEMENT_TEXTINPUT);
}

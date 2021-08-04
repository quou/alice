#pragma once

#include "alice/core.h"
#include "alice/maths.h"
#include "alice/resource.h"
#include "alice/graphics.h"

typedef struct alice_text_renderer_t alice_text_renderer_t;
typedef struct alice_ui_context_t alice_ui_context_t;
typedef struct alice_ui_element_t alice_ui_element_t;
typedef struct alice_ui_window_t alice_ui_window_t;

ALICE_API alice_text_renderer_t* alice_new_text_renderer(alice_resource_t* font_data,
		float font_size, alice_shader_t* font_shader);
ALICE_API void alice_free_text_renderer(alice_text_renderer_t* renderer);

ALICE_API void alice_set_text_renderer_color(alice_text_renderer_t* renderer, alice_color_t color);

ALICE_API void alice_set_text_renderer_dimentions(alice_text_renderer_t* renderer, alice_v2f_t dimentions);
ALICE_API void alice_render_text(alice_text_renderer_t* renderer, alice_v2f_t position, const char* string);

ALICE_API alice_v2f_t alice_calculate_text_dimentions(alice_text_renderer_t* renderer, const char* string);

typedef struct alice_ui_rect_t {
	float x, y, w, h;
} alice_ui_rect_t;

ALICE_API bool alice_mouse_over_ui_rect(alice_ui_rect_t rect);

typedef struct alice_ui_renderer_t {
	alice_vertex_buffer_t* vb;
	alice_shader_t* shader;
	alice_m4f_t projection;

	u32 quad_count;
	u32 max_quads;

	u32 vertices_per_quad;
	u32 indices_per_quad;
} alice_ui_renderer_t;

ALICE_API alice_ui_renderer_t* alice_new_ui_renderer(alice_shader_t* rect_shader);
ALICE_API void alice_free_ui_renderer(alice_ui_renderer_t* renderer);
ALICE_API void alice_set_ui_renderer_dimentions(alice_ui_renderer_t* renderer, alice_v2f_t dimentions);
ALICE_API void alice_ui_renderer_begin_batch(alice_ui_renderer_t* renderer);
ALICE_API void alice_ui_renderer_end_batch(alice_ui_renderer_t* renderer);
ALICE_API void alice_draw_ui_rect(alice_ui_renderer_t* renderer, alice_ui_rect_t rect, alice_color_t color);

typedef void (*alice_ui_element_on_click_f)(alice_ui_context_t* context, alice_ui_element_t* element);
typedef void (*alice_ui_element_on_hover_f)(alice_ui_context_t* context, alice_ui_element_t* element);

typedef enum alice_ui_element_type_t {
	ALICE_UIELEMENT_BUTTON,
	ALICE_UIELEMENT_LABEL,
	ALICE_UIELEMENT_TEXTINPUT,
	ALICE_UIELEMENT_TOGGLE
} alice_ui_element_type_t;

struct alice_ui_element_t {
	alice_ui_element_type_t type;
	alice_v2f_t position;

	alice_ui_window_t* window;

	alice_ui_element_on_click_f on_click;
	alice_ui_element_on_hover_f on_hover;
};

typedef struct alice_ui_button_t {
	alice_ui_element_t base;
	const char* text;
} alice_ui_button_t;

typedef struct alice_ui_label_t {
	alice_ui_element_t base;
	const char* text;
} alice_ui_label_t;

typedef struct alice_ui_text_input_t {
	alice_ui_element_t base;
	const char* label;
	char* buffer;
} alice_ui_text_input_t;

typedef struct alice_ui_toggle_t {
	alice_ui_element_t base;
	const char* label;
	bool value;
} alice_ui_toggle_t;

ALICE_API alice_v2f_t alice_calculate_ui_element_dimentions(alice_ui_context_t* context, alice_ui_element_t* element);

struct alice_ui_window_t {
	u32 id;

	alice_ui_context_t* context;

	alice_ui_element_t** elements;
	u32 element_count;
	u32 element_capacity;

	bool can_close;
	bool visible;
	bool interactable;

	u32 z_index;

	alice_ui_element_t* last_element;

	bool being_dragged;
	alice_v2f_t drag_offset;

	const char* title;
	alice_v2f_t position;
	alice_v2f_t dimentions;
};

ALICE_API void alice_init_ui_window(alice_ui_window_t* window, u32 id);
ALICE_API void alice_deinit_ui_window(alice_ui_window_t* window);

ALICE_API void alice_clear_ui_window(alice_ui_window_t* window);
ALICE_API void alice_ui_window_remove(alice_ui_window_t* window, alice_ui_element_t* element);

ALICE_API void alice_free_ui_element(alice_ui_element_t* element);

ALICE_API alice_ui_button_t* alice_add_ui_button(alice_ui_window_t* window);
ALICE_API alice_ui_label_t* alice_add_ui_label(alice_ui_window_t* window);
ALICE_API alice_ui_text_input_t* alice_add_ui_text_input(alice_ui_window_t* window);
ALICE_API alice_ui_toggle_t* alice_add_ui_toggle(alice_ui_window_t* window);

enum {
	ALICE_UICFG_PADDING = 0,
	ALICE_UICFG_OUTLINE_WIDTH = 1,
	ALICE_UICFG_COLUMN_SIZE = 2,

	ALICE_UICFG_COUNT = 3
};

enum {
	ALICE_UICOLOR_BACKGROUND = 0,
	ALICE_UICOLOR_OUTLINE = 1,
	ALICE_UICOLOR_WINDOW_TITLE_BACKGROUND = 2,
	ALICE_UICOLOR_TEXT = 3,
	ALICE_UICOLOR_ACTIVE = 4,
	ALICE_UICOLOR_HOVERED = 5,
	ALICE_UICOLOR_ACCENT1 = 6,
	ALICE_UICOLOR_ACCENT2 = 7,
	ALICE_UICOLOR_INACTIVE = 8,

	ALICE_UICOLOR_COUNT = 9
};

enum {
	ALICE_GIZMOTEXTURE_POINT_LIGHT = 0,
	ALICE_GIZMOTEXTURE_DIRECTIONAL_LIGHT = 1,

	ALICE_GIZMOTEXTURE_COUNT = 2
};

struct alice_ui_context_t {
	alice_text_renderer_t* text_renderer;
	alice_ui_renderer_t* renderer;

	alice_color_t ui_colors[ALICE_UICOLOR_COUNT];
	float ui_cfg[ALICE_UICFG_COUNT];

	alice_ui_window_t* windows;
	u32 window_count;
	u32 window_capacity;

	alice_ui_element_t* hovered_element;
	alice_ui_element_t* active_input;

	bool z_order_changed;

	float text_size;

	void* user_pointer;

	alice_texture_t* gizmo_textures[ALICE_GIZMOTEXTURE_COUNT];
	alice_shader_t* gizmo_shader;
	alice_vertex_buffer_t* gizmo_quad;
};

ALICE_API alice_ui_context_t* alice_new_ui_context(alice_shader_t* rect_shader,
		alice_shader_t* text_shader,
		alice_shader_t* gizmo_shader,
		alice_resource_t* font_data, float font_size);
ALICE_API void alice_apply_default_ui_config(alice_ui_context_t* context);
ALICE_API void alice_free_ui_context(alice_ui_context_t* context);

ALICE_API void alice_draw_ui(alice_ui_context_t* context);
ALICE_API void alice_draw_scene_gizmos(alice_ui_context_t* context, alice_scene_t* scene);

typedef void (*alice_ui_window_create_f)(alice_ui_context_t*, alice_ui_window_t*);

ALICE_API alice_ui_window_t* alice_new_ui_window(alice_ui_context_t* context,
		alice_ui_window_create_f create_function);
ALICE_API void alice_destroy_ui_window(alice_ui_context_t* context, alice_ui_window_t* window);

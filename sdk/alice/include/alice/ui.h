#pragma once

#include "alice/core.h"
#include "alice/maths.h"
#include "alice/resource.h"
#include "alice/graphics.h"

typedef struct alice_TextRenderer alice_TextRenderer;
typedef struct alice_UIContext alice_UIContext;
typedef struct alice_UIElement alice_UIElement;
typedef struct alice_UIWindow alice_UIWindow;

ALICE_API alice_TextRenderer* alice_new_text_renderer(alice_Resource* font_data,
		float font_size, alice_Shader* font_shader);
ALICE_API void alice_free_text_renderer(alice_TextRenderer* renderer);

ALICE_API void alice_set_text_renderer_color(alice_TextRenderer* renderer, alice_Color color);

ALICE_API void alice_set_text_renderer_dimentions(alice_TextRenderer* renderer, alice_v2f dimentions);
ALICE_API void alice_render_text(alice_TextRenderer* renderer, alice_v2f position, const char* string);

ALICE_API alice_v2f alice_calculate_text_dimentions(alice_TextRenderer* renderer, const char* string);

typedef struct alice_UIRect {
	float x, y, w, h;
} alice_UIRect;

ALICE_API bool alice_mouse_over_ui_rect(alice_UIRect rect);

typedef struct alice_UIRenderer {
	alice_VertexBuffer* vb;
	alice_Shader* shader;
	alice_m4f projection;

	u32 quad_count;
	u32 max_quads;

	u32 vertices_per_quad;
	u32 indices_per_quad;
} alice_UIRenderer;

ALICE_API alice_UIRenderer* alice_new_ui_renderer(alice_Shader* rect_shader);
ALICE_API void alice_free_ui_renderer(alice_UIRenderer* renderer);
ALICE_API void alice_set_ui_renderer_dimentions(alice_UIRenderer* renderer, alice_v2f dimentions);
ALICE_API void alice_ui_renderer_begin_batch(alice_UIRenderer* renderer);
ALICE_API void alice_ui_renderer_end_batch(alice_UIRenderer* renderer);
ALICE_API void alice_draw_ui_rect(alice_UIRenderer* renderer, alice_UIRect rect, alice_Color color);

typedef void (*alice_UIElementOnClickFunction)(alice_UIContext* context, alice_UIElement* element);
typedef void (*alice_UIElementOnHoverFunction)(alice_UIContext* context, alice_UIElement* element);

typedef enum alice_UIElementType {
	ALICE_UIELEMENT_BUTTON,
	ALICE_UIELEMENT_LABEL,
	ALICE_UIELEMENT_TEXTINPUT,
	ALICE_UIELEMENT_TOGGLE
} alice_UIElementType;

struct alice_UIElement {
	alice_UIElementType type;
	alice_v2f position;

	alice_UIWindow* window;

	alice_UIElementOnClickFunction on_click;
	alice_UIElementOnHoverFunction on_hover;
};

typedef struct alice_UIButton {
	alice_UIElement base;
	const char* text;
} alice_UIButton;

typedef struct alice_UILabel {
	alice_UIElement base;
	const char* text;
} alice_UILabel;

typedef struct alice_UITextInput {
	alice_UIElement base;
	const char* label;
	char* buffer;
} alice_UITextInput;

typedef struct alice_UIToggle {
	alice_UIElement base;
	const char* label;
	bool value;
} alice_UIToggle;

ALICE_API alice_v2f alice_calculate_ui_element_dimentions(alice_UIContext* context, alice_UIElement* element);

struct alice_UIWindow {
	u32 id;

	alice_UIContext* context;

	alice_UIElement** elements;
	u32 element_count;
	u32 element_capacity;

	bool can_close;
	bool visible;
	bool interactable;

	u32 z_index;

	alice_UIElement* last_element;

	bool being_dragged;
	alice_v2f drag_offset;

	const char* title;
	alice_v2f position;
	alice_v2f dimentions;
};

ALICE_API void alice_init_ui_window(alice_UIWindow* window, u32 id);
ALICE_API void alice_deinit_ui_window(alice_UIWindow* window);

ALICE_API void alice_clear_ui_window(alice_UIWindow* window);
ALICE_API void alice_ui_window_remove(alice_UIWindow* window, alice_UIElement* element);

ALICE_API void alice_free_ui_element(alice_UIElement* element);

ALICE_API alice_UIButton* alice_add_ui_button(alice_UIWindow* window);
ALICE_API alice_UILabel* alice_add_ui_label(alice_UIWindow* window);
ALICE_API alice_UITextInput* alice_add_ui_text_input(alice_UIWindow* window);
ALICE_API alice_UIToggle* alice_add_ui_toggle(alice_UIWindow* window);

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

	ALICE_UICOLOR_COUNT = 8
};

enum {
	ALICE_GIZMOTEXTURE_POINT_LIGHT = 0,
	ALICE_GIZMOTEXTURE_DIRECTIONAL_LIGHT = 1,

	ALICE_GIZMOTEXTURE_COUNT = 2
};

struct alice_UIContext {
	alice_TextRenderer* text_renderer;
	alice_UIRenderer* renderer;

	alice_Color ui_colors[ALICE_UICOLOR_COUNT];
	float ui_cfg[ALICE_UICFG_COUNT];

	alice_UIWindow* windows;
	u32 window_count;
	u32 window_capacity;

	alice_UIElement* hovered_element;
	alice_UIElement* active_input;

	bool z_order_changed;

	float text_size;

	void* user_pointer;

	alice_Texture* gizmo_textures[ALICE_GIZMOTEXTURE_COUNT];
	alice_Shader* gizmo_shader;
	alice_VertexBuffer* gizmo_quad;
};

ALICE_API alice_UIContext* alice_new_ui_context(alice_Shader* rect_shader,
		alice_Shader* text_shader,
		alice_Shader* gizmo_shader,
		alice_Resource* font_data, float font_size);
ALICE_API void alice_apply_default_ui_config(alice_UIContext* context);
ALICE_API void alice_free_ui_context(alice_UIContext* context);

ALICE_API void alice_draw_ui(alice_UIContext* context);
ALICE_API void alice_draw_scene_gizmos(alice_UIContext* context, alice_Scene* scene);

typedef void (*alice_WindowCreateFunction)(alice_UIContext*, alice_UIWindow*);

ALICE_API alice_UIWindow* alice_new_ui_window(alice_UIContext* context,
		alice_WindowCreateFunction create_function);
ALICE_API void alice_destroy_ui_window(alice_UIContext* context, alice_UIWindow* window);

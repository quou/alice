#pragma once

#include "alice/core.h"

typedef struct GLFWwindow GLFWwindow;

typedef struct alice_Application {
	const char* name;
	u32 width;
	u32 height;

	double timestep;
	double last, now;

	GLFWwindow* window;
} alice_Application;

typedef struct alice_ApplicationConfig {
	const char* name;
	const char* splash_image;
	const char* splash_shader;
	u32 width;
	u32 height;
	bool fullscreen;
} alice_ApplicationConfig;

ALICE_API void alice_init_application(alice_ApplicationConfig cfg);
ALICE_API void alice_free_application();

ALICE_API alice_Application* alice_get_application();

ALICE_API void alice_update_events();
ALICE_API void alice_update_application();
ALICE_API bool alice_is_application_running();
ALICE_API void alice_quit_application();
ALICE_API void alice_cancel_application_quit();

ALICE_API void alice_resize_application(u32 new_width, u32 new_height);
ALICE_API void alice_rename_application(const char* new_name);
ALICE_API void alice_set_application_fullscreen(u32 monitor_index, bool fullscreen);

ALICE_API void alice_hide_mouse();
ALICE_API void alice_show_mouse();

ALICE_API i32 alice_random_int(i32 min, i32 max);

ALICE_API double alice_get_timestep();

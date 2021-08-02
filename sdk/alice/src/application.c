#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>

#include "alice/application.h"
#include "alice/input.h"
#include "alice/maths.h"
#include "alice/graphics.h"

static void APIENTRY gl_debug_callback(u32 source, u32 type, u32 id,
	u32 severity, i32 length, const char* message, const void* up) {

	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) {
		return;
	}

	const char* s;
	const char* t;

	switch (source) {
		case GL_DEBUG_SOURCE_API: s = "API"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM: s = "window system"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: s = "shader compiler"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY: s = "third party"; break;
		case GL_DEBUG_SOURCE_APPLICATION: s = "application"; break;
		case GL_DEBUG_SOURCE_OTHER: s = "other"; break;
	}

	switch (type) {
		case GL_DEBUG_TYPE_ERROR: t = "type error"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: t = "deprecated behaviour"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: t = "undefined behaviour"; break;
		case GL_DEBUG_TYPE_PORTABILITY: t = "portability"; break;
		case GL_DEBUG_TYPE_PERFORMANCE: t = "performance"; break;
		case GL_DEBUG_TYPE_MARKER: t = "marker"; break;
		case GL_DEBUG_TYPE_PUSH_GROUP: t = "push group"; break;
		case GL_DEBUG_TYPE_POP_GROUP: t = "pop group"; break;
		case GL_DEBUG_TYPE_OTHER: t = "other"; break;
	}

	switch (severity) {
		case GL_DEBUG_SEVERITY_HIGH:
		case GL_DEBUG_SEVERITY_MEDIUM:
			alice_log_error("OpenGL (source: %s; type: %s): %s", s, t, message);
			break;
		case GL_DEBUG_SEVERITY_LOW:
			alice_log_warning("OpenGL (source: %s; type: %s): %s", s, t, message);
			break;
		case GL_DEBUG_SEVERITY_NOTIFICATION:
			alice_log("OpenGL (source: %s; type: %s): %s", s, t, message);
			break;
	}
}

static void framebuffer_size_callback(GLFWwindow* window, i32 width, i32 height) {
	alice_Application* app = glfwGetWindowUserPointer(window);

	app->width = width;
	app->height = height;

	glViewport(0, 0, width, height);
}

static void key_callback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods) {
	alice_set_key(key, action);
}

static void mouse_button_callback(GLFWwindow* window, i32 button, i32 action, i32 mods) {
	alice_set_mouse_button(button, action);
}

static void mouse_move_callback(GLFWwindow* window, double x, double y) {
	alice_set_mouse_position((alice_v2i){(i32)x, (i32)y});
}

alice_Application app;

alice_Application* alice_get_application() {
	return &app;
}

void alice_init_application(alice_ApplicationConfig cfg) {
	srand((u32)time(alice_null));

	app.name = cfg.name;
	app.width = cfg.width;
	app.height = cfg.height;

	app.timestep = 0.0;
	app.now = 0.0;
	app.last = 0.0;

	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	app.window = glfwCreateWindow(app.width, app.height, app.name,
		alice_null, alice_null);

	if (cfg.fullscreen) {
		alice_set_application_fullscreen(0, true);
	}

	if (!app.window) {
		alice_log_error("Failed to create window");
		abort();
	}

	glfwSetWindowUserPointer(app.window, &app);
	glfwSetFramebufferSizeCallback(app.window, framebuffer_size_callback);
	glfwSetKeyCallback(app.window, key_callback);
	glfwSetMouseButtonCallback(app.window, mouse_button_callback);
	glfwSetCursorPosCallback(app.window, mouse_move_callback);

	glfwMakeContextCurrent(app.window);
	glfwSwapInterval(0);

	gladLoadGL();

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(gl_debug_callback, NULL);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);

	alice_init_input();

	if (cfg.splash_image && cfg.splash_shader) {
		alice_Texture* splash_texture = alice_load_texture(cfg.splash_image, ALICE_TEXTURE_ANTIALIASED);

		alice_Shader* shader = alice_load_shader(cfg.splash_shader);

		float verts[] = {
			/* position     UV */
			 0.5f,  0.5f, 	1.0f, 0.0f,
			 0.5f, -0.5f, 	1.0f, 1.0f,
			-0.5f, -0.5f, 	0.0f, 1.0f,
			-0.5f,  0.5f, 	0.0f, 0.0f
		};

		u32 indices[] = {
			0, 1, 3,
			1, 2, 3
		};

		alice_VertexBuffer* quad = alice_new_vertex_buffer(
				ALICE_VERTEXBUFFER_STATIC_DRAW |
				ALICE_VERTEXBUFFER_DRAW_TRIANGLES);

		alice_bind_vertex_buffer_for_edit(quad);
		alice_push_vertices(quad, verts, sizeof(verts) / sizeof(float));
		alice_push_indices(quad, indices, sizeof(indices) / sizeof(u32));
		alice_configure_vertex_buffer(quad, 0, 2, 4, 0);
		alice_configure_vertex_buffer(quad, 1, 2, 4, 2);

		alice_m4f projection = alice_m4f_ortho(-((float)cfg.width / 2.0f), (float)cfg.width / 2.0f,
			(float)cfg.height / 2.0f, -((float)cfg.height / 2.0f), -1.0f, 1.0f);
		alice_m4f model = alice_m4f_scale(
			alice_m4f_identity(),
			(alice_v3f) {splash_texture->width / 2.0f, splash_texture->height / 2.0f, 0.0f});

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		alice_render_clear();
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

		alice_bind_shader(shader);
		alice_bind_texture(splash_texture, 0);
		alice_shader_set_int(shader, "image", 0);
		alice_shader_set_m4f(shader, "transform", model);
		alice_shader_set_m4f(shader, "projection", projection);

		alice_bind_vertex_buffer_for_draw(quad);
		alice_draw_vertex_buffer(quad);

		alice_update_application();

		alice_free_vertex_buffer(quad);
	}
}

void alice_update_events() {
	alice_reset_input();
	glfwPollEvents();
}

void alice_update_application() {
	glfwSwapBuffers(app.window);

	app.now = glfwGetTime();
	app.timestep = app.now - app.last;
	app.last = app.now;
}

bool alice_is_application_running() {
	return !glfwWindowShouldClose(app.window);
}

void alice_quit_application() {
	glfwSetWindowShouldClose(app.window, true);
}

void alice_cancel_application_quit() {
	glfwSetWindowShouldClose(app.window, false);
}

void alice_free_application() {
	glfwDestroyWindow(app.window);
	glfwTerminate();
}

void alice_resize_application(u32 new_width, u32 new_height) {
	app.width = new_width;
	app.height = new_height;

	glfwSetWindowSize(app.window, new_width, new_height);
}

void alice_rename_application(const char* new_name) {
	app.name = new_name;

	glfwSetWindowTitle(app.window, new_name);
}

void alice_set_application_fullscreen(u32 monitor_index, bool fullscreen) {
	if (fullscreen) {
		i32 monitor_count;
		GLFWmonitor** monitors = glfwGetMonitors(&monitor_count);

		if (monitor_index >= (u32)monitor_count) {
			monitor_index = 0;
		}

		GLFWmonitor* monitor = monitors[monitor_index];

		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwSetWindowMonitor(app.window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);

		app.width = mode->width;
		app.height = mode->height;
	} else {
		glfwSetWindowMonitor(app.window, alice_null, 100, 100, app.width, app.height, 0);
	}
}

void alice_hide_mouse() {
	glfwSetInputMode(app.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void alice_show_mouse() {
	glfwSetInputMode(app.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

i32 alice_random_int(i32 min, i32 max) {
	i32 r = 0;
	i32 l = 0;
	i32 h = 0;

	if (min < max) {
		l = min;
		h = max + 1;
	} else {
		l = max + 1;
		h = min;
	}

	return (rand() % (h - l)) + l;
}

double alice_get_timestep() {
	return app.timestep;
}

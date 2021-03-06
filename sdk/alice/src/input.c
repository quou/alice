#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <GLFW/glfw3.h>

#include "alice/input.h"

typedef struct alice_Input {
	bool held_keys[ALICE_KEY_COUNT];
	bool pressed_keys[ALICE_KEY_COUNT];
	bool released_keys[ALICE_KEY_COUNT];

	bool held_mouse_buttons[ALICE_MOUSE_BUTTON_COUNT];
	bool pressed_mouse_buttons[ALICE_MOUSE_BUTTON_COUNT];
	bool released_mouse_buttons[ALICE_MOUSE_BUTTON_COUNT];

	bool scrolled;
	alice_v2f_t scroll_offset;

	bool mouse_moved;
	alice_v2i_t mouse_position;

	char text_input[32];
} alice_Input;

alice_Input input;

void alice_init_input() {
	memset(input.held_keys, 0, ALICE_KEY_COUNT * sizeof(bool));
	memset(input.pressed_keys, 0, ALICE_KEY_COUNT * sizeof(bool));
	memset(input.released_keys, 0, ALICE_KEY_COUNT * sizeof(bool));

	memset(input.held_mouse_buttons, 0, ALICE_MOUSE_BUTTON_COUNT * sizeof(bool));
	memset(input.pressed_mouse_buttons, 0, ALICE_MOUSE_BUTTON_COUNT * sizeof(bool));
	memset(input.released_mouse_buttons, 0, ALICE_MOUSE_BUTTON_COUNT * sizeof(bool));

	input.scrolled = false;
	input.mouse_moved = false;
	input.text_input[0] = '\0';
}

void alice_reset_input() {
	memset(input.pressed_keys, 0, ALICE_KEY_COUNT * sizeof(bool));
	memset(input.released_keys, 0, ALICE_KEY_COUNT * sizeof(bool));

	memset(input.pressed_mouse_buttons, 0, ALICE_MOUSE_BUTTON_COUNT * sizeof(bool));
	memset(input.released_mouse_buttons, 0, ALICE_MOUSE_BUTTON_COUNT * sizeof(bool));

	input.scrolled = false;
	input.mouse_moved = false;
	input.text_input[0] = '\0';
}

void alice_set_key(i32 key, i32 action) {
	input.held_keys[key] = action == GLFW_PRESS || action == GLFW_REPEAT;
	input.pressed_keys[key] = action == GLFW_PRESS;
	input.released_keys[key] = action == GLFW_RELEASE;
}

void alice_set_mouse_button(i32 button, i32 action) {
	input.held_mouse_buttons[button] = action == GLFW_PRESS || action == GLFW_REPEAT;
	input.pressed_mouse_buttons[button] = action == GLFW_PRESS;
	input.released_mouse_buttons[button] = action == GLFW_RELEASE;
}

void alice_set_mouse_moved(bool moved) {
	input.mouse_moved = moved;
}

void alice_set_scrolled(bool scrolled) {
	input.scrolled = scrolled;
}

void alice_set_mouse_position(alice_v2i_t pos) {
	input.mouse_position = pos;
}

void alice_set_scroll_offset(alice_v2f_t offset) {
	input.scroll_offset = offset;
}

const char* alice_get_text_input() {
	return input.text_input;
}

void alice_add_input_character(char character) {
	u32 index = 0;
	
	for (const char* c = input.text_input; *c; c++) {
		index++;
	}

	input.text_input[index] = character;
}

bool alice_key_pressed(i32 key) {
	return input.held_keys[key];
}

bool alice_key_just_pressed(i32 key) {
	return input.pressed_keys[key];
}

bool alice_key_just_released(i32 key) {
	return input.released_keys[key];
}

bool alice_mouse_button_pressed(i32 button) {
	return input.held_mouse_buttons[button];
}

bool alice_mouse_button_just_pressed(i32 button) {
	return input.pressed_mouse_buttons[button];
}

bool alice_mouse_button_just_released(i32 button) {
	return input.released_mouse_buttons[button];
}

bool alice_scrolled() {
	return input.scrolled;
}

bool alice_mouse_moved() {
	return input.mouse_moved;
}

alice_v2i_t alice_get_mouse_position() {
	return input.mouse_position;
}

alice_v2f_t alice_get_scroll_offset() {
	return input.scroll_offset;
}

#pragma once

#include "alice/core.h"
#include "alice/maths.h"

#define ALICE_KEY_UNKNOWN            -1

#define ALICE_KEY_SPACE              32
#define ALICE_KEY_APOSTROPHE         39  /* ' */
#define ALICE_KEY_COMMA              44  /* , */
#define ALICE_KEY_MINUS              45  /* - */
#define ALICE_KEY_PERIOD             46  /* . */
#define ALICE_KEY_SLASH              47  /* / */
#define ALICE_KEY_0                  48
#define ALICE_KEY_1                  49
#define ALICE_KEY_2                  50
#define ALICE_KEY_3                  51
#define ALICE_KEY_4                  52
#define ALICE_KEY_5                  53
#define ALICE_KEY_6                  54
#define ALICE_KEY_7                  55
#define ALICE_KEY_8                  56
#define ALICE_KEY_9                  57
#define ALICE_KEY_SEMICOLON          59  /* ; */
#define ALICE_KEY_EQUAL              61  /* = */
#define ALICE_KEY_A                  65
#define ALICE_KEY_B                  66
#define ALICE_KEY_C                  67
#define ALICE_KEY_D                  68
#define ALICE_KEY_E                  69
#define ALICE_KEY_F                  70
#define ALICE_KEY_G                  71
#define ALICE_KEY_H                  72
#define ALICE_KEY_I                  73
#define ALICE_KEY_J                  74
#define ALICE_KEY_K                  75
#define ALICE_KEY_L                  76
#define ALICE_KEY_M                  77
#define ALICE_KEY_N                  78
#define ALICE_KEY_O                  79
#define ALICE_KEY_P                  80
#define ALICE_KEY_Q                  81
#define ALICE_KEY_R                  82
#define ALICE_KEY_S                  83
#define ALICE_KEY_T                  84
#define ALICE_KEY_U                  85
#define ALICE_KEY_V                  86
#define ALICE_KEY_W                  87
#define ALICE_KEY_X                  88
#define ALICE_KEY_Y                  89
#define ALICE_KEY_Z                  90
#define ALICE_KEY_LEFT_BRACKET       91  /* [ */
#define ALICE_KEY_BACKSLASH          92  /* \ */
#define ALICE_KEY_RIGHT_BRACKET      93  /* ] */
#define ALICE_KEY_GRAVE_ACCENT       96  /* ` */
#define ALICE_KEY_WORLD_1            161 /* non-US #1 */
#define ALICE_KEY_WORLD_2            162 /* non-US #2 */

/* Function keys */
#define ALICE_KEY_ESCAPE             256
#define ALICE_KEY_ENTER              257
#define ALICE_KEY_TAB                258
#define ALICE_KEY_BACKSPACE          259
#define ALICE_KEY_INSERT             260
#define ALICE_KEY_DELETE             261
#define ALICE_KEY_RIGHT              262
#define ALICE_KEY_LEFT               263
#define ALICE_KEY_DOWN               264
#define ALICE_KEY_UP                 265
#define ALICE_KEY_PAGE_UP            266
#define ALICE_KEY_PAGE_DOWN          267
#define ALICE_KEY_HOME               268
#define ALICE_KEY_END                269
#define ALICE_KEY_CAPS_LOCK          280
#define ALICE_KEY_SCROLL_LOCK        281
#define ALICE_KEY_NUM_LOCK           282
#define ALICE_KEY_PRINT_SCREEN       283
#define ALICE_KEY_PAUSE              284
#define ALICE_KEY_F1                 290
#define ALICE_KEY_F2                 291
#define ALICE_KEY_F3                 292
#define ALICE_KEY_F4                 293
#define ALICE_KEY_F5                 294
#define ALICE_KEY_F6                 295
#define ALICE_KEY_F7                 296
#define ALICE_KEY_F8                 297
#define ALICE_KEY_F9                 298
#define ALICE_KEY_F10                299
#define ALICE_KEY_F11                300
#define ALICE_KEY_F12                301
#define ALICE_KEY_F13                302
#define ALICE_KEY_F14                303
#define ALICE_KEY_F15                304
#define ALICE_KEY_F16                305
#define ALICE_KEY_F17                306
#define ALICE_KEY_F18                307
#define ALICE_KEY_F19                308
#define ALICE_KEY_F20                309
#define ALICE_KEY_F21                310
#define ALICE_KEY_F22                311
#define ALICE_KEY_F23                312
#define ALICE_KEY_F24                313
#define ALICE_KEY_F25                314
#define ALICE_KEY_KP_0               320
#define ALICE_KEY_KP_1               321
#define ALICE_KEY_KP_2               322
#define ALICE_KEY_KP_3               323
#define ALICE_KEY_KP_4               324
#define ALICE_KEY_KP_5               325
#define ALICE_KEY_KP_6               326
#define ALICE_KEY_KP_7               327
#define ALICE_KEY_KP_8               328
#define ALICE_KEY_KP_9               329
#define ALICE_KEY_KP_DECIMAL         330
#define ALICE_KEY_KP_DIVIDE          331
#define ALICE_KEY_KP_MULTIPLY        332
#define ALICE_KEY_KP_SUBTRACT        333
#define ALICE_KEY_KP_ADD             334
#define ALICE_KEY_KP_ENTER           335
#define ALICE_KEY_KP_EQUAL           336
#define ALICE_KEY_LEFT_SHIFT         340
#define ALICE_KEY_LEFT_CONTROL       341
#define ALICE_KEY_LEFT_ALT           342
#define ALICE_KEY_LEFT_SUPER         343
#define ALICE_KEY_RIGHT_SHIFT        344
#define ALICE_KEY_RIGHT_CONTROL      345
#define ALICE_KEY_RIGHT_ALT          346
#define ALICE_KEY_RIGHT_SUPER        347
#define ALICE_KEY_MENU               348

#define ALICE_KEY_COUNT        ALICE_KEY_MENU + 1

#define ALICE_MOD_SHIFT           0x0001
#define ALICE_MOD_CONTROL         0x0002
#define ALICE_MOD_ALT             0x0004
#define ALICE_MOD_SUPER           0x0008
#define ALICE_MOD_CAPS_LOCK       0x0010
#define ALICE_MOD_NUM_LOCK        0x0020

#define ALICE_MOUSE_BUTTON_1         0
#define ALICE_MOUSE_BUTTON_2         1
#define ALICE_MOUSE_BUTTON_3         2
#define ALICE_MOUSE_BUTTON_4         3
#define ALICE_MOUSE_BUTTON_5         4
#define ALICE_MOUSE_BUTTON_6         5
#define ALICE_MOUSE_BUTTON_7         6
#define ALICE_MOUSE_BUTTON_8         7
#define ALICE_MOUSE_BUTTON_COUNT     ALICE_MOUSE_BUTTON_8 + 1
#define ALICE_MOUSE_BUTTON_LEFT      ALICE_MOUSE_BUTTON_1
#define ALICE_MOUSE_BUTTON_RIGHT     ALICE_MOUSE_BUTTON_2
#define ALICE_MOUSE_BUTTON_MIDDLE    ALICE_MOUSE_BUTTON_3

ALICE_API void alice_init_input();
ALICE_API void alice_reset_input();

ALICE_API void alice_set_key(i32 key, i32 action);
ALICE_API void alice_set_mouse_button(i32 button, i32 action);
ALICE_API void alice_set_mouse_moved(bool moved);
ALICE_API void alice_set_mouse_position(alice_v2i_t position);
ALICE_API void alice_set_scrolled(bool scrolled);
ALICE_API void alice_set_scroll_offset(alice_v2f_t offset);

ALICE_API const char* alice_get_text_input();
ALICE_API void alice_add_input_character(char character);

ALICE_API bool alice_key_pressed(i32 key);
ALICE_API bool alice_key_just_pressed(i32 key);
ALICE_API bool alice_key_just_released(i32 key);

ALICE_API bool alice_mouse_button_pressed(i32 button);
ALICE_API bool alice_mouse_button_just_pressed(i32 button);
ALICE_API bool alice_mouse_button_just_released(i32 button);

ALICE_API bool alice_scrolled();
ALICE_API bool alice_mouse_moved();

ALICE_API alice_v2i_t alice_get_mouse_position();
ALICE_API alice_v2f_t alice_get_scroll_offset();

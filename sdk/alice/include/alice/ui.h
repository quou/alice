#pragma once

#include "alice/core.h"
#include "alice/maths.h"
#include "alice/resource.h"
#include "alice/graphics.h"

typedef struct mu_Context mu_Context;

ALICE_API void alice_init_microui_renderer(alice_shader_t* shader);
ALICE_API void alice_deinit_microui_renderer();
ALICE_API i32 alice_microui_text_width(mu_Font font, const char* test, i32 len);
ALICE_API i32 alice_microui_text_height(mu_Font font);
ALICE_API void alice_update_microui(mu_Context* context);
ALICE_API void alice_render_microui(mu_Context* context, u32 width, u32 height);

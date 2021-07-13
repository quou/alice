#pragma once

#include "alice/core.h"
#include "alice/maths.h"
#include "alice/resource.h"

typedef struct alice_TextRenderer alice_TextRenderer;

ALICE_API alice_TextRenderer* alice_new_text_renderer(alice_Resource* font_data,
		float font_size, alice_Shader* font_shader);
ALICE_API void alice_free_text_renderer(alice_TextRenderer* renderer);

ALICE_API void alice_set_text_renderer_dimentions(alice_TextRenderer* renderer, alice_v2f dimentions);
ALICE_API alice_render_text(alice_TextRenderer* renderer, alice_v2f position, const char* string);

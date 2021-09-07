#pragma once

#include "stb_truetype.h"

#define ALICE_MAX_GLYPHSET 256

typedef struct alice_glyph_set_t {
	alice_texture_t* atlas;
	stbtt_bakedchar glyphs[256];
} alice_glyph_set_t;

struct alice_font_t {
	void* data;
	stbtt_fontinfo info;
	alice_glyph_set_t* sets[ALICE_MAX_GLYPHSET];
	float size;
	i32 height;
};

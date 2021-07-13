#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <stb_truetype.h>

#include "alice/ui.h"
#include "alice/graphics.h"

struct alice_TextRenderer {
	alice_VertexBuffer* vb;
	alice_Texture* font_bitmap;
	alice_Shader* shader;

	float font_size;

	stbtt_packedchar char_data[96];
	stbtt_fontinfo font_info;

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

alice_render_text(alice_TextRenderer* renderer, alice_v2f position, const char* string) {
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
				index_offset + 0, index_offset + 1, index_offset + 3,
				index_offset + 1, index_offset + 2, index_offset + 3
			};

			alice_update_vertices(renderer->vb, verts, quad_count * 4 * 4, 4 * 4);
			alice_update_indices(renderer->vb, indices, quad_count * 6, 6);

			quad_count++;
		}

		string++;
	}

	alice_bind_vertex_buffer_for_edit(alice_null);
	alice_bind_shader(renderer->shader);

	alice_m4f projection = alice_m4f_ortho(0.0, renderer->dimentions.x,
			renderer->dimentions.y, 0.0, -1.0, 1.0);

	alice_shader_set_color(renderer->shader, "color_mod", ALICE_COLOR_WHITE);
	alice_shader_set_m4f(renderer->shader, "camera", projection);
	
	alice_bind_texture(renderer->font_bitmap, 0);
	alice_shader_set_int(renderer->shader, "font", 0);

	alice_bind_vertex_buffer_for_draw(renderer->vb);
	alice_draw_vertex_buffer_custom_count(renderer->vb, quad_count * 6);
	alice_bind_vertex_buffer_for_draw(alice_null);

	alice_bind_texture(alice_null, 0);
	alice_bind_shader(alice_null);
}

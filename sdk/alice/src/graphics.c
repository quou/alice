#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <glad/glad.h>
#include <stb_image.h>

#include "alice/graphics.h"
#include "alice/physics.h"

alice_Color alice_color_from_rgb_color(alice_RGBColor rgb) {
	i8 r = (i8)(rgb.r * 255.0);
	i8 g = (i8)(rgb.g * 255.0);
	i8 b = (i8)(rgb.b * 255.0);

	return ((r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff);
}

alice_RGBColor alice_rgb_color_from_color(alice_Color color) {
	int r = (color >> 16) & 0xFF;
	int g = (color >> 8) & 0xFF;
	int b = color & 0xFF;

	float rf = (float)r / 255.0f;
	float gf = (float)g / 255.0f;
	float bf = (float)b / 255.0f;

	return (alice_RGBColor) { rf, gf, bf };
}

alice_RenderTarget* alice_new_render_target(u32 width, u32 height, u32 color_attachment_count) {
	alice_RenderTarget* target = malloc(sizeof(alice_RenderTarget));

	target->width = width;
	target->height = height;
	target->old_width = width;
	target->old_height = height;

	target->color_attachment_count = color_attachment_count;
	target->color_attachments = malloc(color_attachment_count * sizeof(u32));

	glGenFramebuffers(1, &target->frame_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, target->frame_buffer);

	glGenTextures(color_attachment_count, target->color_attachments);
	for (u32 i = 0; i < color_attachment_count; i++) {
		glBindTexture(GL_TEXTURE_2D, target->color_attachments[i]);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA,
			GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D,
			target->color_attachments[i], 0);
	}

	glGenRenderbuffers(1, &target->render_buffer);
	glBindRenderbuffer(GL_RENDERBUFFER, target->render_buffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
		GL_RENDERBUFFER, target->render_buffer);

	alice_unbind_render_target(target);

	return target;
}

void alice_free_render_target(alice_RenderTarget* target) {
	assert(target);

	glDeleteRenderbuffers(1, &target->render_buffer);
	glDeleteFramebuffers(1, &target->frame_buffer);

	free(target);
}

void alice_bind_render_target(alice_RenderTarget* target, u32 old_width, u32 old_height) {
	assert(target);

	target->old_width = old_width;
	target->old_height = old_height;

	glBindFramebuffer(GL_FRAMEBUFFER, target->frame_buffer);
	glBindRenderbuffer(GL_RENDERBUFFER, target->render_buffer);
	glViewport(0, 0, target->width, target->height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void alice_unbind_render_target(alice_RenderTarget* target) {
	assert(target);

	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0, 0, target->old_width, target->old_height);
}

void alice_resize_render_target(alice_RenderTarget* target, u32 width, u32 height) {
	assert(target);

	if (target->width == width && target->height == height) {
		return;
	}

	target->width = width;
	target->height = height;

	glBindFramebuffer(GL_FRAMEBUFFER, target->frame_buffer);
	for (u32 i = 0; i < target->color_attachment_count; i++) {
		glBindTexture(GL_TEXTURE_2D, target->color_attachments[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA,
			GL_FLOAT, NULL);
	}

	glBindRenderbuffer(GL_RENDERBUFFER, target->render_buffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
		GL_RENDERBUFFER, target->render_buffer);

	alice_unbind_render_target(target);
}

void alice_render_target_bind_output(alice_RenderTarget* target, u32 attachment_index, u32 unit) {
	assert(target);
	assert(attachment_index < target->color_attachment_count);

	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, target->color_attachments[attachment_index]);
}

alice_VertexBuffer* alice_new_vertex_buffer(alice_VertexBufferFlags flags) {
	alice_VertexBuffer* buffer = malloc(sizeof(alice_VertexBuffer));

	buffer->flags = flags;

	glGenVertexArrays(1, &buffer->va_id);
	glGenBuffers(1, &buffer->vb_id);
	glGenBuffers(1, &buffer->ib_id);

	return buffer;
}

void alice_free_vertex_buffer(alice_VertexBuffer* buffer) {
	assert(buffer);

	glDeleteVertexArrays(1, &buffer->va_id);
	glDeleteBuffers(1, &buffer->vb_id);
	glDeleteBuffers(1, &buffer->ib_id);

	free(buffer);
}

void alice_bind_vertex_buffer_for_draw(alice_VertexBuffer* buffer) {
	glBindVertexArray(buffer ? buffer->va_id : 0);
}

void alice_bind_vertex_buffer_for_edit(alice_VertexBuffer* buffer) {
	if (!buffer) {
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	else {
		glBindVertexArray(buffer->va_id);
		glBindBuffer(GL_ARRAY_BUFFER, buffer->vb_id);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->ib_id);
	}
}

void alice_push_vertices(alice_VertexBuffer* buffer, float* vertices, u32 count) {
	assert(buffer);

	u32 mode = GL_STATIC_DRAW;
	if (buffer->flags & ALICE_VERTEXBUFFER_DYNAMIC_DRAW) {
		mode = GL_DYNAMIC_DRAW;
	}

	glBufferData(GL_ARRAY_BUFFER, count * sizeof(float), vertices, mode);
}

void alice_push_indices(alice_VertexBuffer* buffer, u32* indices, u32 count) {
	assert(buffer);

	u32 mode = GL_STATIC_DRAW;
	if (buffer->flags & ALICE_VERTEXBUFFER_DYNAMIC_DRAW) {
		mode = GL_DYNAMIC_DRAW;
	}

	buffer->index_count = count;

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(u32), indices, mode);
}

void alice_update_vertices(alice_VertexBuffer* buffer, float* vertices, u32 offset, u32 count) {
	assert(buffer);

	glBufferSubData(GL_ARRAY_BUFFER, offset * sizeof(float),
		count * sizeof(float), vertices);
}

void alice_update_indices(alice_VertexBuffer* buffer, u32* indices, u32 offset, u32 count) {
	assert(buffer);

	buffer->index_count = count;

	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset * sizeof(u32),
		count * sizeof(u32), indices);
}

void alice_configure_vertex_buffer(alice_VertexBuffer* buffer, u32 index, u32 component_count,
	u32 stride, u32 offset) {
	assert(buffer);

	glVertexAttribPointer(index, component_count, GL_FLOAT, GL_FALSE,
		stride * sizeof(float), (void*)(u64)(offset * sizeof(float)));
	glEnableVertexAttribArray(index);
}

void alice_draw_vertex_buffer(alice_VertexBuffer* buffer) {
	u32 draw_type = GL_TRIANGLES;
	if (buffer->flags & ALICE_VERTEXBUFFER_DRAW_LINES) {
		draw_type = GL_LINES;
	}
	else if (buffer->flags & ALICE_VERTEXBUFFER_DRAW_LINE_STRIP) {
		draw_type = GL_LINE_STRIP;
	}

	glDrawElements(draw_type, buffer->index_count, GL_UNSIGNED_INT, 0);
}

void alice_draw_vertex_buffer_custom_count(alice_VertexBuffer* buffer, u32 count) {
	u32 draw_type = GL_TRIANGLES;
	if (buffer->flags & ALICE_VERTEXBUFFER_DRAW_LINES) {
		draw_type = GL_LINES;
	}
	else if (buffer->flags & ALICE_VERTEXBUFFER_DRAW_LINE_STRIP) {
		draw_type = GL_LINE_STRIP;
	}

	glDrawElements(draw_type, count, GL_UNSIGNED_INT, 0);
}

alice_Shader* alice_init_shader(alice_Shader* shader, char* source) {
	assert(shader);

	shader->panic_mode = false;

	const u32 source_len = strlen(source);

	char* vertex_source = malloc(source_len);
	char* fragment_source = malloc(source_len);
	char* geometry_source = malloc(source_len);
	memset(vertex_source, 0, source_len);
	memset(fragment_source, 0, source_len);
	memset(geometry_source, 0, source_len);

	bool has_geometry = false;

	u32 count = 0;
	i32 adding_to = -1;
	for (char* current = source; *current != '\0'; current++) {
		if (*current == '\n' && *(current + 1) != '\0') {
			i32 minus = 1;

			current++;

			char* line = current - count - minus;
			line[count] = '\0';

			if (strstr(line, "#begin VERTEX")) {
				adding_to = 0;
			}
			else if (strstr(line, "#begin FRAGMENT")) {
				adding_to = 1;
			}
			else if (strstr(line, "#begin GEOMETRY")) {
				adding_to = 2;
				has_geometry = true;
			}
			else if (strstr(line, "#end VERTEX") ||
				strstr(line, "#end FRAGMENT") ||
				strstr(line, "#end GEOMETRY")) {
				adding_to = -1;
			}
			else if (adding_to == 0) {
				strcat(vertex_source, line);
				strcat(vertex_source, "\n");
			}
			else if (adding_to == 1) {
				strcat(fragment_source, line);
				strcat(fragment_source, "\n");
			}
			else if (adding_to == 2) {
				strcat(geometry_source, line);
				strcat(geometry_source, "\n");
			}

			count = 0;
		}

		count++;
	}

	const char* vsp = vertex_source;
	const char* fsp = fragment_source;
	const char* gsp = geometry_source;

	i32 success;
	u32 v, f, g;
	v = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(v, 1, &vsp, NULL);
	glCompileShader(v);

	glGetShaderiv(v, GL_COMPILE_STATUS, &success);
	if (!success) {
		char info_log[1024];
		glGetShaderInfoLog(v, 1024, alice_null, info_log);
		alice_log_error("%s", info_log);
		shader->panic_mode = true;
	}

	f = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(f, 1, &fsp, NULL);
	glCompileShader(f);

	glGetShaderiv(f, GL_COMPILE_STATUS, &success);
	if (!success) {
		char info_log[1024];
		glGetShaderInfoLog(f, 1024, alice_null, info_log);
		alice_log_error("%s", info_log);
		shader->panic_mode = true;
	}

	if (has_geometry) {
		g = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(g, 1, &gsp, NULL);
		glCompileShader(g);

		glGetShaderiv(g, GL_COMPILE_STATUS, &success);
		if (!success) {
			char info_log[1024];
			glGetShaderInfoLog(g, 1024, alice_null, info_log);
			alice_log_error("%s", info_log);
			shader->panic_mode = true;
		}
	}

	u32 id;
	id = glCreateProgram();
	glAttachShader(id, v);
	glAttachShader(id, f);
	if (has_geometry) {
		glAttachShader(id, g);
	}
	glLinkProgram(id);

	glGetProgramiv(id, GL_LINK_STATUS, &success);
	if (!success) {
		shader->panic_mode = true;
	}

	glDeleteShader(v);
	glDeleteShader(f);

	if (has_geometry) {
		glDeleteShader(g);
	}

	free(vertex_source);
	free(fragment_source);
	free(geometry_source);

	shader->id = id;
	return shader;
}

void alice_deinit_shader(alice_Shader* shader) {
	assert(shader);

	if (shader->panic_mode) { return; };

	glDeleteProgram(shader->id);
}

alice_Shader* alice_new_shader(alice_Resource* resource) {
	assert(resource);

	if (resource->type != ALICE_RESOURCE_STRING || resource->payload_size <= 0) {
		alice_log_error("Resource insuffucient for shader creation.");
		return NULL;
	}

	alice_Shader* s = malloc(sizeof(alice_Shader));
	alice_init_shader(s, resource->payload);

	return s;
}

void alice_free_shader(alice_Shader* shader) {
	assert(shader);
	alice_deinit_shader(shader);

	free(shader);
}

void alice_bind_shader(alice_Shader* shader) {
	if (!shader || shader->panic_mode) { return; };

	glUseProgram(shader ? shader->id : 0);
}

void alice_shader_set_int(alice_Shader* shader, const char* name, i32 v) {
	assert(shader);

	if (shader->panic_mode) { return; };

	u32 location = glGetUniformLocation(shader->id, name);
	glUniform1i(location, v);
}

void alice_shader_set_uint(alice_Shader* shader, const char* name, u32 v) {
	assert(shader);

	if (shader->panic_mode) { return; };

	u32 location = glGetUniformLocation(shader->id, name);
	glUniform1ui(location, v);
}

void alice_shader_set_float(alice_Shader* shader, const char* name, float v) {
	assert(shader);

	if (shader->panic_mode) { return; };

	u32 location = glGetUniformLocation(shader->id, name);
	glUniform1f(location, v);
}

void alice_shader_set_color(alice_Shader* shader, const char* name, alice_Color color) {
	assert(shader);

	alice_RGBColor rgb = alice_rgb_color_from_color(color);

	alice_shader_set_v3f(shader, name, (alice_v3f) { rgb.r, rgb.g, rgb.b });
}

void alice_shader_set_rgb_color(alice_Shader* shader, const char* name, alice_RGBColor color) {
	assert(shader);

	alice_shader_set_v3f(shader, name, (alice_v3f) { color.r, color.g, color.b });
}

void alice_shader_set_v2i(alice_Shader* shader, const char* name, alice_v2i v) {
	assert(shader);

	if (shader->panic_mode) { return; };

	u32 location = glGetUniformLocation(shader->id, name);
	glUniform2i(location, v.x, v.y);
}

void alice_shader_set_v2u(alice_Shader* shader, const char* name, alice_v2u v) {
	assert(shader);

	if (shader->panic_mode) { return; };

	u32 location = glGetUniformLocation(shader->id, name);
	glUniform2ui(location, v.x, v.y);
}

void alice_shader_set_v2f(alice_Shader* shader, const char* name, alice_v2f v) {
	assert(shader);

	if (shader->panic_mode) { return; };

	u32 location = glGetUniformLocation(shader->id, name);
	glUniform2f(location, v.x, v.y);
}

void alice_shader_set_v3i(alice_Shader* shader, const char* name, alice_v3i v) {
	assert(shader);

	if (shader->panic_mode) { return; };

	u32 location = glGetUniformLocation(shader->id, name);
	glUniform3i(location, v.x, v.y, v.z);
}

void alice_shader_set_v3u(alice_Shader* shader, const char* name, alice_v3u v) {
	assert(shader);

	if (shader->panic_mode) { return; };

	u32 location = glGetUniformLocation(shader->id, name);
	glUniform3ui(location, v.x, v.y, v.z);
}

void alice_shader_set_v3f(alice_Shader* shader, const char* name, alice_v3f v) {
	assert(shader);

	if (shader->panic_mode) { return; };

	u32 location = glGetUniformLocation(shader->id, name);
	glUniform3f(location, v.x, v.y, v.z);
}

void alice_shader_set_v4i(alice_Shader* shader, const char* name, alice_v4i v) {
	assert(shader);

	if (shader->panic_mode) { return; };

	u32 location = glGetUniformLocation(shader->id, name);
	glUniform4i(location, v.x, v.y, v.z, v.w);
}

void alice_shader_set_v4u(alice_Shader* shader, const char* name, alice_v4u v) {
	assert(shader);

	if (shader->panic_mode) { return; };

	u32 location = glGetUniformLocation(shader->id, name);
	glUniform4ui(location, v.x, v.y, v.z, v.w);
}

void alice_shader_set_v4f(alice_Shader* shader, const char* name, alice_v4f v) {
	assert(shader);

	if (shader->panic_mode) { return; };

	u32 location = glGetUniformLocation(shader->id, name);
	glUniform4f(location, v.x, v.y, v.z, v.w);
}

void alice_shader_set_m4f(alice_Shader* shader, const char* name, alice_m4f v) {
	assert(shader);

	if (shader->panic_mode) { return; };

	u32 location = glGetUniformLocation(shader->id, name);
	glUniformMatrix4fv(location, 1, GL_FALSE, (float*)v.elements);
}

alice_Texture* alice_new_texture(alice_Resource* resource, alice_TextureFlags flags) {
	assert(resource);

	alice_Texture* texture = malloc(sizeof(alice_Texture));
	alice_init_texture(texture, resource, flags);
	return texture;
}

alice_Texture* alice_new_texture_from_memory(void* data, u32 size, alice_TextureFlags flags) {
	alice_Texture* texture = malloc(sizeof(alice_Texture));
	alice_init_texture_from_memory(texture, data, size, flags);
	return texture;
}


alice_Texture* alice_new_texture_from_memory_uncompressed(unsigned char* pixels,
	u32 size, i32 width, i32 height, i32 component_count, alice_TextureFlags flags) {
	alice_Texture* texture = malloc(sizeof(alice_Texture));
	alice_init_texture_from_memory_uncompressed(texture, pixels, size, width, height, component_count, flags);
	return texture;
}

void alice_init_texture(alice_Texture* texture, alice_Resource* resource, alice_TextureFlags flags) {
	if (resource->payload == NULL || resource->type != ALICE_RESOURCE_BINARY) {
		alice_log_error("Resource insufficient for texture creation.");
		return;
	}

	alice_init_texture_from_memory(texture, resource->payload, resource->payload_size, flags);
}

void alice_init_texture_from_memory(alice_Texture* texture, void* data, u32 size, alice_TextureFlags flags) {
	if (data == NULL || size == 0) {
		alice_log_error("Data insufficient for texture creation.");
		return;
	}

	i32 width, height, component_count;

	u8* pixels = stbi_load_from_memory(
		data, size,
		&width, &height, &component_count, 0);

	if (pixels == NULL) {
		alice_log_error("Failed to load texture: %s", stbi_failure_reason());
		return;
	}

	alice_init_texture_from_memory_uncompressed(texture, pixels, size,
		width, height, component_count, flags);

	free(pixels);
}

void alice_init_texture_from_memory_uncompressed(alice_Texture* texture, unsigned char* pixels,
	u32 size, i32 width, i32 height, i32 component_count, alice_TextureFlags flags) {
	if (pixels == NULL || size == 0) {
		alice_log_error("Data insufficient for texture creation.");
		return;
	}

	texture->width = width;
	texture->height = height;
	texture->component_count = component_count;

	texture->flags = flags;

	glGenTextures(1, &texture->id);
	glBindTexture(GL_TEXTURE_2D, texture->id);

	u32 alias_mode = GL_LINEAR;
	if (flags & ALICE_TEXTURE_ALIASED) {
		alias_mode = GL_NEAREST;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, alias_mode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, alias_mode);

	u32 format = GL_SRGB;
	u32 internal_format = GL_RGB;
	if (texture->component_count == 4) {
		internal_format = GL_RGBA;
		format = GL_SRGB_ALPHA;
	}
	else if (texture->component_count == 1) {
		internal_format = GL_RED;
		format = GL_RED;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, format,
		width, height, 0, internal_format,
		GL_UNSIGNED_BYTE, pixels);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void alice_deinit_texture(alice_Texture* texture) {
	assert(texture);
	glDeleteTextures(1, &texture->id);
}

void alice_free_texture(alice_Texture* texture) {
	assert(texture);

	alice_deinit_texture(texture);

	free(texture);
}

void alice_bind_texture(alice_Texture* texture, u32 slot) {
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, texture ? texture->id : 0);
}

void alice_init_mesh(alice_Mesh* mesh, alice_VertexBuffer* vb) {
	assert(mesh);

	*mesh = (alice_Mesh){
		.transform = alice_m4f_identity(),

		.vb = vb
	};
}

void alice_deinit_mesh(alice_Mesh* mesh) {
	assert(mesh);

	alice_free_vertex_buffer(mesh->vb);
}

alice_Mesh alice_new_cube_mesh() {
	float verts[] = {
		 0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.625, 0.500,
		-0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.875, 0.500,
		-0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.875, 0.750,
		 0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.625, 0.500,
		-0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.875, 0.750,
		 0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.625, 0.750,
		 0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.375, 0.750,
		 0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.625, 0.750,
		-0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.625, 1.0f,
		 0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.375, 0.750,
		-0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.625, 1.0f,
		-0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.375, 1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 0.375, 0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 0.625, 0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.625, 0.250,
		-0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 0.375, 0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.625, 0.250,
		-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.375, 0.250,
		-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.125, 0.500,
		 0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.375, 0.500,
		 0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.375, 0.750,
		-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.125, 0.500,
		 0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.375, 0.750,
		-0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.125, 0.750,
		 0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.375, 0.500,
		 0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.625, 0.500,
		 0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.625, 0.750,
		 0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.375, 0.500,
		 0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.625, 0.750,
		 0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.375, 0.750,
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.375, 0.250,
		-0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.625, 0.250,
		 0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.625, 0.500,
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.375, 0.250,
		 0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.625, 0.500,
		 0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.375, 0.500,
	};

	u32 indices[] = {
		0, 1, 2, 3, 4, 5, 6,
		7, 8, 9, 10, 11, 12, 13,
		14, 15, 16, 17, 18, 19, 20,
		21, 22, 23, 24, 25, 26, 27,
		28, 29, 30, 31, 32, 33, 34,
		35,
	};

	alice_VertexBuffer* cube = alice_new_vertex_buffer(
		ALICE_VERTEXBUFFER_DRAW_TRIANGLES | ALICE_VERTEXBUFFER_STATIC_DRAW);

	alice_bind_vertex_buffer_for_edit(cube);
	alice_push_vertices(cube, verts, sizeof(verts) / sizeof(float));
	alice_push_indices(cube, indices, sizeof(indices) / sizeof(u32));
	alice_configure_vertex_buffer(cube, 0, 3, 8, 0); /* vec3 position */
	alice_configure_vertex_buffer(cube, 1, 3, 8, 3); /* vec3 normal */
	alice_configure_vertex_buffer(cube, 2, 2, 8, 6); /* vec2 uv */
	alice_bind_vertex_buffer_for_edit(NULL);

	alice_Mesh mesh;

	alice_init_mesh(&mesh, cube);

	alice_calculate_aabb_from_mesh(&mesh.aabb, mesh.transform, verts, sizeof(verts) / sizeof(float), 8);

	return mesh;
}

alice_Mesh alice_new_sphere_mesh() {
	const float sector_count = 36.0f;
	const float stack_count = 18.0f;
	const float radius = 0.5;

	float x, y, z, xy;
	float nx, ny, nz, lengthInv = 1.0f / radius;

	float s, t;

	float sectorStep = 2 * alice_pi / sector_count;
	float stackStep = alice_pi / stack_count;
	float sector_angle, stackAngle;

	float* vertices = NULL;
	u32 vertex_count = 0;

	u32* indices = NULL;
	u32 index_count = 0;

	for (int i = 0; i <= stack_count; ++i) {
		int k1 = i * (sector_count + 1);
		int k2 = k1 + sector_count + 1;

		stackAngle = alice_pi / 2 - i * stackStep;
		xy = radius * cosf(stackAngle);
		z = radius * sinf(stackAngle);

		for (int j = 0; j <= sector_count; ++j, ++k1, ++k2) {
			sector_angle = j * sectorStep;

			x = xy * cosf(sector_angle);
			y = xy * sinf(sector_angle);

			nx = x * lengthInv;
			ny = y * lengthInv;
			nz = z * lengthInv;

			s = (float)j / sector_count;
			t = (float)i / stack_count;

			vertices = realloc(vertices, (vertex_count + 8) * sizeof(float));

			vertices[vertex_count++] = x;
			vertices[vertex_count++] = y;
			vertices[vertex_count++] = z;
			vertices[vertex_count++] = nx;
			vertices[vertex_count++] = ny;
			vertices[vertex_count++] = nz;
			vertices[vertex_count++] = s;
			vertices[vertex_count++] = t;

			if (i != 0) {
				indices = realloc(indices, (index_count + 3) * sizeof(u32));
				indices[index_count++] = k1;
				indices[index_count++] = k2;
				indices[index_count++] = k1 + 1;
			}


			if (i != (stack_count - 1)) {
				indices = realloc(indices, (index_count + 3) * sizeof(u32));
				indices[index_count++] = k1 + 1;
				indices[index_count++] = k2;
				indices[index_count++] = k2 + 1;
			}
		}
	}

	alice_VertexBuffer* sphere = alice_new_vertex_buffer(
		ALICE_VERTEXBUFFER_DRAW_TRIANGLES | ALICE_VERTEXBUFFER_STATIC_DRAW);

	alice_bind_vertex_buffer_for_edit(sphere);
	alice_push_vertices(sphere, vertices, vertex_count);
	alice_push_indices(sphere, indices, index_count);
	alice_configure_vertex_buffer(sphere, 0, 3, 8, 0); /* vec3 position */
	alice_configure_vertex_buffer(sphere, 1, 3, 8, 3); /* vec3 normal */
	alice_configure_vertex_buffer(sphere, 2, 2, 8, 6); /* vec2 uv */
	alice_bind_vertex_buffer_for_edit(NULL);

	alice_Mesh mesh;

	alice_init_mesh(&mesh, sphere);

	alice_calculate_aabb_from_mesh(&mesh.aabb, mesh.transform, vertices, vertex_count, 8);

	free(vertices);
	free(indices);

	return mesh;
}

void alice_init_model(alice_Model* model) {
	assert(model);

	model->meshes = alice_null;
	model->mesh_count = 0;
	model->mesh_capacity = 0;
}

void alice_deinit_model(alice_Model* model) {
	assert(model);

	for (u32 i = 0; i < model->mesh_count; i++) {
		alice_deinit_mesh(&model->meshes[i]);
	}

	if (model->mesh_capacity > 0) {
		free(model->meshes);
	}
}

alice_Model* alice_new_model() {
	alice_Model* new = malloc(sizeof(alice_Model));

	alice_init_model(new);

	return new;
}

void alice_free_model(alice_Model* model) {
	assert(model);

	alice_deinit_model(model);

	free(model);
}

void alice_model_add_mesh(alice_Model* model, alice_Mesh mesh) {
	assert(model);

	if (model->mesh_count >= model->mesh_capacity) {
		model->mesh_capacity = alice_grow_capacity(model->mesh_capacity);
		model->meshes = realloc(model->meshes, model->mesh_capacity * sizeof(alice_Mesh));
	}

	model->meshes[model->mesh_count++] = mesh;
}

alice_calculate_aabb_from_mesh(alice_AABB* aabb, alice_m4f transform,
		float* vertices, u32 position_count, u32 position_stride) {
	assert(aabb);
	assert(vertices);

	aabb->min = (alice_v3f) { 0.0f, 0.0f, 0.0f };
	aabb->max = (alice_v3f) { 0.0f, 0.0f, 0.0f };

	for (u32 i = 0; i < position_count; i += 3 * position_stride) {
		alice_v3f position = (alice_v3f) {
			.x = vertices[i],
			.y = vertices[i + 1],
			.z = vertices[i + 2]
		};

		position = alice_v3f_transform(position, transform);

		if (position.x < aabb->min.x &&
				position.y < aabb->min.y,
				position.z < aabb->min.z) {
			aabb->min = position;
		}

		if (position.x > aabb->max.x &&
				position.y > aabb->max.y &&
				position.z > aabb->max.z) {
			aabb->max = position;
		}
	}
}

void alice_render_clear() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void alice_depth_clear() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void alice_enable_depth() {
	glEnable(GL_DEPTH_TEST);
}

void alice_disable_depth() {
	glDisable(GL_DEPTH_TEST);
}

alice_Camera3D* alice_get_scene_camera(alice_Scene* scene) {
	assert(scene);

	alice_Camera3D* camera = alice_null;

	for (alice_entity_iter(scene, iter, alice_Camera3D)) {
		alice_Camera3D* entity = iter.current_ptr;
		if (entity->active) {
			camera = entity;
			break;
		}
	}

	return camera;
}

alice_m4f alice_get_camera_3d_matrix(alice_Scene* scene, alice_Camera3D* camera) {
	assert(camera);

	alice_v3f rotation = alice_torad_v3f(camera->base.rotation);

	alice_v3f position = alice_get_entity_world_position(scene, (alice_Entity*)camera);

	alice_v3f direction = (alice_v3f){
		.x = cosf(rotation.x) * sinf(rotation.y),
		.y = sinf(rotation.x),
		.z = cosf(rotation.x) * cosf(rotation.y)
	};

	alice_m4f view = alice_m4f_lookat(position,
		(alice_v3f){
			.x = position.x + direction.x,
			.y = position.y + direction.y,
			.z = position.z + direction.z
		}, (alice_v3f){0.0, 1.0, 0.0});

	alice_m4f projection = alice_m4f_persp(camera->fov,
		camera->dimentions.x / camera->dimentions.y,
		camera->near, camera->far);

	return alice_m4f_multiply(projection, view);
}

void alice_apply_material(alice_Scene* scene, alice_Material* material) {
	assert(material);

	if (!material->shader) {
		alice_log_warning("Attempting to render object who's material doesn't have a shader");
		return;
	}

	alice_bind_shader(material->shader);
	alice_shader_set_color(material->shader, "material.albedo", material->albedo);
	alice_shader_set_float(material->shader, "material.metallic", material->metallic);
	alice_shader_set_float(material->shader, "material.roughness", material->roughness);
	alice_shader_set_float(material->shader, "material.emissive", material->emissive);

	if (material->albedo_map) {
		alice_shader_set_int(material->shader, "material.use_albedo_map", 1);
		alice_shader_set_int(material->shader, "material.albedo_map", 0);
		alice_bind_texture(material->albedo_map, 0);
	} else {
		alice_shader_set_int(material->shader, "material.use_albedo_map", 0);
	}

	if (material->normal_map) {
		alice_shader_set_int(material->shader, "material.use_normal_map", 1);
		alice_shader_set_int(material->shader, "material.normal_map", 1);
		alice_bind_texture(material->normal_map, 1);
	} else {
		alice_shader_set_int(material->shader, "material.use_normal_map", 0);
	}

	if (material->metallic_map) {
		alice_shader_set_int(material->shader, "material.use_metallic_map", 1);
		alice_shader_set_int(material->shader, "material.metallic_map", 2);
		alice_bind_texture(material->metallic_map, 2);
	} else {
		alice_shader_set_int(material->shader, "material.use_metallic_map", 0);
	}

	if (material->roughness_map) {
		alice_shader_set_int(material->shader, "material.use_roughness_map", 1);
		alice_shader_set_int(material->shader, "material.roughness_map", 3);
		alice_bind_texture(material->roughness_map, 3);
	} else {
		alice_shader_set_int(material->shader, "material.use_roughness_map", 0);
	}

	if (material->ambient_occlusion_map) {
		alice_shader_set_int(material->shader, "material.use_ambient_occlusion_map", 1);
		alice_shader_set_int(material->shader, "material.ambient_occlusion_map", 4);
		alice_bind_texture(material->ambient_occlusion_map, 4);
	} else {
		alice_shader_set_int(material->shader, "material.use_ambient_occlusion_map", 0);
	}

	/* Apply directional lights */
	u32 light_count = 0;
	for (alice_entity_iter(scene, iter, alice_DirectionalLight)) {
		alice_DirectionalLight* light = iter.current_ptr;

		char name[256];

		sprintf(name, "directional_lights[%d].color", light_count);
		alice_shader_set_color(material->shader, name, light->color);

		sprintf(name, "directional_lights[%d].direction", light_count);
		alice_shader_set_v3f(material->shader, name, light->base.position);

		sprintf(name, "directional_lights[%d].intensity", light_count);
		alice_shader_set_float(material->shader, name, light->intensity);

		light_count++;
	}

	alice_shader_set_uint(material->shader, "directional_light_count", light_count);
}

void alice_apply_point_lights(alice_Scene* scene, alice_AABB mesh_aabb, alice_Material* material) {
	assert(scene);
	assert(material);

	u32 light_count = 0;
	for (alice_entity_iter(scene, iter, alice_PointLight)) {
		alice_PointLight* light = iter.current_ptr;

		if (!alice_sphere_vs_aabb(mesh_aabb, light->base.position, light->range)) {
			continue;
		}

		char name[256];

		sprintf(name, "point_lights[%d].color", light_count);
		alice_shader_set_color(material->shader, name, light->color);

		sprintf(name, "point_lights[%d].position", light_count);
		alice_shader_set_v3f(material->shader, name,
				alice_get_entity_world_position(scene, (alice_Entity*)light));

		sprintf(name, "point_lights[%d].intensity", light_count);
		alice_shader_set_float(material->shader, name, light->intensity);

		sprintf(name, "point_lights[%d].range", light_count);
		alice_shader_set_float(material->shader, name, light->range);

		light_count++;
	}

	alice_shader_set_uint(material->shader, "point_light_count", light_count);
}

void alice_on_renderable_3d_create(alice_Scene* scene, alice_EntityHandle handle, void* ptr) {
	alice_Renderable3D* renderable = ptr;

	renderable->materials = alice_null;
	renderable->material_count = 0;
	renderable->material_capacity = 0;
}

void alice_on_renderable_3d_destroy(alice_Scene* scene, alice_EntityHandle handle, void* ptr) {
	alice_Renderable3D* renderable = ptr;

	if (renderable->material_capacity > 0) {
		free(renderable->materials);
	}
}

void alice_renderable_3d_add_material(alice_Renderable3D* renderable, const char* material_path) {
	assert(renderable);

	if (renderable->material_count >= renderable->material_capacity) {
		renderable->material_capacity = alice_grow_capacity(renderable->material_capacity);
		renderable->materials = realloc(renderable->materials, renderable->material_capacity * sizeof(alice_Material*));
	}

	renderable->materials[renderable->material_count++] = alice_load_material(material_path);
}

alice_SceneRenderer3D* alice_new_scene_renderer_3d(alice_Shader* postprocess_shader,
		alice_Shader* extract_shader, alice_Shader* blur_shader) {
	assert(postprocess_shader);
	assert(extract_shader);
	assert(blur_shader);

	alice_SceneRenderer3D* new = malloc(sizeof(alice_SceneRenderer3D));

	new->output = alice_new_render_target(128, 128, 1);
	new->bright_pixels = alice_new_render_target(128, 128, 1);
	new->bloom_ping_pong[0] = alice_new_render_target(128, 128, 1);
	new->bloom_ping_pong[1] = alice_new_render_target(128, 128, 1);

	new->postprocess = postprocess_shader;
	new->extract = extract_shader;
	new->blur = blur_shader;

	new->use_bloom = false;
	new->use_antialiasing = false;

	new->color_mod = ALICE_COLOR_WHITE;

	float verts[] = {
		 1.0,  1.0, 	1.0f, 1.0f,
		 1.0, -1.0, 	1.0f, 0.0f,
		-1.0, -1.0, 	0.0f, 0.0f,
		-1.0,  1.0, 	0.0f, 1.0f
	};

	u32 indices[] = {
		3, 2, 1,
		3, 1, 0
	};

	new->quad = alice_new_vertex_buffer(
		ALICE_VERTEXBUFFER_DRAW_TRIANGLES | ALICE_VERTEXBUFFER_STATIC_DRAW);

	alice_bind_vertex_buffer_for_edit(new->quad);
	alice_push_vertices(new->quad, verts, sizeof(verts) / sizeof(float));
	alice_push_indices(new->quad, indices, sizeof(indices) / sizeof(u32));
	alice_configure_vertex_buffer(new->quad, 0, 2, 4, 0); /* vec2 position */
	alice_configure_vertex_buffer(new->quad, 1, 2, 4, 2); /* vec2 uv */
	alice_bind_vertex_buffer_for_edit(NULL);

	return new;
}

void alice_free_scene_renderer_3d(alice_SceneRenderer3D* renderer) {
	assert(renderer);

	alice_free_render_target(renderer->output);
	alice_free_render_target(renderer->bright_pixels);
	alice_free_render_target(renderer->bloom_ping_pong[0]);
	alice_free_render_target(renderer->bloom_ping_pong[1]);
	alice_free_vertex_buffer(renderer->quad);
	free(renderer);
}

void alice_render_scene_3d(alice_SceneRenderer3D* renderer, u32 width, u32 height,
		alice_Scene* scene, alice_RenderTarget* render_target) {
	assert(renderer);
	assert(scene);

	alice_Camera3D* camera = alice_get_scene_camera(scene);
	if (!camera) {
		alice_log_warning("Attempting 3D scene render with no active 3D camera");
		return;
	}

	camera->dimentions = (alice_v2f){width, height};

	alice_resize_render_target(renderer->output, width, height);
	alice_bind_render_target(renderer->output, width, height);

	alice_enable_depth();

	for (alice_entity_iter(scene, iter, alice_Renderable3D)) {
		alice_Renderable3D* renderable = iter.current_ptr;

		alice_m4f transform_matrix = alice_get_entity_transform(scene, (alice_Entity*)renderable);

		alice_Model* model = renderable->model;
		if (!model) {
			continue;
		}

		for (u32 i = 0; i < model->mesh_count; i++) {
			alice_Mesh* mesh = &model->meshes[i];
			alice_VertexBuffer* vb = mesh->vb;

			alice_Material* material = alice_null;
			if (i < renderable->material_count) {
				material = renderable->materials[i];
			} else if (renderable->material_count == 1) {
				material = renderable->materials[0];
			}

			if (!material) {
				alice_log_warning("Attempting to render object that doesn't have any materials.");
				goto renderable_iter_continue;
			}

			alice_apply_material(scene, material);

			alice_AABB mesh_aabb = mesh->aabb;
			mesh_aabb.min = alice_v3f_transform(mesh_aabb.min, transform_matrix);
			mesh_aabb.max = alice_v3f_transform(mesh_aabb.max, transform_matrix);

			alice_apply_point_lights(scene, mesh_aabb, material);

			alice_Shader* shader = material->shader;

			alice_m4f model = alice_m4f_multiply(transform_matrix, mesh->transform);

			alice_shader_set_m4f(shader, "transform", model);
			alice_shader_set_v3f(shader, "camera_position", alice_get_entity_world_position(scene, camera));
			alice_shader_set_float(shader, "gamma", camera->gamma);
			alice_shader_set_m4f(shader, "camera", alice_get_camera_3d_matrix(scene, camera));

			alice_bind_vertex_buffer_for_draw(vb);
			alice_draw_vertex_buffer(vb);
		}

renderable_iter_continue:
		continue;
	}

	alice_unbind_render_target(renderer->output);


	alice_RenderTarget* bloom_output;
	if (renderer->use_bloom) {
		/* Draw bright areas to bloom target */
		alice_resize_render_target(renderer->bright_pixels, width, height);
		alice_bind_render_target(renderer->bright_pixels, width, height);

		alice_bind_shader(renderer->extract);

		alice_render_target_bind_output(renderer->output, 0, 0);
		alice_shader_set_int(renderer->extract, "input_color", 0);

		alice_shader_set_float(renderer->extract, "input_width", renderer->output->width);
		alice_shader_set_float(renderer->extract, "input_height", renderer->output->height);

		alice_bind_vertex_buffer_for_draw(renderer->quad);
		alice_draw_vertex_buffer(renderer->quad);
		alice_bind_vertex_buffer_for_draw(alice_null);

		alice_unbind_render_target(renderer->bright_pixels);

		/* Blur the bloom target */
		bool horizontal = true;
		bool first_iteration = true;
		for (u32 i = 0; i < 10; i++) {
			bloom_output = renderer->bloom_ping_pong[horizontal];

			alice_resize_render_target(bloom_output, width, height);
			alice_bind_render_target(bloom_output, width, height);

			alice_bind_shader(renderer->blur);

			if (first_iteration) {
				alice_render_target_bind_output(renderer->bright_pixels, 0, 0);
				first_iteration = false;
			} else {
				alice_render_target_bind_output(renderer->bloom_ping_pong[!horizontal], 0, 0);
			}

			alice_shader_set_int(renderer->blur, "input_color", 0);

			alice_shader_set_int(renderer->blur, "horizontal", horizontal);

			alice_shader_set_float(renderer->blur, "input_width", renderer->bright_pixels->width);
			alice_shader_set_float(renderer->blur, "input_height", renderer->bright_pixels->height);

			alice_bind_vertex_buffer_for_draw(renderer->quad);
			alice_draw_vertex_buffer(renderer->quad);
			alice_bind_vertex_buffer_for_draw(alice_null);

			alice_unbind_render_target(bloom_output);

			horizontal = !horizontal;
		}
	}

	/* Draw to render target/backbuffer */
	if (render_target) {
		alice_resize_render_target(render_target, width, height);
		alice_bind_render_target(render_target, width, height);
	}

	alice_bind_shader(renderer->postprocess);
	alice_render_target_bind_output(renderer->output, 0, 0);

	if (renderer->use_bloom) {
		alice_render_target_bind_output(bloom_output, 0, 1);
		alice_shader_set_int(renderer->postprocess, "bloom_texture", 1);
	}

	alice_shader_set_int(renderer->postprocess, "use_bloom", renderer->use_bloom);
	alice_shader_set_int(renderer->postprocess, "use_antialiasing", renderer->use_antialiasing);

	alice_shader_set_int(renderer->postprocess, "input_color", 0);

	alice_shader_set_float(renderer->postprocess, "input_width", renderer->output->width);
	alice_shader_set_float(renderer->postprocess, "input_height", renderer->output->height);
	alice_shader_set_float(renderer->postprocess, "exposure", camera->exposure);
	alice_shader_set_float(renderer->postprocess, "gamma", camera->gamma);

	alice_shader_set_color(renderer->postprocess, "color_mod", renderer->color_mod);

	alice_bind_vertex_buffer_for_draw(renderer->quad);
	alice_draw_vertex_buffer(renderer->quad);
	alice_bind_vertex_buffer_for_draw(alice_null);

	alice_bind_shader(alice_null);

	if (render_target) {
		alice_unbind_render_target(render_target);
	}

	alice_disable_depth();
}

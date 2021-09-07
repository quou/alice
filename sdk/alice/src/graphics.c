#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <glad/glad.h>
#include <stb_image.h>

#include "alice/graphics.h"
#include "alice/debugrenderer.h"
#include "alice/input.h"

u32 total_draw_calls;

u32 alice_get_total_draw_calls() {
	return total_draw_calls;
}

alice_color_t alice_color_from_rgb_color(alice_rgb_color_t rgb) {
	i8 r = (i8)(rgb.r * 255.0);
	i8 g = (i8)(rgb.g * 255.0);
	i8 b = (i8)(rgb.b * 255.0);

	return ((r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff);
}

alice_rgb_color_t alice_rgb_color_from_color(alice_color_t color) {
	int r = (color >> 16) & 0xFF;
	int g = (color >> 8) & 0xFF;
	int b = color & 0xFF;

	float rf = (float)r / 255.0f;
	float gf = (float)g / 255.0f;
	float bf = (float)b / 255.0f;

	return (alice_rgb_color_t) { rf, gf, bf };
}

alice_render_target_t* alice_new_render_target(u32 width, u32 height, u32 color_attachment_count) {
	alice_render_target_t* target = malloc(sizeof(alice_render_target_t));

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

void alice_free_render_target(alice_render_target_t* target) {
	assert(target);

	glDeleteRenderbuffers(1, &target->render_buffer);
	glDeleteFramebuffers(1, &target->frame_buffer);

	glDeleteTextures(target->color_attachment_count, target->color_attachments);

	if (target->color_attachment_count > 0) {
		free(target->color_attachments);
	}

	free(target);
}

void alice_bind_render_target(alice_render_target_t* target, u32 old_width, u32 old_height) {
	assert(target);

	target->old_width = old_width;
	target->old_height = old_height;

	glBindFramebuffer(GL_FRAMEBUFFER, target->frame_buffer);
	glBindRenderbuffer(GL_RENDERBUFFER, target->render_buffer);
	glViewport(0, 0, target->width, target->height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void alice_unbind_render_target(alice_render_target_t* target) {
	assert(target);

	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0, 0, target->old_width, target->old_height);
}

void alice_resize_render_target(alice_render_target_t* target, u32 width, u32 height) {
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

void alice_render_target_bind_output(alice_render_target_t* target, u32 attachment_index, u32 unit) {
	assert(target);
	assert(attachment_index < target->color_attachment_count);

	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, target->color_attachments[attachment_index]);
}

alice_vertex_buffer_t* alice_new_vertex_buffer(alice_vertex_buffer_flags_t flags) {
	alice_vertex_buffer_t* buffer = malloc(sizeof(alice_vertex_buffer_t));

	buffer->flags = flags;

	glGenVertexArrays(1, &buffer->va_id);
	glGenBuffers(1, &buffer->vb_id);
	glGenBuffers(1, &buffer->ib_id);

	return buffer;
}

void alice_free_vertex_buffer(alice_vertex_buffer_t* buffer) {
	assert(buffer);

	glDeleteVertexArrays(1, &buffer->va_id);
	glDeleteBuffers(1, &buffer->vb_id);
	glDeleteBuffers(1, &buffer->ib_id);

	free(buffer);
}

void alice_bind_vertex_buffer_for_draw(alice_vertex_buffer_t* buffer) {
	glBindVertexArray(buffer ? buffer->va_id : 0);
}

void alice_bind_vertex_buffer_for_edit(alice_vertex_buffer_t* buffer) {
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

void alice_push_vertices(alice_vertex_buffer_t* buffer, float* vertices, u32 count) {
	assert(buffer);

	u32 mode = GL_STATIC_DRAW;
	if (buffer->flags & ALICE_VERTEXBUFFER_DYNAMIC_DRAW) {
		mode = GL_DYNAMIC_DRAW;
	}

	glBufferData(GL_ARRAY_BUFFER, count * sizeof(float), vertices, mode);
}

void alice_push_indices(alice_vertex_buffer_t* buffer, u32* indices, u32 count) {
	assert(buffer);

	u32 mode = GL_STATIC_DRAW;
	if (buffer->flags & ALICE_VERTEXBUFFER_DYNAMIC_DRAW) {
		mode = GL_DYNAMIC_DRAW;
	}

	buffer->index_count = count;

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(u32), indices, mode);
}

void alice_update_vertices(alice_vertex_buffer_t* buffer, float* vertices, u32 offset, u32 count) {
	assert(buffer);

	glBufferSubData(GL_ARRAY_BUFFER, offset * sizeof(float),
		count * sizeof(float), vertices);
}

void alice_update_indices(alice_vertex_buffer_t* buffer, u32* indices, u32 offset, u32 count) {
	assert(buffer);

	buffer->index_count = count;

	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset * sizeof(u32),
		count * sizeof(u32), indices);
}

void alice_configure_vertex_buffer(alice_vertex_buffer_t* buffer, u32 index, u32 component_count,
	u32 stride, u32 offset) {
	assert(buffer);

	glVertexAttribPointer(index, component_count, GL_FLOAT, GL_FALSE,
		stride * sizeof(float), (void*)(u64)(offset * sizeof(float)));
	glEnableVertexAttribArray(index);
}

void alice_draw_vertex_buffer(alice_vertex_buffer_t* buffer) {
	assert(buffer);

	u32 draw_type = GL_TRIANGLES;
	if (buffer->flags & ALICE_VERTEXBUFFER_DRAW_LINES) {
		draw_type = GL_LINES;
	}
	else if (buffer->flags & ALICE_VERTEXBUFFER_DRAW_LINE_STRIP) {
		draw_type = GL_LINE_STRIP;
	}

	glDrawElements(draw_type, buffer->index_count, GL_UNSIGNED_INT, 0);

	total_draw_calls++;
}

void alice_draw_vertex_buffer_custom_count(alice_vertex_buffer_t* buffer, u32 count) {
	assert(buffer);

	u32 draw_type = GL_TRIANGLES;
	if (buffer->flags & ALICE_VERTEXBUFFER_DRAW_LINES) {
		draw_type = GL_LINES;
	}
	else if (buffer->flags & ALICE_VERTEXBUFFER_DRAW_LINE_STRIP) {
		draw_type = GL_LINE_STRIP;
	}

	glDrawElements(draw_type, count, GL_UNSIGNED_INT, 0);

	total_draw_calls++;
}

alice_shader_t* alice_init_shader(alice_shader_t* shader, char* source) {
	assert(shader);

	shader->panic_mode = false;

	const u32 source_len = (u32)strlen(source);

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

void alice_deinit_shader(alice_shader_t* shader) {
	assert(shader);

	if (shader->panic_mode) { return; };

	glDeleteProgram(shader->id);
}

alice_shader_t* alice_new_shader(alice_resource_t* resource) {
	assert(resource);

	if (resource->type != ALICE_RESOURCE_STRING || resource->payload_size <= 0) {
		alice_log_error("Resource insuffucient for shader creation.");
		return NULL;
	}

	alice_shader_t* s = malloc(sizeof(alice_shader_t));
	alice_init_shader(s, resource->payload);

	return s;
}

void alice_free_shader(alice_shader_t* shader) {
	assert(shader);
	alice_deinit_shader(shader);

	free(shader);
}

void alice_bind_shader(alice_shader_t* shader) {
	if (!shader || shader->panic_mode) { return; };

	glUseProgram(shader ? shader->id : 0);
}

void alice_shader_set_int(alice_shader_t* shader, const char* name, i32 v) {
	assert(shader);

	if (shader->panic_mode) { return; };

	u32 location = glGetUniformLocation(shader->id, name);
	glUniform1i(location, v);
}

void alice_shader_set_uint(alice_shader_t* shader, const char* name, u32 v) {
	assert(shader);

	if (shader->panic_mode) { return; };

	u32 location = glGetUniformLocation(shader->id, name);
	glUniform1ui(location, v);
}

void alice_shader_set_float(alice_shader_t* shader, const char* name, float v) {
	assert(shader);

	if (shader->panic_mode) { return; };

	u32 location = glGetUniformLocation(shader->id, name);
	glUniform1f(location, v);
}

void alice_shader_set_color(alice_shader_t* shader, const char* name, alice_color_t color) {
	assert(shader);

	alice_rgb_color_t rgb = alice_rgb_color_from_color(color);

	alice_shader_set_v3f(shader, name, (alice_v3f_t) { rgb.r, rgb.g, rgb.b });
}

void alice_shader_set_rgb_color(alice_shader_t* shader, const char* name, alice_rgb_color_t color) {
	assert(shader);

	alice_shader_set_v3f(shader, name, (alice_v3f_t) { color.r, color.g, color.b });
}

void alice_shader_set_v2i(alice_shader_t* shader, const char* name, alice_v2i_t v) {
	assert(shader);

	if (shader->panic_mode) { return; };

	u32 location = glGetUniformLocation(shader->id, name);
	glUniform2i(location, v.x, v.y);
}

void alice_shader_set_v2u(alice_shader_t* shader, const char* name, alice_v2u_t v) {
	assert(shader);

	if (shader->panic_mode) { return; };

	u32 location = glGetUniformLocation(shader->id, name);
	glUniform2ui(location, v.x, v.y);
}

void alice_shader_set_v2f(alice_shader_t* shader, const char* name, alice_v2f_t v) {
	assert(shader);

	if (shader->panic_mode) { return; };

	u32 location = glGetUniformLocation(shader->id, name);
	glUniform2f(location, v.x, v.y);
}

void alice_shader_set_v3i(alice_shader_t* shader, const char* name, alice_v3i_t v) {
	assert(shader);

	if (shader->panic_mode) { return; };

	u32 location = glGetUniformLocation(shader->id, name);
	glUniform3i(location, v.x, v.y, v.z);
}

void alice_shader_set_v3u(alice_shader_t* shader, const char* name, alice_v3u_t v) {
	assert(shader);

	if (shader->panic_mode) { return; };

	u32 location = glGetUniformLocation(shader->id, name);
	glUniform3ui(location, v.x, v.y, v.z);
}

void alice_shader_set_v3f(alice_shader_t* shader, const char* name, alice_v3f_t v) {
	assert(shader);

	if (shader->panic_mode) { return; };

	u32 location = glGetUniformLocation(shader->id, name);
	glUniform3f(location, v.x, v.y, v.z);
}

void alice_shader_set_v4i(alice_shader_t* shader, const char* name, alice_v4i v) {
	assert(shader);

	if (shader->panic_mode) { return; };

	u32 location = glGetUniformLocation(shader->id, name);
	glUniform4i(location, v.x, v.y, v.z, v.w);
}

void alice_shader_set_v4u(alice_shader_t* shader, const char* name, alice_v4u v) {
	assert(shader);

	if (shader->panic_mode) { return; };

	u32 location = glGetUniformLocation(shader->id, name);
	glUniform4ui(location, v.x, v.y, v.z, v.w);
}

void alice_shader_set_v4f(alice_shader_t* shader, const char* name, alice_v4f_t v) {
	assert(shader);

	if (shader->panic_mode) { return; };

	u32 location = glGetUniformLocation(shader->id, name);
	glUniform4f(location, v.x, v.y, v.z, v.w);
}

void alice_shader_set_m4f(alice_shader_t* shader, const char* name, alice_m4f_t v) {
	assert(shader);

	if (shader->panic_mode) { return; };

	u32 location = glGetUniformLocation(shader->id, name);
	glUniformMatrix4fv(location, 1, GL_FALSE, (float*)v.elements);
}

alice_texture_t* alice_new_texture(alice_resource_t* resource, alice_texture_flags_t flags) {
	assert(resource);

	alice_texture_t* texture = malloc(sizeof(alice_texture_t));
	alice_init_texture(texture, resource, flags);
	return texture;
}

alice_texture_t* alice_new_texture_from_memory(void* data, u32 size, alice_texture_flags_t flags) {
	alice_texture_t* texture = malloc(sizeof(alice_texture_t));
	alice_init_texture_from_memory(texture, data, size, flags);
	return texture;
}


alice_texture_t* alice_new_texture_from_memory_uncompressed(unsigned char* pixels,
	u32 size, i32 width, i32 height, i32 component_count, alice_texture_flags_t flags) {
	alice_texture_t* texture = malloc(sizeof(alice_texture_t));
	alice_init_texture_from_memory_uncompressed(texture, pixels, size, width, height, component_count, flags);
	return texture;
}

void alice_init_texture(alice_texture_t* texture, alice_resource_t* resource, alice_texture_flags_t flags) {
	if (resource->payload == NULL || resource->type != ALICE_RESOURCE_BINARY) {
		alice_log_error("Resource insufficient for texture creation.");
		return;
	}

	alice_init_texture_from_memory(texture, resource->payload, resource->payload_size, flags);
}

void alice_init_texture_from_memory(alice_texture_t* texture, void* data, u32 size, alice_texture_flags_t flags) {
	if (data == NULL || size == 0) {
		alice_log_error("Data insufficient for texture creation.");
		return;
	}

	i32 width, height, component_count;

	stbi_set_flip_vertically_on_load(true);

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

void alice_init_texture_from_memory_uncompressed(alice_texture_t* texture, unsigned char* pixels,
	u32 size, i32 width, i32 height, i32 component_count, alice_texture_flags_t flags) {
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

	u32 format = GL_RGB;
	u32 internal_format = GL_RGB;
	if (texture->component_count == 4) {
		internal_format = GL_RGBA;
		format = GL_RGBA;
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

void alice_deinit_texture(alice_texture_t* texture) {
	assert(texture);
	glDeleteTextures(1, &texture->id);
}

void alice_free_texture(alice_texture_t* texture) {
	assert(texture);

	alice_deinit_texture(texture);

	free(texture);
}

void alice_bind_texture(alice_texture_t* texture, u32 slot) {
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, texture ? texture->id : 0);
}

void alice_init_mesh(alice_mesh_t* mesh, alice_vertex_buffer_t* vb) {
	assert(mesh);

	*mesh = (alice_mesh_t){
		.transform = alice_m4f_identity(),

		.vb = vb
	};
}

void alice_deinit_mesh(alice_mesh_t* mesh) {
	assert(mesh);

	alice_free_vertex_buffer(mesh->vb);
}

alice_mesh_t alice_new_cube_mesh() {
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

	alice_vertex_buffer_t* cube = alice_new_vertex_buffer(
		ALICE_VERTEXBUFFER_DRAW_TRIANGLES | ALICE_VERTEXBUFFER_STATIC_DRAW);

	alice_bind_vertex_buffer_for_edit(cube);
	alice_push_vertices(cube, verts, sizeof(verts) / sizeof(float));
	alice_push_indices(cube, indices, sizeof(indices) / sizeof(u32));
	alice_configure_vertex_buffer(cube, 0, 3, 8, 0); /* vec3 position */
	alice_configure_vertex_buffer(cube, 1, 3, 8, 3); /* vec3 normal */
	alice_configure_vertex_buffer(cube, 2, 2, 8, 6); /* vec2 uv */
	alice_bind_vertex_buffer_for_edit(NULL);

	alice_mesh_t mesh;

	alice_init_mesh(&mesh, cube);

	alice_calculate_aabb_from_mesh(&mesh.aabb, verts, sizeof(verts) / sizeof(float), 8);

	return mesh;
}

alice_mesh_t alice_new_sphere_mesh() {
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

	for (u32 i = 0; i <= stack_count; ++i) {
		u32 k1 = i * ((u32)sector_count + 1);
		u32 k2 = k1 + (u32)sector_count + 1;

		stackAngle = alice_pi / 2 - i * stackStep;
		xy = radius * cosf(stackAngle);
		z = radius * sinf(stackAngle);

		for (u32 j = 0; j <= sector_count; ++j, ++k1, ++k2) {
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

	alice_vertex_buffer_t* sphere = alice_new_vertex_buffer(
		ALICE_VERTEXBUFFER_DRAW_TRIANGLES | ALICE_VERTEXBUFFER_STATIC_DRAW);

	alice_bind_vertex_buffer_for_edit(sphere);
	alice_push_vertices(sphere, vertices, vertex_count);
	alice_push_indices(sphere, indices, index_count);
	alice_configure_vertex_buffer(sphere, 0, 3, 8, 0); /* vec3 position */
	alice_configure_vertex_buffer(sphere, 1, 3, 8, 3); /* vec3 normal */
	alice_configure_vertex_buffer(sphere, 2, 2, 8, 6); /* vec2 uv */
	alice_bind_vertex_buffer_for_edit(NULL);

	alice_mesh_t mesh;

	alice_init_mesh(&mesh, sphere);

	alice_calculate_aabb_from_mesh(&mesh.aabb, vertices, vertex_count, 8);

	free(vertices);
	free(indices);

	return mesh;
}

void alice_init_model(alice_model_t* model) {
	assert(model);

	model->meshes = alice_null;
	model->mesh_count = 0;
	model->mesh_capacity = 0;
}

void alice_deinit_model(alice_model_t* model) {
	assert(model);

	for (u32 i = 0; i < model->mesh_count; i++) {
		alice_deinit_mesh(&model->meshes[i]);
	}

	if (model->mesh_capacity > 0) {
		free(model->meshes);
	}
}

alice_model_t* alice_new_model() {
	alice_model_t* new = malloc(sizeof(alice_model_t));

	alice_init_model(new);

	return new;
}

void alice_free_model(alice_model_t* model) {
	assert(model);

	alice_deinit_model(model);

	free(model);
}

void alice_model_add_mesh(alice_model_t* model, alice_mesh_t mesh) {
	assert(model);

	if (model->mesh_count >= model->mesh_capacity) {
		model->mesh_capacity = alice_grow_capacity(model->mesh_capacity);
		model->meshes = realloc(model->meshes, model->mesh_capacity * sizeof(alice_mesh_t));
	}

	model->meshes[model->mesh_count++] = mesh;
}

void alice_calculate_aabb_from_mesh(alice_aabb_t* aabb,
		float* vertices, u32 position_count, u32 position_stride) {
	assert(aabb);
	assert(vertices);

	*aabb = (alice_aabb_t){
		.min = {INFINITY, INFINITY, INFINITY},
		.max = {-INFINITY, -INFINITY, -INFINITY}
	};

	for (u32 i = 0; i < position_count; i += position_stride) {
		alice_v3f_t position = (alice_v3f_t) {
			.x = vertices[i],
			.y = vertices[i + 1],
			.z = vertices[i + 2]
		};

		if (position.x < aabb->min.x) {
			aabb->min.x = position.x;
		}

		if (position.y < aabb->min.y) {
			aabb->min.y = position.y;
		}

		if (position.z < aabb->min.z) {
			aabb->min.z = position.z;
		}

		if (position.x > aabb->max.x) {
			aabb->max.x = position.x;
		}

		if (position.y > aabb->max.y) {
			aabb->max.y = position.y;
		}

		if (position.z > aabb->max.z) {
			aabb->max.z = position.z;
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

alice_camera_3d_t* alice_get_scene_camera_3d(alice_scene_t* scene) {
	assert(scene);

	for (alice_entity_iter(scene, iter, alice_camera_3d_t)) {
		alice_camera_3d_t* entity = iter.current_ptr;
		if (entity->active) {
			return (alice_camera_3d_t*)entity;
		}
	}

	return alice_null;
}

alice_m4f_t alice_get_camera_3d_view(alice_scene_t* scene, alice_camera_3d_t* camera) {
	assert(scene);
	assert(camera);

	alice_v3f_t rotation = alice_torad_v3f(alice_get_entity_world_rotation(scene, (alice_entity_t*)camera));

	alice_v3f_t position = alice_get_entity_world_position(scene, (alice_entity_t*)camera);

	alice_v3f_t direction = (alice_v3f_t){
		.x = cosf(rotation.x) * sinf(rotation.y),
		.y = sinf(rotation.x),
		.z = cosf(rotation.x) * cosf(rotation.y)
	};

	return alice_m4f_lookat(position,
		(alice_v3f_t){
			.x = position.x + direction.x,
			.y = position.y + direction.y,
			.z = position.z + direction.z
		}, (alice_v3f_t){0.0, 1.0, 0.0});
}

alice_m4f_t alice_get_camera_3d_view_no_direction(alice_scene_t* scene, alice_camera_3d_t* camera) {
	assert(scene);
	assert(camera);


	alice_v3f_t position = alice_get_entity_world_position(scene, (alice_entity_t*)camera);

	return alice_m4f_lookat(position,
		(alice_v3f_t){
			.x = position.x,
			.y = position.y,
			.z = position.z
		}, (alice_v3f_t){0.0, 1.0, 0.0});
}

alice_m4f_t alice_get_camera_3d_projection(alice_camera_3d_t* camera) {
	assert(camera);

	return alice_m4f_persp(camera->fov,
		camera->dimentions.x / camera->dimentions.y,
		camera->near, camera->far);
}

alice_m4f_t alice_get_camera_3d_matrix(alice_scene_t* scene, alice_camera_3d_t* camera) {
	alice_m4f_t projection = alice_get_camera_3d_projection(camera);
	alice_m4f_t view = alice_get_camera_3d_view(scene, camera);

	return alice_m4f_multiply(projection, view);
}

ALICE_API alice_camera_2d_t* alice_get_scene_camera_3d_2d(alice_scene_t* scene) {
	assert(scene);

	for (alice_entity_iter(scene, iter, alice_camera_2d_t)) {
		alice_camera_2d_t* entity = iter.current_ptr;
		if (entity->active) {
			return (alice_camera_2d_t*)entity;
		}
	}

	return alice_null;
}

ALICE_API alice_m4f_t alice_get_camera_2d_matrix(alice_scene_t* scene, alice_camera_2d_t* camera) {
	assert(scene);
	assert(camera);

	alice_m4f_t translation = alice_m4f_translate(alice_m4f_identity(), camera->base.position);

	alice_m4f_t projection = alice_m4f_ortho(0.0f, camera->dimentions.x,
			camera->dimentions.y, 0.0f, -1.0f, 1.0f);

	return alice_m4f_multiply(translation, projection);
}

static void alice_apply_pbr_material(alice_shader_t* shader, alice_pbr_material_t* material) {
	assert(shader);
	assert(material);

	alice_shader_set_color(shader, "material.albedo", material->albedo);
	alice_shader_set_float(shader, "material.metallic", material->metallic);
	alice_shader_set_float(shader, "material.roughness", material->roughness);
	alice_shader_set_float(shader, "material.emissive", material->emissive);

	if (material->albedo_map) {
		alice_shader_set_int(shader, "material.use_albedo_map", 1);
		alice_shader_set_int(shader, "material.albedo_map", 0);
		alice_bind_texture(material->albedo_map, 0);
	} else {
		alice_shader_set_int(shader, "material.use_albedo_map", 0);
	}

	if (material->normal_map) {
		alice_shader_set_int(shader, "material.use_normal_map", 1);
		alice_shader_set_int(shader, "material.normal_map", 1);
		alice_bind_texture(material->normal_map, 1);
	} else {
		alice_shader_set_int(shader, "material.use_normal_map", 0);
	}	

	if (material->metallic_map) {
		alice_shader_set_int(shader, "material.use_metallic_map", 1);
		alice_shader_set_int(shader, "material.metallic_map", 2);
		alice_bind_texture(material->metallic_map, 2);
	} else {
		alice_shader_set_int(shader, "material.use_metallic_map", 0);
	}

	if (material->roughness_map) {
		alice_shader_set_int(shader, "material.use_roughness_map", 1);
		alice_shader_set_int(shader, "material.roughness_map", 3);
		alice_bind_texture(material->roughness_map, 3);
	} else {
		alice_shader_set_int(shader, "material.use_roughness_map", 0);
	}

	if (material->ambient_occlusion_map) {
		alice_shader_set_int(shader, "material.use_ambient_occlusion_map", 1);
		alice_shader_set_int(shader, "material.ambient_occlusion_map", 4);
		alice_bind_texture(material->ambient_occlusion_map, 4);
	} else {
		alice_shader_set_int(shader, "material.use_ambient_occlusion_map", 0);
	}

	if (material->emissive_map) {
		alice_shader_set_int(shader, "material.use_emissive_map", 1);
		alice_shader_set_int(shader, "material.ambient_occlusion_map", 5);
		alice_bind_texture(material->emissive_map, 5);
	} else {
		alice_shader_set_int(shader, "material.use_emissive_map", 0);
	}
}

static void alice_apply_phong_material(alice_shader_t* shader, alice_phong_material_t* material) {
	assert(shader);
	assert(material);

	alice_shader_set_color(shader, "material.diffuse", material->diffuse);
	alice_shader_set_color(shader, "material.ambient", material->ambient);
	alice_shader_set_color(shader, "material.specular", material->specular);

	alice_shader_set_float(shader, "material.shininess", material->shininess);
	alice_shader_set_float(shader, "material.emissive", material->emissive);

	if (material->diffuse_map) {
		alice_shader_set_int(shader, "material.use_diffuse_map", 1);
		alice_shader_set_int(shader, "material.diffuse_map", 2);
		alice_bind_texture(material->diffuse_map, 2);
	} else {
		alice_shader_set_int(shader, "material.use_diffuse_map", 0);
	}
}

void alice_apply_material(alice_scene_t* scene, alice_material_t* material) {
	assert(material);

	if (!material->shader) {
		alice_log_warning("Attempting to render object who's material doesn't have a shader");
		return;
	}

	alice_bind_shader(material->shader);

	switch (material->type) {
		case ALICE_MATERIAL_PBR:
			alice_apply_pbr_material(material->shader, &material->as.pbr);
			break;
		case ALICE_MATERIAL_PHONG:
			alice_apply_phong_material(material->shader, &material->as.phong);
			break;
		default: break;
	}

	/* Apply directional lights */
	u32 light_count = 0;
	for (alice_entity_iter(scene, iter, alice_directional_light_t)) {
		alice_directional_light_t* light = iter.current_ptr;

		char name[256];

		sprintf(name, "directional_lights[%d].color", light_count);
		alice_shader_set_color(material->shader, name, light->color);

		sprintf(name, "directional_lights[%d].direction", light_count);
		alice_shader_set_v3f(material->shader, name, light->base.position);

		sprintf(name, "directional_lights[%d].intensity", light_count);
		alice_shader_set_float(material->shader, name, light->intensity);

		sprintf(name, "directional_lights[%d].transform", light_count);
		alice_shader_set_m4f(material->shader, name, light->transform);

		light_count++;
	}

	alice_shader_set_uint(material->shader, "directional_light_count", light_count);
}

void alice_apply_point_lights(alice_scene_t* scene, alice_aabb_t mesh_aabb, alice_material_t* material) {
	assert(scene);
	assert(material);

	u32 light_count = 0;
	for (alice_entity_iter(scene, iter, alice_point_light_t)) {
		alice_point_light_t* light = iter.current_ptr;

		alice_v3f_t world_position = alice_get_entity_world_position(scene, (alice_entity_t*)light);

		if (!alice_sphere_vs_aabb(mesh_aabb, world_position, light->range * 5.0f)) {
			continue;
		}

		char name[256];

		sprintf(name, "point_lights[%d].color", light_count);
		alice_shader_set_color(material->shader, name, light->color);

		sprintf(name, "point_lights[%d].position", light_count);
		alice_shader_set_v3f(material->shader, name, world_position);

		sprintf(name, "point_lights[%d].intensity", light_count);
		alice_shader_set_float(material->shader, name, light->intensity);

		sprintf(name, "point_lights[%d].range", light_count);
		alice_shader_set_float(material->shader, name, light->range);

		sprintf(name, "point_lights[%d].cast_shadows", light_count);
		alice_shader_set_int(material->shader, name, light->cast_shadows);

		light_count++;
	}

	alice_shader_set_uint(material->shader, "point_light_count", light_count);
}

void alice_on_renderable_3d_create(alice_scene_t* scene, alice_entity_handle_t handle, void* ptr) {
	alice_renderable_3d_t* renderable = ptr;

	renderable->materials = alice_null;
	renderable->material_count = 0;
	renderable->material_capacity = 0;

	renderable->model = alice_null;

	renderable->cast_shadows = true;
}

void alice_on_renderable_3d_destroy(alice_scene_t* scene, alice_entity_handle_t handle, void* ptr) {
	alice_renderable_3d_t* renderable = ptr;

	if (renderable->material_capacity > 0) {
		free(renderable->materials);
	}
}

void alice_renderable_3d_add_material(alice_renderable_3d_t* renderable, const char* material_path) {
	assert(renderable);

	if (renderable->material_count >= renderable->material_capacity) {
		renderable->material_capacity = alice_grow_capacity(renderable->material_capacity);
		renderable->materials = realloc(renderable->materials, renderable->material_capacity * sizeof(alice_material_t*));
	}

	renderable->materials[renderable->material_count++] = alice_load_material(material_path);
}

alice_shadowmap_t* alice_new_shadowmap(u32 res, alice_shader_t* shader) {
	alice_shadowmap_t* new = malloc(sizeof(alice_shadowmap_t));

	new->shader = shader;
	new->res = res;

	glGenFramebuffers(1, &new->framebuffer);

	glGenTextures(1, &new->output);
	glBindTexture(GL_TEXTURE_2D, new->output);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
			res, res, 0, GL_DEPTH_COMPONENT,
			GL_FLOAT, alice_null);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	float border_color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);

	glBindFramebuffer(GL_FRAMEBUFFER, new->framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, new->output, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return new;
}

void alice_free_shadowmap(alice_shadowmap_t* shadowmap) {
	assert(shadowmap);

	glDeleteTextures(1, &shadowmap->output);
	glDeleteFramebuffers(1, &shadowmap->framebuffer);

	free(shadowmap);
}

void alice_draw_shadowmap(alice_shadowmap_t* shadowmap, alice_scene_t* scene, alice_camera_3d_t* camera) {
	assert(shadowmap);

	shadowmap->in_use = false;

	shadowmap->draw_call_count = 0;

	alice_directional_light_t* light = alice_null;

	for (alice_entity_iter(scene, iter, alice_directional_light_t)) {
		alice_directional_light_t* entity = iter.current_ptr;

		if (entity->cast_shadows) {
			light = entity;
			break;
		}
	}

	if (!light) { return; }

	shadowmap->in_use = true;

	glViewport(0, 0, shadowmap->res, shadowmap->res);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowmap->framebuffer);
	glClear(GL_DEPTH_BUFFER_BIT);

	alice_m4f_t light_view = alice_m4f_lookat(
			(alice_v3f_t) {
				-light->base.position.x,
				-light->base.position.y,
				-light->base.position.z },
			(alice_v3f_t) { 0.0f, 0.0f, 0.0f },
			(alice_v3f_t) { 0.0f, 1.0f, 0.0f });

	alice_aabb_t scene_aabb = alice_compute_scene_aabb(scene);
	scene_aabb = alice_transform_aabb(scene_aabb, alice_m4f_inverse(light_view));

	alice_m4f_t light_projection = alice_m4f_ortho(
			scene_aabb.min.x, scene_aabb.max.x,
			scene_aabb.min.y, scene_aabb.max.y,
			scene_aabb.min.z, scene_aabb.max.z);
	alice_m4f_t light_matrix = alice_m4f_multiply(light_projection, light_view);

	light->transform = light_matrix;

	alice_bind_shader(shadowmap->shader);
	alice_shader_set_m4f(shadowmap->shader, "light", light_matrix);

	for (alice_entity_iter(scene, iter, alice_renderable_3d_t)) {
		alice_renderable_3d_t* renderable = iter.current_ptr;

		alice_model_t* model = renderable->model;
		if (!model || !renderable->cast_shadows) {
			continue;
		}

		alice_m4f_t transform_matrix = renderable->base.transform;

		for (u32 i = 0; i < model->mesh_count; i++) {
			alice_mesh_t* mesh = &model->meshes[i];
			alice_vertex_buffer_t* vb = mesh->vb;

			alice_m4f_t model = alice_m4f_multiply(transform_matrix, mesh->transform);
			alice_shader_set_m4f(shadowmap->shader, "transform", model);

			alice_bind_vertex_buffer_for_draw(vb);
			alice_draw_vertex_buffer(vb);
			shadowmap->draw_call_count++;
		}
	}
}

void alice_bind_shadowmap_output(alice_shadowmap_t* shadowmap, u32 unit) {
	assert(shadowmap);

	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, shadowmap->output);
}

ALICE_API alice_point_shadowmap_t* alice_new_point_shadowmap(u32 res, alice_shader_t* shader) {
	alice_point_shadowmap_t* new = malloc(sizeof(alice_point_light_t));

	new->shader = shader;
	new->res = res;

	glGenFramebuffers(1, &new->framebuffer);

	glGenTextures(1, &new->cubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, new->cubemap);
	for (u32 i = 0; i < 6; i++) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
				res, res, 0, GL_DEPTH_COMPONENT,
				GL_FLOAT, alice_null);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, new->framebuffer);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, new->cubemap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return new;
}

ALICE_API void alice_free_point_shadowmap(alice_point_shadowmap_t* shadowmap) {
	assert(shadowmap);
	
	glDeleteTextures(1, &shadowmap->cubemap);
	glDeleteFramebuffers(1, &shadowmap->framebuffer);

	free(shadowmap);
}

ALICE_API void alice_draw_point_shadowmap(alice_point_shadowmap_t* shadowmap,
		alice_scene_t* scene) {

	assert(shadowmap);
	assert(scene);

	shadowmap->in_use = false;

	alice_point_light_t* light = alice_null;

	for (alice_entity_iter(scene, iter, alice_point_light_t)) {
		alice_point_light_t* entity = iter.current_ptr;

		if (entity->cast_shadows) {
			light = entity;
			break;
		}
	}

	if (!light) { return; }

	alice_v3f_t light_pos = alice_get_entity_world_position(scene, (alice_entity_t*)light);

	shadowmap->in_use = true;

	glViewport(0, 0, shadowmap->res, shadowmap->res);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowmap->framebuffer);
	glClear(GL_DEPTH_BUFFER_BIT);

	const float near = 0.0f;
	const float far = 25.0f;

	alice_m4f_t shadow_proj = alice_m4f_persp(45.0f, 1.0f, near, far);

	alice_m4f_t shadow_matrices[] = {
		alice_m4f_multiply(shadow_proj, alice_m4f_lookat(light_pos, (alice_v3f_t){light_pos.x + 1.0f, light_pos.y + 0.0f, light_pos.z + 0.0f}, (alice_v3f_t){0.0f, -1.0f,  0.0f})),
		alice_m4f_multiply(shadow_proj, alice_m4f_lookat(light_pos, (alice_v3f_t){light_pos.x - 1.0f, light_pos.y + 0.0f, light_pos.z + 0.0f}, (alice_v3f_t){0.0f, -1.0f,  0.0f})),
		alice_m4f_multiply(shadow_proj, alice_m4f_lookat(light_pos, (alice_v3f_t){light_pos.x + 0.0f, light_pos.y + 1.0f, light_pos.z + 0.0f}, (alice_v3f_t){0.0f,  0.0f,  1.0f})),
		alice_m4f_multiply(shadow_proj, alice_m4f_lookat(light_pos, (alice_v3f_t){light_pos.x + 0.0f, light_pos.y - 1.0f, light_pos.z + 0.0f}, (alice_v3f_t){0.0f,  0.0f, -1.0f})),
		alice_m4f_multiply(shadow_proj, alice_m4f_lookat(light_pos, (alice_v3f_t){light_pos.x + 0.0f, light_pos.y + 0.0f, light_pos.z + 1.0f}, (alice_v3f_t){0.0f, -1.0f,  0.0f})),
		alice_m4f_multiply(shadow_proj, alice_m4f_lookat(light_pos, (alice_v3f_t){light_pos.x + 0.0f, light_pos.y + 0.0f, light_pos.z - 1.0f}, (alice_v3f_t){0.0f, -1.0f,  0.0f}))
	};

	alice_bind_shader(shadowmap->shader);

	for (u32 i = 0; i < 6; i++) {
		char name[32];
		sprintf(name, "shadow_matrices[%d]", i);
		alice_shader_set_m4f(shadowmap->shader, name, shadow_matrices[i]);
	}

	alice_shader_set_float(shadowmap->shader, "far", far);
	alice_shader_set_v3f(shadowmap->shader, "light_position", light_pos);

	for (alice_entity_iter(scene, iter, alice_renderable_3d_t)) {
		alice_renderable_3d_t* renderable = iter.current_ptr;

		alice_model_t* model = renderable->model;
		if (!model || !renderable->cast_shadows) {
			continue;
		}

		alice_m4f_t transform_matrix = renderable->base.transform;

		for (u32 i = 0; i < model->mesh_count; i++) {
			alice_mesh_t* mesh = &model->meshes[i];
			alice_vertex_buffer_t* vb = mesh->vb;

			alice_m4f_t model = alice_m4f_multiply(transform_matrix, mesh->transform);
			alice_shader_set_m4f(shadowmap->shader, "transform", model);

			alice_bind_vertex_buffer_for_draw(vb);
			alice_draw_vertex_buffer(vb);
		}
	}
}

ALICE_API void alice_bind_point_shadowmap_output(alice_point_shadowmap_t* shadowmap, u32 unit) {
	assert(shadowmap);

	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, shadowmap->cubemap);
}

alice_scene_renderer_3d_t* alice_new_scene_renderer_3d(alice_shader_t* postprocess_shader,
	alice_shader_t* extract_shader, alice_shader_t* blur_shader, alice_shader_t* depth_shader,
	alice_shader_t* point_depth_shader, bool debug, alice_shader_t* debug_shader,
	u32 shadowmap_resolution) {

	assert(postprocess_shader);
	assert(extract_shader);
	assert(blur_shader);
	assert(depth_shader);
	assert(point_depth_shader);

	alice_scene_renderer_3d_t* new = malloc(sizeof(alice_scene_renderer_3d_t));

	new->draw_call_count = 0;

	new->output = alice_new_render_target(128, 128, 1);
	new->bright_pixels = alice_new_render_target(128, 128, 1);
	new->bloom_ping_pong[0] = alice_new_render_target(128, 128, 1);
	new->bloom_ping_pong[1] = alice_new_render_target(128, 128, 1);

	new->postprocess = postprocess_shader;
	new->extract = extract_shader;
	new->blur = blur_shader;
	new->shadowmap = alice_new_shadowmap(shadowmap_resolution, depth_shader);
	new->point_shadowmap = alice_new_point_shadowmap(1024, point_depth_shader);

	new->use_bloom = false;
	new->bloom_threshold = 100.0f;
	new->bloom_blur_iterations = 10;

	new->use_antialiasing = false;

	new->debug = debug;
	if (debug && debug_shader) {
		new->debug_renderer = alice_new_debug_renderer(debug_shader);
	}

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

void alice_free_scene_renderer_3d(alice_scene_renderer_3d_t* renderer) {
	assert(renderer);

	alice_free_render_target(renderer->output);
	alice_free_render_target(renderer->bright_pixels);
	alice_free_render_target(renderer->bloom_ping_pong[0]);
	alice_free_render_target(renderer->bloom_ping_pong[1]);
	alice_free_vertex_buffer(renderer->quad);

	alice_free_shadowmap(renderer->shadowmap);
	alice_free_point_shadowmap(renderer->point_shadowmap);

	if (renderer->debug) {
		alice_free_debug_renderer(renderer->debug_renderer);
	}

	free(renderer);
}

alice_aabb_t alice_transform_aabb(alice_aabb_t aabb, alice_m4f_t m) {
	alice_v3f_t corners[]  = {
		aabb.min,
		(alice_v3f_t) { aabb.min.x, aabb.max.y, aabb.min.z },
		(alice_v3f_t) { aabb.min.x, aabb.max.y, aabb.max.z },
		(alice_v3f_t) { aabb.min.x, aabb.min.y, aabb.max.z },
		(alice_v3f_t) { aabb.max.x, aabb.min.y, aabb.min.z },
		(alice_v3f_t) { aabb.max.x, aabb.max.y, aabb.min.z },
		aabb.max,
		(alice_v3f_t) { aabb.max.x, aabb.min.y, aabb.max.z }
	};

	alice_v3f_t tmin = (alice_v3f_t) { 0.0f, 0.0f, 0.0f };
	alice_v3f_t tmax = tmin;

	for (u32 i = 0; i < 8; i++) {
		alice_v4f_t point_4 =
			alice_v4f_transform((alice_v4f_t) { corners[i].x, corners[i].y, corners[i].z, 1.0f }, m);

		alice_v3f_t point = (alice_v3f_t) { point_4.x, point_4.y, point_4.z };

		if (point.x < tmin.x) {
			tmin.x = point.x;
		}

		if (point.y < tmin.y) {
			tmin.y = point.y;
		}

		if (point.z < tmin.z) {
			tmin.z = point.z;
		}

		if (point.x > tmax.x) {
			tmax.x = point.x;
		}

		if (point.y > tmax.y) {
			tmax.y = point.y;
		}

		if (point.z > tmax.z) {
			tmax.z = point.z;
		}
	}

	return (alice_aabb_t) {
		.min = tmin,
		.max = tmax
	};
}

alice_aabb_t alice_compute_scene_aabb(alice_scene_t* scene) {
	assert(scene);

	alice_aabb_t result = (alice_aabb_t) {
		.min = {INFINITY, INFINITY, INFINITY},
		.max = {-INFINITY, -INFINITY, -INFINITY}
	};

	for (alice_entity_iter(scene, iter, alice_renderable_3d_t)) {
		alice_renderable_3d_t* renderable = iter.current_ptr;

		alice_m4f_t transform_matrix = renderable->base.transform;

		alice_model_t* model = renderable->model;
		if (!model) {
			continue;
		}

		for (u32 i = 0; i < model->mesh_count; i++) {
			alice_mesh_t* mesh = &model->meshes[i];

			alice_m4f_t mesh_transform = alice_m4f_multiply(transform_matrix, mesh->transform);

			alice_aabb_t mesh_aabb = alice_transform_aabb(mesh->aabb, mesh_transform);
			mesh_aabb.min.x += mesh_transform.elements[3][0];
			mesh_aabb.min.y += mesh_transform.elements[3][1];
			mesh_aabb.min.z += mesh_transform.elements[3][2];
			mesh_aabb.max.x += mesh_transform.elements[3][0];
			mesh_aabb.max.y += mesh_transform.elements[3][1];
			mesh_aabb.max.z += mesh_transform.elements[3][2];

			result.min.x = alice_min(result.min.x, mesh_aabb.min.x);
			result.min.y = alice_min(result.min.y, mesh_aabb.min.y);
			result.min.z = alice_min(result.min.z, mesh_aabb.min.z);
			result.max.x = alice_max(result.max.x, mesh_aabb.max.x);
			result.max.y = alice_max(result.max.y, mesh_aabb.max.y);
			result.max.z = alice_max(result.max.z, mesh_aabb.max.z);
		}
	}

	return result;
}

void alice_render_scene_3d(alice_scene_renderer_3d_t* renderer, u32 width, u32 height,
		alice_scene_t* scene, alice_render_target_t* render_target) {
	assert(renderer);
	assert(scene);

	renderer->draw_call_count = 0;

	alice_camera_3d_t* camera = alice_get_scene_camera_3d(scene);
	if (!camera) {
		alice_log_warning("Attempting 3D scene render with no active 3D camera");
		return;
	}

	camera->dimentions = (alice_v2f_t){(float)width, (float)height};

	alice_enable_depth();

	alice_draw_shadowmap(renderer->shadowmap, scene, camera);

	renderer->draw_call_count += renderer->shadowmap->draw_call_count;

	alice_resize_render_target(renderer->output, width, height);
	alice_bind_render_target(renderer->output, width, height);

	if (renderer->debug) {
		renderer->debug_renderer->scene = scene;
		renderer->debug_renderer->camera = camera;
	}

	alice_m4f_t camera_matrix = alice_get_camera_3d_matrix(scene, camera);

	for (alice_entity_iter(scene, iter, alice_renderable_3d_t)) {
		alice_renderable_3d_t* renderable = iter.current_ptr;

		alice_m4f_t transform_matrix = renderable->base.transform;

		alice_model_t* model = renderable->model;
		if (!model) {
			continue;
		}

		for (u32 i = 0; i < model->mesh_count; i++) {
			alice_mesh_t* mesh = &model->meshes[i];
			alice_vertex_buffer_t* vb = mesh->vb;

			alice_material_t* material = alice_null;
			if (i < renderable->material_count) {
				material = renderable->materials[i];
			} else if (renderable->material_count == 1) {
				material = renderable->materials[0];
			}

			if (!material) {
				alice_log_warning("Attempting to render object that doesn't have any materials.");
				goto renderable_iter_continue;
			}

			alice_m4f_t mesh_transform = alice_m4f_multiply(transform_matrix, mesh->transform);

			alice_aabb_t mesh_aabb = alice_transform_aabb(mesh->aabb, mesh_transform);
			mesh_aabb.min.x += mesh_transform.elements[3][0];
			mesh_aabb.min.y += mesh_transform.elements[3][1];
			mesh_aabb.min.z += mesh_transform.elements[3][2];
			mesh_aabb.max.x += mesh_transform.elements[3][0];
			mesh_aabb.max.y += mesh_transform.elements[3][1];
			mesh_aabb.max.z += mesh_transform.elements[3][2];

			alice_apply_material(scene, material);
			alice_apply_point_lights(scene, mesh_aabb, material);

			alice_shader_t* shader = material->shader;

			alice_shader_set_color(shader, "ambient_color", renderer->ambient_color);
			alice_shader_set_int(shader, "use_shadows", renderer->shadowmap->in_use);
			alice_shader_set_float(shader, "ambient_intensity", renderer->ambient_intensity);

			alice_shader_set_int(shader, "shadowmap", 8);
			alice_bind_shadowmap_output(renderer->shadowmap, 8);

			alice_m4f_t model = alice_m4f_multiply(transform_matrix, mesh->transform);

			alice_shader_set_m4f(shader, "transform", model);
			alice_shader_set_v3f(shader, "camera_position",
					alice_get_entity_world_position(scene, (alice_entity_t*)camera));
			alice_shader_set_float(shader, "gamma", camera->gamma);
			alice_shader_set_m4f(shader, "camera", camera_matrix);

			alice_bind_vertex_buffer_for_draw(vb);
			alice_draw_vertex_buffer(vb);

			renderer->draw_call_count++;
		}

renderable_iter_continue:
		continue;
	}

	alice_disable_depth();

	if (renderer->debug) {
		alice_aabb_t scene_aabb = alice_compute_scene_aabb(scene);
		alice_debug_renderer_draw_aabb(renderer->debug_renderer, scene_aabb);

		for (alice_entity_iter(scene, iter, alice_renderable_3d_t)) {
			alice_renderable_3d_t* renderable = iter.current_ptr;

			alice_m4f_t transform_matrix = renderable->base.transform;

			alice_model_t* model = renderable->model;
			if (!model) {
				continue;
			}

			for (u32 i = 0; i < model->mesh_count; i++) {
				alice_mesh_t* mesh = &model->meshes[i];

				alice_m4f_t mesh_transform = alice_m4f_multiply(transform_matrix, mesh->transform);

				alice_aabb_t mesh_aabb = alice_transform_aabb(mesh->aabb, mesh_transform);
				mesh_aabb.min.x += mesh_transform.elements[3][0];
				mesh_aabb.min.y += mesh_transform.elements[3][1];
				mesh_aabb.min.z += mesh_transform.elements[3][2];
				mesh_aabb.max.x += mesh_transform.elements[3][0];
				mesh_aabb.max.y += mesh_transform.elements[3][1];
				mesh_aabb.max.z += mesh_transform.elements[3][2];

				alice_debug_renderer_draw_aabb(renderer->debug_renderer, mesh_aabb);
			}
		}
	}

	alice_unbind_render_target(renderer->output);

	alice_render_target_t* bloom_output;
	if (renderer->use_bloom) {
		/* Draw bright areas to bloom target */
		alice_resize_render_target(renderer->bright_pixels, width, height);
		alice_bind_render_target(renderer->bright_pixels, width, height);

		alice_bind_shader(renderer->extract);

		alice_render_target_bind_output(renderer->output, 0, 0);
		alice_shader_set_int(renderer->extract, "input_color", 0);

		alice_shader_set_float(renderer->extract, "input_width", (float)renderer->output->width);
		alice_shader_set_float(renderer->extract, "input_height", (float)renderer->output->height);

		alice_shader_set_float(renderer->extract, "threshold", renderer->bloom_threshold);

		alice_bind_vertex_buffer_for_draw(renderer->quad);
		alice_draw_vertex_buffer(renderer->quad);
		alice_bind_vertex_buffer_for_draw(alice_null);

		renderer->draw_call_count++;

		alice_unbind_render_target(renderer->bright_pixels);

		/* Blur the bloom target */
		bool horizontal = true;
		bool first_iteration = true;
		for (u32 i = 0; i < renderer->bloom_blur_iterations; i++) {
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

			alice_shader_set_float(renderer->blur, "input_width",
					(float)renderer->bright_pixels->width);
			alice_shader_set_float(renderer->blur, "input_height",
					(float)renderer->bright_pixels->height);

			alice_bind_vertex_buffer_for_draw(renderer->quad);
			alice_draw_vertex_buffer(renderer->quad);
			alice_bind_vertex_buffer_for_draw(alice_null);

			renderer->draw_call_count++;

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

	alice_shader_set_float(renderer->postprocess, "input_width", (float)renderer->output->width);
	alice_shader_set_float(renderer->postprocess, "input_height", (float)renderer->output->height);
	alice_shader_set_float(renderer->postprocess, "exposure", camera->exposure);
	alice_shader_set_float(renderer->postprocess, "gamma", camera->gamma);

	alice_shader_set_color(renderer->postprocess, "color_mod", renderer->color_mod);

	alice_bind_vertex_buffer_for_draw(renderer->quad);
	alice_draw_vertex_buffer(renderer->quad);
	alice_bind_vertex_buffer_for_draw(alice_null);

	renderer->draw_call_count++;

	alice_bind_shader(alice_null);

	if (render_target) {
		alice_unbind_render_target(render_target);
	}
}

alice_3d_pick_context_t* alice_new_3d_pick_context(alice_shader_t* shader) {
	assert(shader);

	alice_3d_pick_context_t* new = malloc(sizeof(alice_3d_pick_context_t));

	new->shader = shader;
	new->target = alice_new_render_target(128, 128, 1);

	return new;
}

void alice_free_3d_pick_context(alice_3d_pick_context_t* context) {
	assert(context);

	alice_free_render_target(context->target);

	free(context);
}

alice_entity_handle_t alice_3d_pick(alice_3d_pick_context_t* context, alice_scene_t* scene) {
	assert(context);
	assert(scene);

	alice_camera_3d_t* camera = alice_get_scene_camera_3d(scene);
	if (!camera) {
		alice_log_warning("Attempting 3D scene render with no active 3D camera");
		return alice_null_entity_handle;
	}

	alice_enable_depth();

	alice_resize_render_target(context->target, camera->dimentions.x, camera->dimentions.y);
	alice_bind_render_target(context->target, camera->dimentions.x, camera->dimentions.y);

	alice_m4f_t camera_matrix = alice_get_camera_3d_matrix(scene, camera);

	alice_shader_t* shader = context->shader;
	alice_bind_shader(shader);
	alice_shader_set_m4f(shader, "camera", camera_matrix);

	for (alice_entity_iter(scene, iter, alice_renderable_3d_t)) {
		alice_renderable_3d_t* renderable = iter.current_ptr;

		alice_m4f_t transform_matrix = renderable->base.transform;

		alice_model_t* model = renderable->model;
		if (!model) {
			continue;
		}

		alice_entity_handle_t entity_id = alice_get_entity_handle_id(iter.current) + 1;

		for (u32 i = 0; i < model->mesh_count; i++) {
			alice_mesh_t* mesh = &model->meshes[i];
			alice_vertex_buffer_t* vb = mesh->vb;

			alice_m4f_t mesh_transform = alice_m4f_multiply(transform_matrix, mesh->transform);

			alice_m4f_t model = alice_m4f_multiply(transform_matrix, mesh->transform);

			i32 r = (entity_id & 0x000000FF) >> 0;
			i32 g = (entity_id & 0x0000FF00) >> 8;
			i32 b = (entity_id & 0x00FF0000) >> 16;

			alice_shader_set_m4f(shader, "transform", model);
			alice_shader_set_v3f(shader, "object", (alice_v3f_t){
					(float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f});

			alice_bind_vertex_buffer_for_draw(vb);
			alice_draw_vertex_buffer(vb);
		}
	}

	alice_disable_depth();

	alice_v2i_t mouse_pos = alice_get_mouse_position();

	glReadBuffer(GL_COLOR_ATTACHMENT0);

	u8 data[3];
	glReadPixels(mouse_pos.x, camera->dimentions.y - mouse_pos.y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &data);

	u32 picked_id = 
		data[0] + 
		data[1] * 256 +
		data[2] * 256 * 256;

	alice_entity_handle_t picked_entity = alice_null_entity_handle;

	if (picked_id != 0) {
		picked_entity = alice_new_entity_handle(picked_id - 1,
				alice_get_type_info(alice_renderable_3d_t).id);
	}

	glReadBuffer(GL_NONE);

	alice_unbind_render_target(context->target);

	return picked_entity;
}

alice_v3f_t alice_get_sprite_2d_world_position(alice_scene_t* scene, alice_entity_t* entity) {
	assert(scene);
	assert(entity);

	alice_v3f_t result = entity->position;

	if (entity->parent != alice_null_entity_handle) {
		alice_entity_t* parent_ptr = alice_get_entity_ptr(scene, entity->parent);

		alice_v3f_t parent_position = alice_get_sprite_2d_world_position(scene, parent_ptr);

		result = (alice_v3f_t) {
			result.x + parent_position.x,
			result.y + parent_position.y,
			result.z + parent_position.z,
		};
	}

	return result;
}

alice_scene_renderer_2d_t* alice_new_scene_renderer_2d(alice_shader_t* sprite_shader) {
	assert(sprite_shader);

	alice_scene_renderer_2d_t* new = malloc(sizeof(alice_scene_renderer_2d_t));

	new->sprite_shader = sprite_shader;

	alice_vertex_buffer_t* buffer = alice_new_vertex_buffer(
			ALICE_VERTEXBUFFER_DRAW_TRIANGLES | ALICE_VERTEXBUFFER_DYNAMIC_DRAW);

	alice_bind_vertex_buffer_for_edit(buffer);
	alice_push_vertices(buffer, alice_null, (10 * 4) * 10000);
	alice_push_indices(buffer, alice_null, 6 * 10000);
	alice_configure_vertex_buffer(buffer, 0, 2, 11, 0); /* vec2 position */
	alice_configure_vertex_buffer(buffer, 1, 2, 11, 2); /* vec2 uv */
	alice_configure_vertex_buffer(buffer, 2, 4, 11, 4); /* vec4 source_rect */
	alice_configure_vertex_buffer(buffer, 3, 2, 11, 8); /* vec2 texture_size */
	alice_configure_vertex_buffer(buffer, 4, 1, 11, 10); /* float texture_size */
	alice_bind_vertex_buffer_for_edit(alice_null);

	new->quad = buffer;

	return new;
}

void alice_free_scene_renderer_2d(alice_scene_renderer_2d_t* renderer) {
	assert(renderer);

	alice_free_vertex_buffer(renderer->quad);

	free(renderer);
}

static void alice_scene_renderer_2d_push_quad(alice_scene_renderer_2d_t* renderer,
		u32* quad_count,
		u32* texture_count,
		alice_texture_t** used_textures,
		alice_v3f_t position, alice_v3f_t scale, alice_v4f_t source_rect,
		alice_texture_t* texture) {
	i32 texture_index = -1;
	for (u32 i = 0; i < *texture_count; i++) {
		if (used_textures[i] == texture) {
			texture_index = i;
		}
	}

	if (texture_index == -1) {
		used_textures[*texture_count] = texture;
		texture_index = *texture_count;

		alice_bind_texture(texture, *texture_count);

		*texture_count = *texture_count + 1;

		if (*texture_count >= 32) {
			alice_log_warning("Only 32 textures are allowed to render.");
			return;
		}
	}

	alice_v4f_t s = {
		.x = source_rect.x,
		.y = texture->height - (source_rect.y + source_rect.w),
		.z = source_rect.z,
		.w = source_rect.w
	};

	float verts[] = {
		position.x,		position.y,		0.0f, 1.0f, s.x, s.y, s.z, s.w, texture->width, texture->height, (float)texture_index,
		position.x + scale.x,	position.y,		1.0f, 1.0f, s.x, s.y, s.z, s.w, texture->width, texture->height, (float)texture_index,
		position.x + scale.x,	position.y + scale.y,	1.0f, 0.0f, s.x, s.y, s.z, s.w, texture->width, texture->height, (float)texture_index,
		position.x,		position.y + scale.y,	0.0f, 0.0f, s.x, s.y, s.z, s.w, texture->width, texture->height, (float)texture_index,
	};

	const u32 index_offset = *quad_count * 4;

	u32 indices[] = {
		index_offset + 3, index_offset + 2, index_offset + 1,
		index_offset + 3, index_offset + 1, index_offset + 0
	};

	alice_update_vertices(renderer->quad, verts, *quad_count * 11 * 4, 11 * 4);
	alice_update_indices(renderer->quad, indices, *quad_count * 6, 6);

	*quad_count = *quad_count + 1;
}

void alice_render_scene_2d(alice_scene_renderer_2d_t* renderer, u32 width, u32 height,
		alice_scene_t* scene, alice_render_target_t* render_target) {
	assert(renderer);
	assert(scene);

	glViewport(0.0f, 0.0f, width, height);

	alice_camera_2d_t* camera = alice_get_scene_camera_3d_2d(scene);

	if (!camera) {
		alice_log_warning("Attempting to render 2D scene without an active 2D camera");
	}

	if (!camera->stretch) {
		camera->dimentions = (alice_v2f_t) { (float)width, (float)height };
	}

	if (render_target) {
		alice_resize_render_target(render_target, width, height);
		alice_bind_render_target(render_target, width, height);
	}

	u32 quad_count = 0;
	u32 texture_count = 0;

	alice_texture_t* used_textures[32];

	alice_bind_vertex_buffer_for_edit(renderer->quad);

	for (alice_entity_iter(scene, iter, alice_sprite_2d_t)) {
		alice_sprite_2d_t* sprite = iter.current_ptr;

		alice_v3f_t position = alice_get_sprite_2d_world_position(scene, (alice_entity_t*)sprite);
		alice_v3f_t scale = sprite->base.scale;

		alice_v4f_t s = sprite->source_rect;

		alice_scene_renderer_2d_push_quad(renderer,
				&quad_count, &texture_count, used_textures,
				position, scale, s, sprite->image);
	}

	for (alice_entity_iter(scene, iter, alice_tilemap_t)) {
		alice_tilemap_t* tilemap = iter.current_ptr;

		for (u32 x = 0; x < tilemap->dimentions.x; x++) {
			for (u32 y = 0; y < tilemap->dimentions.y; y++) {
				alice_v3f_t position = {
					.x = tilemap->base.position.x +
						x * tilemap->tile_size * tilemap->base.scale.x,
					.y = tilemap->base.position.y +
						y * tilemap->tile_size * tilemap->base.scale.y,
					.z = 0.0f
				};

				alice_v3f_t scale = { 
					tilemap->tile_size * tilemap->base.scale.x,
					tilemap->tile_size * tilemap->base.scale.y,
					1.0f };

				i32 tile_id = tilemap->data[x + y * tilemap->dimentions.x];

				if (tile_id == -1) { continue; }

				alice_v4f_t s = tilemap->tiles[tile_id];

				alice_scene_renderer_2d_push_quad(renderer,
						&quad_count, &texture_count, used_textures,
						position, scale, s, tilemap->texture);
			}
		}
	}

	alice_bind_vertex_buffer_for_edit(alice_null);
	alice_bind_shader(renderer->sprite_shader);

	for (u32 i = 0; i < texture_count; i++) {
		char name[128];
		sprintf(name, "textures[%d]", i);

		alice_shader_set_int(renderer->sprite_shader, name, i);
	}

	alice_shader_set_m4f(renderer->sprite_shader, "camera", alice_get_camera_2d_matrix(scene, camera));

	alice_bind_vertex_buffer_for_draw(renderer->quad);
	alice_draw_vertex_buffer_custom_count(renderer->quad, quad_count * 6);
	alice_bind_vertex_buffer_for_draw(alice_null);

	alice_bind_texture(alice_null, 0);
	alice_bind_shader(alice_null);

	if (render_target) {
		alice_unbind_render_target(render_target);
	}
}

void alice_on_tilemap_create(alice_scene_t* scene, alice_entity_handle_t handle, void* ptr) {
	alice_tilemap_t* tilemap = ptr;

	tilemap->data = alice_null;
	tilemap->tile_size = 16;

	for (u32 i = 0; i < sizeof(tilemap->tiles) / sizeof(*tilemap->tiles); i++) {
		tilemap->tiles[i] = (alice_v4f_t) {
			0.0f, 0.0f, 0.0f, 0.0f
		};
	}

	tilemap->dimentions = (alice_v2u_t) { 0, 0 };

	tilemap->texture = alice_null;
}

void alice_on_tilemap_destroy(alice_scene_t* scene, alice_entity_handle_t handle, void* ptr) {
	alice_tilemap_t* tilemap = ptr;

	if (tilemap->data) {
		free(tilemap->data);
	}
}

void alice_tilemap_set_tile(alice_tilemap_t* tilemap, i32 id, alice_v4f_t tile) {
	assert(tilemap);

	if (id < 0 || id > 255) { return; }
	
	tilemap->tiles[id] = tile;
}

void alice_tilemap_set(alice_tilemap_t* tilemap, alice_v2u_t position, i32 tile) {
	assert(tilemap);

	if (
			position.x > tilemap->dimentions.x ||
			position.y > tilemap->dimentions.y) {
		return;
	}

	tilemap->data[position.x + position.y * tilemap->dimentions.x] = tile;
}

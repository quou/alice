#pragma once

#include "alice/maths.h"
#include "alice/core.h"
#include "alice/resource.h"
#include "alice/entity.h"
#include "alice/physics.h"

typedef struct alice_DebugRenderer alice_DebugRenderer;

typedef struct alice_RGBColor {
	float r, g, b;
} alice_RGBColor;

typedef u32 alice_Color;

#define ALICE_COLOR_WHITE 0xffffff
#define ALICE_COLOR_RED 0xff0000
#define ALICE_COLOR_GREEN 0x00ff00
#define ALICE_COLOR_BLUE 0x0000ff
#define ALICE_COLOR_YELLOW 0xffff00
#define ALICE_COLOR_PINK 0xff00ff
#define ALICE_COLOR_CYAN 0x00ffff

ALICE_API alice_Color alice_color_from_rgb_color(alice_RGBColor rgb);
ALICE_API alice_RGBColor alice_rgb_color_from_color(alice_Color color);

typedef struct alice_Shader {
	u32 id;

	bool panic_mode;
} alice_Shader;

ALICE_API alice_Shader* alice_init_shader(alice_Shader* shader, char* source);
ALICE_API void alice_deinit_shader(alice_Shader* shader);

ALICE_API alice_Shader* alice_new_shader(alice_Resource* resource);
ALICE_API void alice_free_shader(alice_Shader* shader);

ALICE_API void alice_bind_shader(alice_Shader* shader);
ALICE_API void alice_shader_set_int(alice_Shader* shader, const char* name, i32 v);
ALICE_API void alice_shader_set_uint(alice_Shader* shader, const char* name, u32 v);
ALICE_API void alice_shader_set_float(alice_Shader* shader, const char* name, float v);
ALICE_API void alice_shader_set_color(alice_Shader* shader, const char* name, alice_Color color);
ALICE_API void alice_shader_set_rgb_color(alice_Shader* shader, const char* name, alice_RGBColor color);
ALICE_API void alice_shader_set_v2i(alice_Shader* shader, const char* name, alice_v2i v);
ALICE_API void alice_shader_set_v2u(alice_Shader* shader, const char* name, alice_v2u v);
ALICE_API void alice_shader_set_v2f(alice_Shader* shader, const char* name, alice_v2f v);
ALICE_API void alice_shader_set_v3i(alice_Shader* shader, const char* name, alice_v3i v);
ALICE_API void alice_shader_set_v3u(alice_Shader* shader, const char* name, alice_v3u v);
ALICE_API void alice_shader_set_v3f(alice_Shader* shader, const char* name, alice_v3f v);
ALICE_API void alice_shader_set_v4i(alice_Shader* shader, const char* name, alice_v4i v);
ALICE_API void alice_shader_set_v4u(alice_Shader* shader, const char* name, alice_v4u v);
ALICE_API void alice_shader_set_v4f(alice_Shader* shader, const char* name, alice_v4f v);
ALICE_API void alice_shader_set_m4f(alice_Shader* shader, const char* name, alice_m4f v);

typedef enum alice_VertexBufferFlags {
	ALICE_VERTEXBUFFER_STATIC_DRAW = 1 << 0,
	ALICE_VERTEXBUFFER_DYNAMIC_DRAW = 1 << 1,
	ALICE_VERTEXBUFFER_DRAW_LINES = 1 << 2,
	ALICE_VERTEXBUFFER_DRAW_LINE_STRIP = 1 << 3,
	ALICE_VERTEXBUFFER_DRAW_TRIANGLES = 1 << 4,
} alice_VertexBufferFlags;

typedef struct alice_VertexBuffer {
	u32 va_id;
	u32 vb_id;
	u32 ib_id;
	u32 index_count;

	alice_VertexBufferFlags flags;
} alice_VertexBuffer;

ALICE_API alice_VertexBuffer* alice_new_vertex_buffer(alice_VertexBufferFlags flags);
ALICE_API void alice_free_vertex_buffer(alice_VertexBuffer* buffer);
ALICE_API void alice_bind_vertex_buffer_for_draw(alice_VertexBuffer* buffer);
ALICE_API void alice_bind_vertex_buffer_for_edit(alice_VertexBuffer* buffer);
ALICE_API void alice_push_vertices(alice_VertexBuffer* buffer, float* vertices, u32 count);
ALICE_API void alice_push_indices(alice_VertexBuffer* buffer, u32* indices, u32 count);
ALICE_API void alice_update_vertices(alice_VertexBuffer* buffer, float* vertices, u32 offset, u32 count);
ALICE_API void alice_update_indices(alice_VertexBuffer* buffer, u32* indices, u32 offset, u32 count);
ALICE_API void alice_configure_vertex_buffer(alice_VertexBuffer* buffer, u32 index, u32 component_count,
	u32 stride, u32 offset);
ALICE_API void alice_draw_vertex_buffer(alice_VertexBuffer* buffer);
ALICE_API void alice_draw_vertex_buffer_custom_count(alice_VertexBuffer* buffer, u32 count);

ALICE_API void alice_render_clear();
ALICE_API void alice_depth_clear();

ALICE_API void alice_enable_depth();
ALICE_API void alice_disable_depth();

typedef enum alice_TextureFlags {
	ALICE_TEXTURE_ALIASED = 1 << 0,
	ALICE_TEXTURE_ANTIALIASED = 1 << 1,
	ALICE_TEXTURE_NORMAL = 1 << 2,
	ALICE_TEXTURE_HDR = 1 << 3
} alice_TextureFlags;

typedef struct alice_Texture {
	u32 id;
	i32 width, height, component_count;

	alice_Resource* resource;

	alice_TextureFlags flags;
} alice_Texture;

ALICE_API alice_Texture* alice_new_texture(alice_Resource* resource, alice_TextureFlags flags);
ALICE_API alice_Texture* alice_new_texture_from_memory(void* data, u32 size, alice_TextureFlags flags);
ALICE_API alice_Texture* alice_new_texture_from_memory_uncompressed(unsigned char* pixels,
	u32 size, i32 width, i32 height, i32 component_count, alice_TextureFlags flags);
ALICE_API void alice_init_texture(alice_Texture* texture, alice_Resource* resource, alice_TextureFlags flags);
ALICE_API void alice_init_texture_from_memory(alice_Texture* texture, void* data, u32 size, alice_TextureFlags flags);
ALICE_API void alice_init_texture_from_memory_uncompressed(alice_Texture* texture, unsigned char* pixels,
	u32 size, i32 width, i32 height, i32 component_count, alice_TextureFlags flags);
ALICE_API void alice_deinit_texture(alice_Texture* texture);

ALICE_API void alice_free_texture(alice_Texture* texture);
ALICE_API void alice_bind_texture(alice_Texture* texture, u32 slot);

typedef struct alice_Rect {
	float x, y, w, h;
} alice_Rect;

typedef struct alice_RenderTarget {
	u32 frame_buffer, render_buffer;

	u32* color_attachments;
	u32 color_attachment_count;

	u32 width, height;

	u32 old_width, old_height;
} alice_RenderTarget;

ALICE_API alice_RenderTarget* alice_new_render_target(u32 width, u32 height, u32 color_attachment_count);
ALICE_API void alice_free_render_target(alice_RenderTarget* target);
ALICE_API void alice_bind_render_target(alice_RenderTarget* target, u32 old_width, u32 old_height);
ALICE_API void alice_unbind_render_target(alice_RenderTarget* target);
ALICE_API void alice_resize_render_target(alice_RenderTarget* target, u32 width, u32 height);
ALICE_API void alice_render_target_bind_output(alice_RenderTarget* rt, u32 attachment_index, u32 unit);

typedef struct alice_Mesh {
	alice_m4f transform;

	alice_VertexBuffer* vb;

	alice_AABB aabb;
} alice_Mesh;

ALICE_API void alice_init_mesh(alice_Mesh* mesh, alice_VertexBuffer* vb);
ALICE_API void alice_deinit_mesh(alice_Mesh* mesh);

ALICE_API alice_Mesh alice_new_cube_mesh();
ALICE_API alice_Mesh alice_new_sphere_mesh();

typedef struct alice_Model {
	alice_Mesh* meshes;
	u32 mesh_count;
	u32 mesh_capacity;
} alice_Model;

ALICE_API void alice_init_model(alice_Model* model);
ALICE_API void alice_deinit_model(alice_Model* model);

ALICE_API alice_Model* alice_new_model();
ALICE_API void alice_free_model(alice_Model* model);
ALICE_API void alice_model_add_mesh(alice_Model* model, alice_Mesh mesh);

ALICE_API void alice_calculate_aabb_from_mesh(alice_AABB* aabb, alice_m4f transform,
		float* vertices, u32 position_count, u32 position_stride);

typedef struct alice_Camera3D {
	alice_Entity base;

	alice_v2f dimentions;

	float fov;
	float near;
	float far;

	float exposure;
	float gamma;

	bool active;
} alice_Camera3D;

ALICE_API alice_Camera3D* alice_get_scene_camera(alice_Scene* scene);
ALICE_API alice_m4f alice_get_camera_3d_matrix(alice_Scene* scene, alice_Camera3D* camera);

typedef struct alice_PointLight {
	alice_Entity base;

	alice_Color color;
	float intensity;
	float range;
} alice_PointLight;

typedef struct alice_DirectionalLight {
	alice_Entity base;

	alice_Color color;
	float intensity;
} alice_DirectionalLight;

typedef struct alice_Material {
	alice_Shader* shader;

	alice_Color albedo;
	float metallic;
	float roughness;

	float emissive;

	alice_Texture* albedo_map;
	alice_Texture* normal_map;
	alice_Texture* metallic_map;
	alice_Texture* roughness_map;
	alice_Texture* ambient_occlusion_map;
} alice_Material;

ALICE_API void alice_apply_material(alice_Scene* scene, alice_Material* material);

typedef struct alice_Renderable3D {
	alice_Entity base;

	alice_Material** materials;
	u32 material_count;
	u32 material_capacity;

	alice_Model* model;
} alice_Renderable3D;

ALICE_API void alice_apply_point_lights(alice_Scene* scene, alice_AABB mesh_aabb, alice_Material* material);

ALICE_API void alice_on_renderable_3d_create(alice_Scene* scene, alice_EntityHandle handle, void* ptr);
ALICE_API void alice_on_renderable_3d_destroy(alice_Scene* scene, alice_EntityHandle handle, void* ptr);
ALICE_API void alice_renderable_3d_add_material(alice_Renderable3D* renderable, const char* material_path);

typedef struct alice_SceneRenderer3D {
	alice_RenderTarget* bright_pixels;
	alice_RenderTarget* bloom_ping_pong[2];
	alice_RenderTarget* output;
	alice_VertexBuffer* quad;
	alice_Shader* postprocess;
	alice_Shader* extract;
	alice_Shader* blur;

	bool debug;
	alice_DebugRenderer* debug_renderer;

	bool use_bloom;
	float bloom_threshold;
	u32 bloom_blur_iterations;

	bool use_antialiasing;

	alice_Color color_mod;
} alice_SceneRenderer3D;

ALICE_API alice_SceneRenderer3D* alice_new_scene_renderer_3d(alice_Shader* postprocess_shader,
		alice_Shader* extract_shader, alice_Shader* blur_shader,
		bool debug, alice_Shader* debug_shader);
ALICE_API void alice_free_scene_renderer_3d(alice_SceneRenderer3D* renderer);
ALICE_API void alice_render_scene_3d(alice_SceneRenderer3D* renderer, u32 width, u32 height,
		alice_Scene* scene, alice_RenderTarget* render_target);

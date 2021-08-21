#pragma once

#include "alice/maths.h"
#include "alice/core.h"
#include "alice/resource.h"
#include "alice/entity.h"
#include "alice/physics.h"

typedef struct alice_debug_renderer_t alice_debug_renderer_t;

typedef struct alice_rgb_color_t {
	float r, g, b;
} alice_rgb_color_t;

typedef u32 alice_color_t;

#define ALICE_COLOR_WHITE 0xffffff
#define ALICE_COLOR_RED 0xff0000
#define ALICE_COLOR_GREEN 0x00ff00
#define ALICE_COLOR_BLUE 0x0000ff
#define ALICE_COLOR_YELLOW 0xffff00
#define ALICE_COLOR_PINK 0xff00ff
#define ALICE_COLOR_CYAN 0x00ffff

ALICE_API alice_color_t alice_color_from_rgb_color(alice_rgb_color_t rgb);
ALICE_API alice_rgb_color_t alice_rgb_color_from_color(alice_color_t color);

typedef struct alice_shader_t {
	u32 id;

	bool panic_mode;
} alice_shader_t;

ALICE_API alice_shader_t* alice_init_shader(alice_shader_t* shader, char* source);
ALICE_API void alice_deinit_shader(alice_shader_t* shader);

ALICE_API alice_shader_t* alice_new_shader(alice_resource_t* resource);
ALICE_API void alice_free_shader(alice_shader_t* shader);

ALICE_API void alice_bind_shader(alice_shader_t* shader);
ALICE_API void alice_shader_set_int(alice_shader_t* shader, const char* name, i32 v);
ALICE_API void alice_shader_set_uint(alice_shader_t* shader, const char* name, u32 v);
ALICE_API void alice_shader_set_float(alice_shader_t* shader, const char* name, float v);
ALICE_API void alice_shader_set_color(alice_shader_t* shader, const char* name, alice_color_t color);
ALICE_API void alice_shader_set_rgb_color(alice_shader_t* shader, const char* name, alice_rgb_color_t color);
ALICE_API void alice_shader_set_v2i(alice_shader_t* shader, const char* name, alice_v2i_t v);
ALICE_API void alice_shader_set_v2u(alice_shader_t* shader, const char* name, alice_v2u_t v);
ALICE_API void alice_shader_set_v2f(alice_shader_t* shader, const char* name, alice_v2f_t v);
ALICE_API void alice_shader_set_v3i(alice_shader_t* shader, const char* name, alice_v3i_t v);
ALICE_API void alice_shader_set_v3u(alice_shader_t* shader, const char* name, alice_v3u_t v);
ALICE_API void alice_shader_set_v3f(alice_shader_t* shader, const char* name, alice_v3f_t v);
ALICE_API void alice_shader_set_v4i(alice_shader_t* shader, const char* name, alice_v4i v);
ALICE_API void alice_shader_set_v4u(alice_shader_t* shader, const char* name, alice_v4u v);
ALICE_API void alice_shader_set_v4f(alice_shader_t* shader, const char* name, alice_v4f_t v);
ALICE_API void alice_shader_set_m4f(alice_shader_t* shader, const char* name, alice_m4f_t v);

typedef enum alice_vertex_buffer_flags_t {
	ALICE_VERTEXBUFFER_STATIC_DRAW = 1 << 0,
	ALICE_VERTEXBUFFER_DYNAMIC_DRAW = 1 << 1,
	ALICE_VERTEXBUFFER_DRAW_LINES = 1 << 2,
	ALICE_VERTEXBUFFER_DRAW_LINE_STRIP = 1 << 3,
	ALICE_VERTEXBUFFER_DRAW_TRIANGLES = 1 << 4,
} alice_vertex_buffer_flags_t;

typedef struct alice_vertex_buffer_t {
	u32 va_id;
	u32 vb_id;
	u32 ib_id;
	u32 index_count;

	alice_vertex_buffer_flags_t flags;
} alice_vertex_buffer_t;

ALICE_API alice_vertex_buffer_t* alice_new_vertex_buffer(alice_vertex_buffer_flags_t flags);
ALICE_API void alice_free_vertex_buffer(alice_vertex_buffer_t* buffer);
ALICE_API void alice_bind_vertex_buffer_for_draw(alice_vertex_buffer_t* buffer);
ALICE_API void alice_bind_vertex_buffer_for_edit(alice_vertex_buffer_t* buffer);
ALICE_API void alice_push_vertices(alice_vertex_buffer_t* buffer, float* vertices, u32 count);
ALICE_API void alice_push_indices(alice_vertex_buffer_t* buffer, u32* indices, u32 count);
ALICE_API void alice_update_vertices(alice_vertex_buffer_t* buffer, float* vertices, u32 offset, u32 count);
ALICE_API void alice_update_indices(alice_vertex_buffer_t* buffer, u32* indices, u32 offset, u32 count);
ALICE_API void alice_configure_vertex_buffer(alice_vertex_buffer_t* buffer, u32 index, u32 component_count,
	u32 stride, u32 offset);
ALICE_API void alice_draw_vertex_buffer(alice_vertex_buffer_t* buffer);
ALICE_API void alice_draw_vertex_buffer_custom_count(alice_vertex_buffer_t* buffer, u32 count);

ALICE_API void alice_render_clear();
ALICE_API void alice_depth_clear();

ALICE_API void alice_enable_depth();
ALICE_API void alice_disable_depth();

typedef enum alice_texture_flags_t {
	ALICE_TEXTURE_ALIASED = 1 << 0,
	ALICE_TEXTURE_ANTIALIASED = 1 << 1,
} alice_texture_flags_t;

typedef struct alice_texture_t {
	u32 id;
	i32 width, height, component_count;

	alice_texture_flags_t flags;
} alice_texture_t;

ALICE_API alice_texture_t* alice_new_texture(alice_resource_t* resource, alice_texture_flags_t flags);
ALICE_API alice_texture_t* alice_new_texture_from_memory(void* data, u32 size, alice_texture_flags_t flags);
ALICE_API alice_texture_t* alice_new_texture_from_memory_uncompressed(unsigned char* pixels,
	u32 size, i32 width, i32 height, i32 component_count, alice_texture_flags_t flags);
ALICE_API void alice_init_texture(alice_texture_t* texture, alice_resource_t* resource, alice_texture_flags_t flags);
ALICE_API void alice_init_texture_from_memory(alice_texture_t* texture, void* data, u32 size, alice_texture_flags_t flags);
ALICE_API void alice_init_texture_from_memory_uncompressed(alice_texture_t* texture, unsigned char* pixels,
	u32 size, i32 width, i32 height, i32 component_count, alice_texture_flags_t flags);
ALICE_API void alice_deinit_texture(alice_texture_t* texture);

ALICE_API void alice_free_texture(alice_texture_t* texture);
ALICE_API void alice_bind_texture(alice_texture_t* texture, u32 slot);

typedef struct alice_rect_t {
	float x, y, w, h;
} alice_rect_t;

typedef struct alice_render_target_t {
	u32 frame_buffer, render_buffer;

	u32* color_attachments;
	u32 color_attachment_count;

	u32 width, height;

	u32 old_width, old_height;
} alice_render_target_t;

ALICE_API alice_render_target_t* alice_new_render_target(u32 width, u32 height, u32 color_attachment_count);
ALICE_API void alice_free_render_target(alice_render_target_t* target);
ALICE_API void alice_bind_render_target(alice_render_target_t* target, u32 old_width, u32 old_height);
ALICE_API void alice_unbind_render_target(alice_render_target_t* target);
ALICE_API void alice_resize_render_target(alice_render_target_t* target, u32 width, u32 height);
ALICE_API void alice_render_target_bind_output(alice_render_target_t* rt, u32 attachment_index, u32 unit);

typedef struct alice_mesh_t {
	alice_m4f_t transform;

	alice_vertex_buffer_t* vb;

	alice_aabb_t aabb;
} alice_mesh_t;

ALICE_API void alice_init_mesh(alice_mesh_t* mesh, alice_vertex_buffer_t* vb);
ALICE_API void alice_deinit_mesh(alice_mesh_t* mesh);

ALICE_API alice_mesh_t alice_new_cube_mesh();
ALICE_API alice_mesh_t alice_new_sphere_mesh();

typedef struct alice_model_t {
	alice_mesh_t* meshes;
	u32 mesh_count;
	u32 mesh_capacity;
} alice_model_t;

ALICE_API void alice_init_model(alice_model_t* model);
ALICE_API void alice_deinit_model(alice_model_t* model);

ALICE_API alice_model_t* alice_new_model();
ALICE_API void alice_free_model(alice_model_t* model);
ALICE_API void alice_model_add_mesh(alice_model_t* model, alice_mesh_t mesh);

ALICE_API void alice_calculate_aabb_from_mesh(alice_aabb_t* aabb,
	float* vertices, u32 position_count, u32 position_stride);

typedef struct alice_camera_3d_t {
	alice_entity_t base;

	alice_v2f_t dimentions;

	float fov;
	float near;
	float far;

	float exposure;
	float gamma;

	bool active;
} alice_camera_3d_t;

ALICE_API alice_camera_3d_t* alice_get_scene_camera_3d(alice_scene_t* scene);
ALICE_API alice_m4f_t alice_get_camera_3d_matrix(alice_scene_t* scene, alice_camera_3d_t* camera);
ALICE_API alice_m4f_t alice_get_camera_3d_view(alice_scene_t* scene, alice_camera_3d_t* camera);
ALICE_API alice_m4f_t alice_get_camera_3d_view_no_direction(alice_scene_t* scene, alice_camera_3d_t* camera);
ALICE_API alice_m4f_t alice_get_camera_3d_projection(alice_camera_3d_t* camera);

typedef struct alice_camera_2d_t {
	alice_entity_t base;

	alice_v2f_t dimentions;

	bool stretch;

	bool active;
} alice_camera_2d_t;

ALICE_API alice_camera_2d_t* alice_get_scene_camera_2d(alice_scene_t* scene);
ALICE_API alice_m4f_t alice_get_camera_2d_matrix(alice_scene_t* scene, alice_camera_2d_t* camera);

typedef struct alice_point_light_t {
	alice_entity_t base;

	alice_color_t color;
	float intensity;
	float range;

	bool cast_shadows;
} alice_point_light_t;

typedef struct alice_directional_light_t {
	alice_entity_t base;

	alice_color_t color;
	float intensity;

	bool cast_shadows;

	alice_m4f_t transform;
} alice_directional_light_t;

typedef struct alice_pbr_material_t {
	alice_color_t albedo;
	float metallic;
	float roughness;

	float emissive;

	alice_texture_t* albedo_map;
	alice_texture_t* normal_map;
	alice_texture_t* metallic_map;
	alice_texture_t* roughness_map;
	alice_texture_t* ambient_occlusion_map;
	alice_texture_t* emissive_map;
} alice_pbr_material_t;

typedef struct alice_phong_material_t {
	alice_color_t diffuse;
	alice_color_t specular;
	alice_color_t ambient;

	float shininess;
	float emissive;

	alice_texture_t* diffuse_map;
} alice_phong_material_t;

typedef enum alice_material_type_t {
	ALICE_MATERIAL_PBR,
	ALICE_MATERIAL_PHONG
} alice_material_type_t;

typedef struct alice_material_t {
	alice_shader_t* shader;

	alice_material_type_t type;

	union {
		alice_pbr_material_t pbr;
		alice_phong_material_t phong;
	} as;
} alice_material_t;

ALICE_API void alice_apply_material(alice_scene_t* scene, alice_material_t* material);

typedef struct alice_renderable_3d_t {
	alice_entity_t base;

	alice_material_t** materials;
	u32 material_count;
	u32 material_capacity;

	alice_model_t* model;

	bool cast_shadows;
} alice_renderable_3d_t;

ALICE_API void alice_apply_point_lights(alice_scene_t* scene, alice_aabb_t mesh_aabb, alice_material_t* material);

ALICE_API void alice_on_renderable_3d_create(alice_scene_t* scene, alice_entity_handle_t handle, void* ptr);
ALICE_API void alice_on_renderable_3d_destroy(alice_scene_t* scene, alice_entity_handle_t handle, void* ptr);
ALICE_API void alice_renderable_3d_add_material(alice_renderable_3d_t* renderable, const char* material_path);

typedef struct alice_shadowmap_t {
	alice_shader_t* shader;

	u32 res;

	bool in_use;

	u32 framebuffer;
	u32 output;
} alice_shadowmap_t;

ALICE_API alice_shadowmap_t* alice_new_shadowmap(u32 res, alice_shader_t* shader);
ALICE_API void alice_free_shadowmap(alice_shadowmap_t* shadowmap);
ALICE_API void alice_draw_shadowmap(alice_shadowmap_t* shadowmap, alice_scene_t* scene, alice_camera_3d_t* camera);
ALICE_API void alice_bind_shadowmap_output(alice_shadowmap_t* shadowmap, u32 unit);

typedef struct alice_point_shadowmap_t {
	alice_shader_t* shader;

	u32 res;

	bool in_use;

	u32 cubemap;
	u32 framebuffer;
} alice_point_shadowmap_t;

ALICE_API alice_point_shadowmap_t* alice_new_point_shadowmap(u32 res, alice_shader_t* shader);
ALICE_API void alice_free_point_shadowmap(alice_point_shadowmap_t* shadowmap);
ALICE_API void alice_draw_point_shadowmap(alice_point_shadowmap_t* shadowmap,
		alice_scene_t* scene);
ALICE_API void alice_bind_point_shadowmap_output(alice_point_shadowmap_t* shadowmap, u32 unit);

typedef struct alice_scene_renderer_3d_t {
	alice_render_target_t* bright_pixels;
	alice_render_target_t* bloom_ping_pong[2];
	alice_render_target_t* output;
	alice_vertex_buffer_t* quad;
	alice_shader_t* postprocess;
	alice_shader_t* extract;
	alice_shader_t* blur;

	bool debug;
	alice_debug_renderer_t* debug_renderer;

	alice_shadowmap_t* shadowmap;
	alice_point_shadowmap_t* point_shadowmap;

	bool use_bloom;
	float bloom_threshold;
	u32 bloom_blur_iterations;

	bool use_antialiasing;

	alice_color_t color_mod;
	alice_color_t ambient_color;
	float ambient_intensity;
} alice_scene_renderer_3d_t;

ALICE_API alice_scene_renderer_3d_t* alice_new_scene_renderer_3d(alice_shader_t* postprocess_shader,
	alice_shader_t* extract_shader, alice_shader_t* blur_shader, alice_shader_t* depth_shader,
	alice_shader_t* point_depth_shader, bool debug, alice_shader_t* debug_shader,
	u32 shadowmap_resolution);
ALICE_API void alice_free_scene_renderer_3d(alice_scene_renderer_3d_t* renderer);
ALICE_API void alice_render_scene_3d(alice_scene_renderer_3d_t* renderer, u32 width, u32 height,
	alice_scene_t* scene, alice_render_target_t* render_target);

ALICE_API alice_aabb_t alice_transform_aabb(alice_aabb_t aabb, alice_m4f_t m);
ALICE_API alice_aabb_t alice_compute_scene_aabb(alice_scene_t* scene);

typedef struct alice_3d_pick_context_t {
	alice_shader_t* shader;
	alice_render_target_t* target;	
} alice_3d_pick_context_t;

ALICE_API alice_3d_pick_context_t* alice_new_3d_pick_context(alice_shader_t* shader);
ALICE_API void alice_free_3d_pick_context(alice_3d_pick_context_t* context);
ALICE_API alice_entity_handle_t alice_3d_pick(alice_3d_pick_context_t* context, alice_scene_t* scene);

typedef struct alice_sprite_2d_t {
	alice_entity_t base;

	alice_texture_t* image;
	alice_v4f_t source_rect;
} alice_sprite_2d_t;

ALICE_API alice_v3f_t alice_get_sprite_2d_world_position(alice_scene_t* scene, alice_entity_t* entity);

typedef struct alice_scene_renderer_2d_t {
	alice_shader_t* sprite_shader;

	alice_vertex_buffer_t* quad;
} alice_scene_renderer_2d_t;

ALICE_API alice_scene_renderer_2d_t* alice_new_scene_renderer_2d(alice_shader_t* sprite_shader);
ALICE_API void alice_free_scene_renderer_2d(alice_scene_renderer_2d_t* renderer);
ALICE_API void alice_render_scene_2d(alice_scene_renderer_2d_t* renderer, u32 width, u32 height,
		alice_scene_t* scene, alice_render_target_t* render_target);

typedef struct alice_tilemap_t {
	alice_entity_t base;

	alice_v4f_t tiles[32];
	u32 tile_count;

	u32 tile_size;

	i32* data;
	alice_v2u_t dimentions;

	alice_texture_t* texture;
} alice_tilemap_t;

ALICE_API void alice_on_tilemap_create(alice_scene_t* scene, alice_entity_handle_t handle, void* ptr);
ALICE_API void alice_on_tilemap_destroy(alice_scene_t* scene, alice_entity_handle_t handle, void* ptr);
ALICE_API void alice_tilemap_set_tile(alice_tilemap_t* tilemap, i32 id, alice_v4f_t tile);
ALICE_API void alice_tilemap_set(alice_tilemap_t* tilemap, alice_v2u_t position, i32 tile);

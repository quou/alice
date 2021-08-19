#pragma once

#include "alice/core.h"
#include "alice/maths.h"
#include "alice/entity.h"

typedef struct alice_aabb_t {
	alice_v3f_t min;
	alice_v3f_t max;
} alice_aabb_t;

typedef struct alice_manifold_t {
	float penetration;
	alice_v3f_t normal;
} alice_manifold_t;

ALICE_API bool alice_aabb_vs_aabb(alice_aabb_t a, alice_aabb_t b, alice_manifold_t* manifold);
ALICE_API bool alice_sphere_vs_aabb(alice_aabb_t a, alice_v3f_t sphere_position, float sphere_radius);
ALICE_API bool alice_ray_vs_aabb(alice_aabb_t a, alice_v3f_t origin, alice_v3f_t direction, float* t);

typedef struct alice_box_collider_t {
	alice_v3f_t position;
	alice_v3f_t dimentions;
} alice_box_collider_t;

typedef struct alice_rigidbody_3d_t {
	alice_entity_t base;

	alice_v3f_t position;
	alice_v3f_t old_position;

	float mass;
	float inverse_mass;

	float restitution;

	float dynamic_friction;
	float static_friction;

	alice_v3f_t velocity;
	alice_v3f_t force;

	alice_v3u_t constraints;

	float gravity_scale;

	alice_box_collider_t box;
} alice_rigidbody_3d_t;

typedef struct alice_rigidbody_pair_t {
	alice_rigidbody_3d_t* a;
	alice_rigidbody_3d_t* b;
	alice_manifold_t manifold;
} alice_rigidbody_pair_t;

typedef struct alice_physics_engine_t {
	alice_rigidbody_pair_t* pairs;
	u32 pair_count;
	u32 pair_capacity;

	alice_rigidbody_pair_t** unique_pairs;
	u32 unique_pair_count;
	u32 unique_pair_capacity;

	alice_scene_t* scene;

	float gravity;

	float accumulator;
} alice_physics_engine_t;

ALICE_API alice_physics_engine_t* alice_new_physics_engine(alice_scene_t* scene);
ALICE_API void alice_free_physics_engine(alice_physics_engine_t* engine);
ALICE_API void alice_update_physics_engine(alice_physics_engine_t* engine, double timestep);

ALICE_API void alice_on_rigidbody_3d_create(alice_scene_t* scene, alice_entity_handle_t handle, void* ptr);

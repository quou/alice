#pragma once

#include "alice/core.h"
#include "alice/maths.h"
#include "alice/entity.h"

typedef struct alice_AABB {
	alice_v3f min;
	alice_v3f max;
} alice_AABB;

typedef struct alice_Manifold {
	float penetration;
	alice_v3f normal;
} alice_Manifold;

ALICE_API bool alice_aabb_vs_aabb(alice_AABB a, alice_AABB b, alice_Manifold* manifold);

typedef struct alice_BoxCollider {
	alice_v3f dimentions;
} alice_BoxCollider;

typedef struct alice_Rigidbody3D {
	alice_Entity base;

	float mass;
	float inverse_mass;

	float restitution;

	alice_v3f velocity;
	alice_v3f force;

	float gravity_scale;

	alice_BoxCollider box;
} alice_Rigidbody3D;

typedef struct alice_RigidbodyPair {
	alice_Rigidbody3D* a;
	alice_Rigidbody3D* b;
	alice_Manifold manifold;
} alice_RigidbodyPair;

typedef struct alice_PhysicsEngine {
	alice_RigidbodyPair* pairs;
	u32 pair_count;
	u32 pair_capacity;

	alice_RigidbodyPair** unique_pairs;
	u32 unique_pair_count;
	u32 unique_pair_capacity;

	alice_Scene* scene;

	float gravity;

	float accumulator;
} alice_PhysicsEngine;

ALICE_API alice_PhysicsEngine* alice_new_physics_engine(alice_Scene* scene);
ALICE_API void alice_free_physics_engine(alice_PhysicsEngine* engine);
ALICE_API void alice_update_physics_engine(alice_PhysicsEngine* engine, double timestep);

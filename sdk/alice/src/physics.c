#include <assert.h>
#include <stdlib.h>
#include <math.h>

#include "alice/physics.h"

static void alice_calculate_body_aabb(alice_PhysicsEngine* engine, alice_Rigidbody3D* body, alice_AABB* aabb) {
	assert(engine);
	assert(body);
	assert(aabb);

	alice_v3f position = alice_get_entity_world_position(engine->scene, (alice_Entity*)body);

	aabb->min = (alice_v3f) {
		.x = position.x,
		.y = position.y,
		.z = position.z,
	};
	aabb->max = (alice_v3f) {
		.x = position.x + body->box.dimentions.x,
		.y = position.y + body->box.dimentions.y,
		.z = position.z + body->box.dimentions.z,
	};
}

static bool alice_compare_rigidbody_pair(const void* a, const void* b) {
	const alice_RigidbodyPair* lhs = a;
	const alice_RigidbodyPair* rhs = b;

	if (lhs->a < rhs->a) {
		return true;
	}

	if (lhs->a == rhs->a) {
		return lhs->b < rhs->b;
	}

	return false;
}

static void alice_correct_rigidbody_positions(alice_RigidbodyPair* pair) {
	assert(pair);

	alice_Rigidbody3D* a = pair->a;
	alice_Rigidbody3D* b = pair->b;

	const float percentage = 0.2f;
	const float slop = 0.01f;

	const float correction = alice_max(pair->manifold.penetration - slop, 0.0f) /
		(a->inverse_mass + b->inverse_mass) * percentage;
	const alice_v3f correction_vector = (alice_v3f) {
		.x = correction * pair->manifold.normal.x,
		.y = correction * pair->manifold.normal.y,
		.z = correction * pair->manifold.normal.z
	};

	a->base.position = (alice_v3f) {
		.x = a->base.position.x - a->inverse_mass * correction_vector.x,
		.y = a->base.position.y - a->inverse_mass * correction_vector.y,
		.z = a->base.position.z - a->inverse_mass * correction_vector.z,
	};

	b->base.position = (alice_v3f) {
		.x = b->base.position.x + b->inverse_mass * correction_vector.x,
		.y = b->base.position.y + b->inverse_mass * correction_vector.y,
		.z = b->base.position.z + b->inverse_mass * correction_vector.z,
	};
}

bool alice_aabb_vs_aabb(alice_AABB a, alice_AABB b, alice_Manifold* manifold) {
	assert(manifold);

	const alice_v3f n = (alice_v3f) {
		.x = b.min.x - a.min.x,
		.y = b.min.y - a.min.y,
		.z = b.min.z - a.min.z,
	};

	float a_extent = (a.max.x - a.min.x) / 2.0f;
	float b_extent = (b.max.x - b.min.x) / 2.0f;

	const float x_overlap = (a_extent + b_extent) - fabs(n.x);

	if (x_overlap > 0.0f) {
		a_extent = (a.max.y - a.min.y) / 2.0f;
		b_extent = (b.max.y - b.min.y) / 2.0f;

		const float y_overlap = (a_extent + b_extent) - fabs(n.y);

		if (y_overlap > 0.0f) {
			a_extent = (a.max.z - a.min.z) / 2.0f;
			b_extent = (b.max.z - b.min.z) / 2.0f;

			const float z_overlap = (a_extent + b_extent) - fabs(n.z);

			if (z_overlap > 0.0f) {
				float smallest_overlap = x_overlap;
				if (y_overlap < smallest_overlap) {
					smallest_overlap = y_overlap;
				} else if (z_overlap < smallest_overlap) {
					smallest_overlap = z_overlap;
				}

				if (smallest_overlap == x_overlap) {
					if (n.x < 0) {
						manifold->normal = (alice_v3f){-1.0f, 0.0f, 0.0f};
					} else {
						manifold->normal = (alice_v3f){1.0f, 0.0f, 0.0f};
					}

					manifold->penetration = x_overlap;

					return true;
				}

				if (smallest_overlap == y_overlap) {
					if (n.y < 0) {
						manifold->normal = (alice_v3f){0.0f, -1.0f, 0.0f};
					} else {
						manifold->normal = (alice_v3f){0.0f, 1.0f, 0.0f};
					}

					manifold->penetration = y_overlap;

					return true;
				}

				if (smallest_overlap == z_overlap) {
					if (n.z < 0) {
						manifold->normal = (alice_v3f){0.0f, 0.0f, -1.0f};
					} else {
						manifold->normal = (alice_v3f){0.0f, 0.0f, 1.0f};
					}

					manifold->penetration = z_overlap;

					return true;
				}

				return true;
			}
		}
	}

	return false;
}

alice_PhysicsEngine* alice_new_physics_engine(alice_Scene* scene) {
	alice_PhysicsEngine* new = malloc(sizeof(alice_PhysicsEngine));

	new->pairs = alice_null;
	new->pair_count = 0;
	new->pair_capacity = 0;

	new->unique_pairs = alice_null;
	new->unique_pair_count = 0;
	new->unique_pair_capacity = 0;

	new->accumulator = 0.0f;

	new->scene = scene;

	new->gravity = -10.0f;

	return new;
}

void alice_free_physics_engine(alice_PhysicsEngine* engine) {
	assert(engine);

	if (engine->pair_capacity > 0) {
		free(engine->pairs);
	}

	if (engine->unique_pair_capacity > 0) {
		free(engine->unique_pairs);
	}

	free(engine);
}

static void alice_tick_physics_engine(alice_PhysicsEngine* engine, double timestep) {
	assert(engine);

	engine->pair_count = 0;
	engine->unique_pair_count = 0;

	alice_AABB a_box;
	alice_AABB b_box;

	/* Integration */
	for (alice_entity_iter(engine->scene, iter, alice_Rigidbody3D)) {
		alice_Rigidbody3D* body = iter.current_ptr;

		body->inverse_mass = body->mass == 0.0f ? 0.0f : 1.0f / body->mass;

		body->velocity = (alice_v3f) {
			.x = body->velocity.x + ((body->inverse_mass * body->force.x) * timestep),
			.y = body->velocity.y + ((body->inverse_mass * body->force.y
						+ (engine->gravity * body->gravity_scale)) * timestep),
			.z = body->velocity.z + ((body->inverse_mass * body->force.z) * timestep),
		};

		body->base.position = (alice_v3f){
			.x = body->base.position.x + body->velocity.x * timestep,
			.y = body->base.position.y + body->velocity.y * timestep,
			.z = body->base.position.z + body->velocity.z * timestep,
		};
	}

	/* Check collisions */
	for (alice_entity_iter(engine->scene, i, alice_Rigidbody3D)) {
		for (alice_entity_iter(engine->scene, j, alice_Rigidbody3D)) {
			alice_Rigidbody3D* a = i.current_ptr;
			alice_Rigidbody3D* b = j.current_ptr;

			if (a == b) {
				continue;
			}

			alice_calculate_body_aabb(engine, a, &a_box);
			alice_calculate_body_aabb(engine, b, &b_box);

			alice_RigidbodyPair pair;
			pair.a = a;
			pair.b = b;

			if (alice_aabb_vs_aabb(a_box, b_box, &pair.manifold)) {
				if (engine->pair_count >= engine->pair_capacity) {
					engine->pair_capacity = alice_grow_capacity(engine->pair_capacity);
					engine->pairs = realloc(engine->pairs,
						engine->pair_capacity * sizeof(alice_RigidbodyPair));
				}

				engine->pairs[engine->pair_count++] = pair;
			}
		}
	}

	/* Cull duplicate pairs */
	qsort(engine->pairs, engine->pair_count, sizeof(alice_RigidbodyPair), alice_compare_rigidbody_pair);
	{
		u32 i = 0;
		while (i < engine->pair_count) {
			if (engine->unique_pair_count >= engine->unique_pair_capacity) {
				engine->unique_pair_capacity = alice_grow_capacity(engine->unique_pair_capacity);
				engine->unique_pairs = realloc(engine->unique_pairs,
						engine->unique_pair_capacity * sizeof(alice_RigidbodyPair*));
			}

			alice_RigidbodyPair* pair = &engine->pairs[i];

			engine->unique_pairs[engine->unique_pair_count++] = pair;

			i++;

			while (i < engine->pair_count) {
				alice_RigidbodyPair* potential_dup = &engine->pairs[i];
				if (pair->a != potential_dup->b || pair->b != potential_dup->a) {
					break;
				}

				i++;
			}
		}
	}

	/* Resolve collisions */
	for (u32 i = 0; i < engine->unique_pair_count; i++) {
		alice_RigidbodyPair* pair = engine->unique_pairs[i];

		alice_Rigidbody3D* a = pair->a;
		alice_Rigidbody3D* b = pair->b;

		alice_v3f relative_velocity = (alice_v3f) {
			.x = b->velocity.x - a->velocity.x,
			.y = b->velocity.y - a->velocity.y,
			.z = b->velocity.z - a->velocity.z
		};

		const float vel_along_normal = alice_v3f_dot(relative_velocity, pair->manifold.normal);

		if (vel_along_normal > 0.0f) {
			continue;
		}

		const float e = alice_min(a->restitution, b->restitution);

		float j = -(1.0f + e) * vel_along_normal;
		j /= a->inverse_mass + b->inverse_mass;

		const alice_v3f impulse = (alice_v3f) {
			.x = j * pair->manifold.normal.x,
			.y = j * pair->manifold.normal.y,
			.z = j * pair->manifold.normal.z
		};

		a->velocity = (alice_v3f){
			.x = a->velocity.x - (a->inverse_mass * impulse.x),
			.y = a->velocity.y - (a->inverse_mass * impulse.y),
			.z = a->velocity.z - (a->inverse_mass * impulse.z),
		};

		b->velocity = (alice_v3f){
			.x = b->velocity.x + (b->inverse_mass * impulse.x),
			.y = b->velocity.y + (b->inverse_mass * impulse.y),
			.z = b->velocity.z + (b->inverse_mass * impulse.z),
		};

		/* Friction */
		relative_velocity = (alice_v3f) {
			.x = b->velocity.x - a->velocity.x,
			.y = b->velocity.y - a->velocity.y,
			.z = b->velocity.z - a->velocity.z
		};

		const float rv_dot_normal = alice_v3f_dot(relative_velocity, pair->manifold.normal);
		const alice_v3f tangent_vector = alice_v3f_normalise((alice_v3f) {
			.x = relative_velocity.x - rv_dot_normal * pair->manifold.normal.x,
			.y = relative_velocity.y - rv_dot_normal * pair->manifold.normal.y,
			.z = relative_velocity.z - rv_dot_normal * pair->manifold.normal.z,
		});

		float jt = -alice_v3f_dot(relative_velocity, tangent_vector);
		jt /= a->inverse_mass + b->inverse_mass;

		float mu = sqrt((a->static_friction * a->static_friction) + (b->static_friction * b->static_friction));

		alice_v3f friction_impulse;
		if (fabs(jt) < j * mu) {
			friction_impulse = (alice_v3f){
				.x = jt * tangent_vector.x,
				.y = jt * tangent_vector.y,
				.z = jt * tangent_vector.z,
			};
		} else {
			float dynamic_friction = sqrt((a->dynamic_friction * a->dynamic_friction) +
					(b->dynamic_friction * b->dynamic_friction));
			friction_impulse = (alice_v3f){
				.x = -j * dynamic_friction * tangent_vector.x,
				.y = -j * dynamic_friction * tangent_vector.y,
				.z = -j * dynamic_friction * tangent_vector.z,
			};
		}

		a->velocity = (alice_v3f){
			.x = a->velocity.x - a->inverse_mass * friction_impulse.x,
			.y = a->velocity.y - a->inverse_mass * friction_impulse.y,
			.z = a->velocity.z - a->inverse_mass * friction_impulse.z,
		};

		b->velocity = (alice_v3f){
			.x = b->velocity.x + b->inverse_mass * friction_impulse.x,
			.y = b->velocity.y + b->inverse_mass * friction_impulse.y,
			.z = b->velocity.z + b->inverse_mass * friction_impulse.z,
		};

		alice_correct_rigidbody_positions(pair);
	}
}

void alice_update_physics_engine(alice_PhysicsEngine* engine, double timestep) {
	assert(engine);

	const float fps = 60.0f;
	const float dt = 1.0f / fps;

	engine->accumulator += timestep;

	if (engine->accumulator > 0.2f) {
		engine->accumulator = 0.2f;
	}

	while (engine->accumulator > dt) {
		alice_tick_physics_engine(engine, dt);
		engine->accumulator -= dt;
	}

	const float alpha = engine->accumulator / dt;

	for (alice_entity_iter(engine->scene, iter, alice_Rigidbody3D)) {
		alice_Rigidbody3D* rigidbody = iter.current_ptr;

		rigidbody->base.position = (alice_v3f) {
			.x = rigidbody->old_position.x * alpha + rigidbody->base.position.x * (1.0f - alpha),
			.y = rigidbody->old_position.y * alpha + rigidbody->base.position.y * (1.0f - alpha),
			.z = rigidbody->old_position.z * alpha + rigidbody->base.position.z * (1.0f - alpha),
		};

		rigidbody->old_position = rigidbody->base.position;
	}
}

void alice_on_rigidbody_3d_create(alice_Scene* scene, alice_EntityHandle handle, void* ptr) {
	alice_Rigidbody3D* rigidbody = ptr;

	rigidbody->old_position = rigidbody->base.position;

	rigidbody->mass = 1.0f;
	rigidbody->restitution = 0.3f;

	rigidbody->velocity = (alice_v3f){0.0f, 0.0f, 0.0f};
	rigidbody->force = (alice_v3f){0.0f, 0.0f, 0.0f};

	rigidbody->gravity_scale = 1.0f;

	rigidbody->dynamic_friction = 0.3f;
	rigidbody->static_friction = 0.3f;

	rigidbody->box = (alice_BoxCollider) {
		.dimentions = (alice_v3f) {
			.x = 2.0f,
			.y = 2.0f,
			.z = 2.0f
		}
	};
}

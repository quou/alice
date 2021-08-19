#include <assert.h>
#include <stdlib.h>
#include <math.h>

#include "alice/physics.h"
#include "alice/scripting.h"

static void alice_calculate_body_aabb(alice_physics_engine_t* engine, alice_rigidbody_3d_t* body, alice_aabb_t* aabb) {
	assert(engine);
	assert(body);
	assert(aabb);

	alice_v3f_t position = alice_get_entity_world_position(engine->scene, (alice_entity_t*)body);

	aabb->min = (alice_v3f_t) {
		.x = body->box.position.x + position.x,
		.y = body->box.position.y + position.y,
		.z = body->box.position.z + position.z,
	};
	aabb->max = (alice_v3f_t) {
		.x = body->box.position.x + position.x + body->box.dimentions.x,
		.y = body->box.position.y + position.y + body->box.dimentions.y,
		.z = body->box.position.z + position.z + body->box.dimentions.z,
	};
}

static i32 alice_compare_rigidbody_pair(const void* a, const void* b) {
	const alice_rigidbody_pair_t* lhs = a;
	const alice_rigidbody_pair_t* rhs = b;

	if (lhs->a < rhs->a) {
		return true;
	}

	if (lhs->a == rhs->a) {
		return lhs->b < rhs->b;
	}

	return false;
}

static void alice_correct_rigidbody_positions(alice_rigidbody_pair_t* pair) {
	assert(pair);

	alice_rigidbody_3d_t* a = pair->a;
	alice_rigidbody_3d_t* b = pair->b;

	const float percentage = 0.2f;
	const float slop = 0.01f;

	const float correction = alice_max(pair->manifold.penetration - slop, 0.0f) /
		(a->inverse_mass + b->inverse_mass) * percentage;
	const alice_v3f_t correction_vector = (alice_v3f_t) {
		.x = correction * pair->manifold.normal.x,
		.y = correction * pair->manifold.normal.y,
		.z = correction * pair->manifold.normal.z
	};

	a->position = (alice_v3f_t) {
		.x = a->position.x - a->inverse_mass * correction_vector.x,
		.y = a->position.y - a->inverse_mass * correction_vector.y,
		.z = a->position.z - a->inverse_mass * correction_vector.z,
	};

	b->position = (alice_v3f_t) {
		.x = b->position.x + b->inverse_mass * correction_vector.x,
		.y = b->position.y + b->inverse_mass * correction_vector.y,
		.z = b->position.z + b->inverse_mass * correction_vector.z,
	};
}

bool alice_aabb_vs_aabb(alice_aabb_t a, alice_aabb_t b, alice_manifold_t* manifold) {
	assert(manifold);

	const alice_v3f_t n = (alice_v3f_t) {
		.x = b.min.x - a.min.x,
		.y = b.min.y - a.min.y,
		.z = b.min.z - a.min.z,
	};

	float a_extent = (a.max.x - a.min.x) / 2.0f;
	float b_extent = (b.max.x - b.min.x) / 2.0f;

	const float x_overlap = (a_extent + b_extent) - (float)fabs(n.x);

	if (x_overlap > 0.0f) {
		a_extent = (a.max.y - a.min.y) / 2.0f;
		b_extent = (b.max.y - b.min.y) / 2.0f;

		const float y_overlap = (a_extent + b_extent) - (float)fabs(n.y);

		if (y_overlap > 0.0f) {
			a_extent = (a.max.z - a.min.z) / 2.0f;
			b_extent = (b.max.z - b.min.z) / 2.0f;

			const float z_overlap = (a_extent + b_extent) - (float)fabs(n.z);

			if (z_overlap > 0.0f) {
				float smallest_overlap = x_overlap;
				if (y_overlap < smallest_overlap) {
					smallest_overlap = y_overlap;
				}
				if (z_overlap < smallest_overlap) {
					smallest_overlap = z_overlap;
				}

				if (smallest_overlap == x_overlap) {
					if (n.x < 0.0f) {
						manifold->normal = (alice_v3f_t){-1.0f, 0.0f, 0.0f};
					} else {
						manifold->normal = (alice_v3f_t){1.0f, 0.0f, 0.0f};
					}

					manifold->penetration = x_overlap;

					return true;
				} else if (smallest_overlap == y_overlap) {
					if (n.y < 0.0f) {
						manifold->normal = (alice_v3f_t){0.0f, -1.0f, 0.0f};
					} else {
						manifold->normal = (alice_v3f_t){0.0f, 1.0f, 0.0f};
					}

					manifold->penetration = y_overlap;

					return true;
				} else {
					if (n.z < 0.0f) {
						manifold->normal = (alice_v3f_t){0.0f, 0.0f, -1.0f};
					} else {
						manifold->normal = (alice_v3f_t){0.0f, 0.0f, 1.0f};
					}

					manifold->penetration = z_overlap;

					return true;
				}
			}
		}
	}

	return false;
}

bool alice_sphere_vs_aabb(alice_aabb_t aabb, alice_v3f_t sphere_position, float sphere_radius) {
	float dist_squared = sphere_radius * sphere_radius;

	if (sphere_position.x < aabb.min.x) {
		dist_squared -= alice_squared(sphere_position.x - aabb.min.x);
	} else if (sphere_position.x > aabb.max.x) {
		dist_squared -= alice_squared(sphere_position.x - aabb.max.x);
	}

	if (sphere_position.y < aabb.min.y) {
		dist_squared -= alice_squared(sphere_position.y - aabb.min.y);
	} else if (sphere_position.y > aabb.max.y) {
		dist_squared -= alice_squared(sphere_position.y - aabb.max.y);
	}

	if (sphere_position.z < aabb.min.z) {
		dist_squared -= alice_squared(sphere_position.z - aabb.min.z);
	} else if (sphere_position.z > aabb.max.z) {
		dist_squared -= alice_squared(sphere_position.z - aabb.max.z);
	}

	return dist_squared > 0.0f;
}

bool alice_ray_vs_aabb(alice_aabb_t a, alice_v3f_t origin, alice_v3f_t direction, float* t) {
	alice_v3f_t inv_dir = (alice_v3f_t) {
		.x = 1.0f / direction.x,
		.y = 1.0f / direction.y,
		.z = 1.0f / direction.z
	};

	float t1 = (a.min.x - origin.x) * inv_dir.x;
	float t2 = (a.max.x - origin.x) * inv_dir.x;
	float t3 = (a.min.y - origin.y) * inv_dir.y;
	float t4 = (a.max.y - origin.y) * inv_dir.y;
	float t5 = (a.min.z - origin.z) * inv_dir.z;
	float t6 = (a.max.z - origin.z) * inv_dir.z;

	float tmin = alice_max(alice_max(alice_min(t1, t2), alice_min(t3, t4)), alice_min(t5, t6));
	float tmax = alice_min(alice_min(alice_max(t1, t2), alice_max(t3, t4)), alice_max(t5, t6));

	if (tmax < 0) {
		*t = tmax;
		return false;
	}

	if (tmin > tmax) {
		*t = tmax;
		return false;
	}

	*t = tmin;
	return true;
}

alice_physics_engine_t* alice_new_physics_engine(alice_scene_t* scene) {
	alice_physics_engine_t* new = malloc(sizeof(alice_physics_engine_t));

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

void alice_free_physics_engine(alice_physics_engine_t* engine) {
	assert(engine);

	if (engine->pair_capacity > 0) {
		free(engine->pairs);
	}

	if (engine->unique_pair_capacity > 0) {
		free(engine->unique_pairs);
	}

	free(engine);
}

static void alice_tick_physics_engine(alice_physics_engine_t* engine, double timestep) {
	assert(engine);

	engine->pair_count = 0;
	engine->unique_pair_count = 0;

	alice_aabb_t a_box;
	alice_aabb_t b_box;

	/* Integration */
	for (alice_entity_iter(engine->scene, iter, alice_rigidbody_3d_t)) {
		alice_rigidbody_3d_t* body = iter.current_ptr;

		body->inverse_mass = body->mass == 0.0f ? 0.0f : 1.0f / body->mass;

		body->velocity = (alice_v3f_t) {
			.x = body->velocity.x + ((body->inverse_mass * body->force.x) * (float)timestep),
			.y = body->velocity.y + ((body->inverse_mass * body->force.y
						+ (engine->gravity * body->gravity_scale)) * (float)timestep),
			.z = body->velocity.z + ((body->inverse_mass * body->force.z) * (float)timestep),
		};

		body->velocity.x = body->constraints.x ? 0.0f : body->velocity.x;
		body->velocity.y = body->constraints.y ? 0.0f : body->velocity.y;
		body->velocity.z = body->constraints.z ? 0.0f : body->velocity.z;

		body->position.x += body->velocity.x * (float)timestep;
		body->position.y += body->velocity.y * (float)timestep;
		body->position.z += body->velocity.z * (float)timestep;
	}

	/* Check collisions */
	for (alice_entity_iter(engine->scene, i, alice_rigidbody_3d_t)) {
		for (alice_entity_iter(engine->scene, j, alice_rigidbody_3d_t)) {
			alice_rigidbody_3d_t* a = i.current_ptr;
			alice_rigidbody_3d_t* b = j.current_ptr;

			if (a == b || a->mass == 0.0f && b->mass == 0.0f) {
				continue;
			}

			alice_calculate_body_aabb(engine, a, &a_box);
			alice_calculate_body_aabb(engine, b, &b_box);

			alice_rigidbody_pair_t pair;
			pair.a = a;
			pair.b = b;

			if (alice_aabb_vs_aabb(a_box, b_box, &pair.manifold)) {
				if (engine->pair_count >= engine->pair_capacity) {
					engine->pair_capacity = alice_grow_capacity(engine->pair_capacity);
					engine->pairs = realloc(engine->pairs,
						engine->pair_capacity * sizeof(alice_rigidbody_pair_t));
				}

				engine->pairs[engine->pair_count++] = pair;
			}
		}
	}

	/* Cull duplicate pairs */
	qsort(engine->pairs, engine->pair_count, sizeof(alice_rigidbody_pair_t), alice_compare_rigidbody_pair);
	{
		u32 i = 0;
		while (i < engine->pair_count) {
			if (engine->unique_pair_count >= engine->unique_pair_capacity) {
				engine->unique_pair_capacity = alice_grow_capacity(engine->unique_pair_capacity);
				engine->unique_pairs = realloc(engine->unique_pairs,
						engine->unique_pair_capacity * sizeof(alice_rigidbody_pair_t*));
			}

			alice_rigidbody_pair_t* pair = &engine->pairs[i];

			engine->unique_pairs[engine->unique_pair_count++] = pair;

			i++;

			while (i < engine->pair_count) {
				alice_rigidbody_pair_t* potential_dup = &engine->pairs[i];
				if (pair->a != potential_dup->b || pair->b != potential_dup->a) {
					break;
				}

				i++;
			}
		}
	}

	/* Resolve collisions */
	for (u32 i = 0; i < engine->unique_pair_count; i++) {
		alice_rigidbody_pair_t* pair = engine->unique_pairs[i];

		alice_rigidbody_3d_t* a = pair->a;
		alice_rigidbody_3d_t* b = pair->b;

		alice_v3f_t relative_velocity = (alice_v3f_t) {
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

		const alice_v3f_t impulse = (alice_v3f_t) {
			.x = j * pair->manifold.normal.x,
			.y = j * pair->manifold.normal.y,
			.z = j * pair->manifold.normal.z
		};

		a->velocity = (alice_v3f_t){
			.x = a->velocity.x - (a->inverse_mass * impulse.x),
			.y = a->velocity.y - (a->inverse_mass * impulse.y),
			.z = a->velocity.z - (a->inverse_mass * impulse.z),
		};

		b->velocity = (alice_v3f_t){
			.x = b->velocity.x + (b->inverse_mass * impulse.x),
			.y = b->velocity.y + (b->inverse_mass * impulse.y),
			.z = b->velocity.z + (b->inverse_mass * impulse.z),
		};

		/* Friction */
		relative_velocity = (alice_v3f_t) {
			.x = b->velocity.x - a->velocity.x,
			.y = b->velocity.y - a->velocity.y,
			.z = b->velocity.z - a->velocity.z
		};

		const float rv_dot_normal = alice_v3f_dot(relative_velocity, pair->manifold.normal);
		const alice_v3f_t tangent_vector = alice_v3f_normalise((alice_v3f_t) {
			.x = relative_velocity.x - rv_dot_normal * pair->manifold.normal.x,
			.y = relative_velocity.y - rv_dot_normal * pair->manifold.normal.y,
			.z = relative_velocity.z - rv_dot_normal * pair->manifold.normal.z,
		});

		float jt = -alice_v3f_dot(relative_velocity, tangent_vector);
		jt /= a->inverse_mass + b->inverse_mass;

		const float mu = sqrtf((a->static_friction * a->static_friction)
				+ (b->static_friction * b->static_friction));

		alice_v3f_t friction_impulse;
		if (fabs(jt) < j * mu) {
			friction_impulse = (alice_v3f_t){
				.x = jt * tangent_vector.x,
				.y = jt * tangent_vector.y,
				.z = jt * tangent_vector.z,
			};
		} else {
			const float dynamic_friction = sqrtf((a->dynamic_friction * a->dynamic_friction) +
					(b->dynamic_friction * b->dynamic_friction));
			friction_impulse = (alice_v3f_t){
				.x = -j * dynamic_friction * tangent_vector.x,
				.y = -j * dynamic_friction * tangent_vector.y,
				.z = -j * dynamic_friction * tangent_vector.z,
			};
		}

		a->velocity = (alice_v3f_t){
			.x = a->velocity.x - a->inverse_mass * friction_impulse.x,
			.y = a->velocity.y - a->inverse_mass * friction_impulse.y,
			.z = a->velocity.z - a->inverse_mass * friction_impulse.z,
		};

		b->velocity = (alice_v3f_t){
			.x = b->velocity.x + b->inverse_mass * friction_impulse.x,
			.y = b->velocity.y + b->inverse_mass * friction_impulse.y,
			.z = b->velocity.z + b->inverse_mass * friction_impulse.z,
		};

		alice_correct_rigidbody_positions(pair);
	}
}

void alice_update_physics_engine(alice_physics_engine_t* engine, double timestep) {
	assert(engine);

	const double fps = 60.0f;
	const double dt = 1.0f / fps;

	engine->accumulator += (float)timestep;

	if (engine->accumulator > 0.2f) {
		engine->accumulator = 0.2f;
	}

	while (engine->accumulator > dt) {
		if (engine->scene->script_context) {
			alice_physics_update_scripts(engine->scene->script_context, dt);
		}

		alice_tick_physics_engine(engine, dt);
		engine->accumulator -= (float)dt;
	}

	const float alpha = engine->accumulator / dt;

	for (alice_entity_iter(engine->scene, iter, alice_rigidbody_3d_t)) {
		alice_rigidbody_3d_t* body = iter.current_ptr;

		body->base.position = (alice_v3f_t) {
			.x = body->old_position.x * alpha + body->position.x * (1.0f - alpha),
			.y = body->old_position.y * alpha + body->position.y * (1.0f - alpha),
			.z = body->old_position.z * alpha + body->position.z * (1.0f - alpha)
		};

		body->old_position = body->position;
	}
}

void alice_on_rigidbody_3d_create(alice_scene_t* scene, alice_entity_handle_t handle, void* ptr) {
	alice_rigidbody_3d_t* rigidbody = ptr;

	rigidbody->position = (alice_v3f_t) { 0.0f, 0.0f, 0.0f };
	rigidbody->old_position = rigidbody->position;

	rigidbody->mass = 1.0f;
	rigidbody->restitution = 0.3f;

	rigidbody->velocity = (alice_v3f_t){0.0f, 0.0f, 0.0f};
	rigidbody->force = (alice_v3f_t){0.0f, 0.0f, 0.0f};

	rigidbody->gravity_scale = 1.0f;

	rigidbody->dynamic_friction = 0.3f;
	rigidbody->static_friction = 0.3f;

	rigidbody->box = (alice_box_collider_t) {
		.position = (alice_v3f_t) {
			.x = 0.0f,
			.y = 0.0f,
			.z = 0.0f
		},
		.dimentions = (alice_v3f_t) {
			.x = 2.0f,
			.y = 2.0f,
			.z = 2.0f
		}
	};

	rigidbody->constraints = (alice_v3u_t) { false, false, false };
}

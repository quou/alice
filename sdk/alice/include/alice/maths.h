#pragma once

#include "alice/core.h"

#define alice_pi 3.14159265358f

#define alice_squared(a_) ((a_) * (a_))

typedef struct alice_v2f {
	float x, y;
} alice_v2f;

typedef struct alice_v2i {
	i32 x, y;
} alice_v2i;

typedef struct alice_v2u {
	u32 x, y;
} alice_v2u;

ALICE_API float alice_v2f_mag(alice_v2f v);
ALICE_API alice_v2f alice_v2f_normalise(alice_v2f v);
ALICE_API float alice_v2f_dist(alice_v2f a, alice_v2f b);
ALICE_API float alice_v2f_dot(alice_v2f a, alice_v2f b);

ALICE_API i32 alice_v2i_mag(alice_v2i v);
ALICE_API alice_v2i alice_v2i_normalise(alice_v2i v);
ALICE_API i32 alice_v2i_dist(alice_v2i a, alice_v2i b);
ALICE_API i32 alice_v2i_dot(alice_v2i a, alice_v2i b);

ALICE_API u32 alice_v2u_mag(alice_v2u v);
ALICE_API alice_v2u alice_v2u_normalise(alice_v2u v);
ALICE_API u32 alice_v2u_dist(alice_v2u a, alice_v2u b);
ALICE_API u32 alice_v2u_dot(alice_v2u a, alice_v2u b);

typedef struct alice_v3f {
	float x, y, z;
} alice_v3f;

typedef struct alice_v3i {
	i32 x, y, z;
} alice_v3i;

typedef struct alice_v3u {
	u32 x, y, z;
} alice_v3u;

ALICE_API float alice_v3f_mag(alice_v3f v);
ALICE_API alice_v3f alice_v3f_normalise(alice_v3f v);
ALICE_API float alice_v3f_dist(alice_v3f a, alice_v3f b);
ALICE_API float alice_v3f_dot(alice_v3f a, alice_v3f b);
ALICE_API alice_v3f alice_v3f_cross(alice_v3f a, alice_v3f b);

ALICE_API i32 alice_v3i_mag(alice_v3i v);
ALICE_API alice_v3i alice_v3i_normalise(alice_v3i v);
ALICE_API i32 alice_v3i_dist(alice_v3i a, alice_v3i b);
ALICE_API i32 alice_v3i_dot(alice_v3i a, alice_v3i b);

ALICE_API u32 alice_v3u_mag(alice_v3u v);
ALICE_API alice_v3u alice_v3u_normalise(alice_v3u v);
ALICE_API u32 alice_v3u_dist(alice_v3u a, alice_v3u b);
ALICE_API u32 alice_v3u_dot(alice_v3u a, alice_v3u b);

typedef struct alice_v4f {
	float x, y, z, w;
} alice_v4f;

typedef struct alice_v4i {
	i32 x, y, z, w;
} alice_v4i;

typedef struct alice_v4u {
	u32 x, y, z, w;
} alice_v4u;

ALICE_API float alice_v4f_mag(alice_v4f v);
ALICE_API alice_v4f alice_v4f_normalise(alice_v4f v);
ALICE_API float alice_v4f_dist(alice_v4f a, alice_v4f b);
ALICE_API float alice_v4f_dot(alice_v4f a, alice_v4f b);

ALICE_API i32 alice_v4i_mag(alice_v4i v);
ALICE_API alice_v4i alice_v4i_normalise(alice_v4i v);
ALICE_API i32 alice_v4i_dist(alice_v4i a, alice_v4i b);
ALICE_API i32 alice_v4i_dot(alice_v4i a, alice_v4i b);

ALICE_API u32 alice_v4u_mag(alice_v4u v);
ALICE_API alice_v4u alice_v4u_normalise(alice_v4u v);
ALICE_API u32 alice_v4u_dist(alice_v4u a, alice_v4u b);
ALICE_API u32 alice_v4u_dot(alice_v4u a, alice_v4u b);

/* To-degrees */
ALICE_API float alice_todeg(float rad);
ALICE_API alice_v2f alice_todeg_v2f(alice_v2f rad);
ALICE_API alice_v3f alice_todeg_v3f(alice_v3f rad);
ALICE_API alice_v4f alice_todeg_v4f(alice_v4f rad);

/* To-radians */
ALICE_API float alice_torad(float deg);
ALICE_API alice_v2f alice_torad_v2f(alice_v2f deg);
ALICE_API alice_v3f alice_torad_v3f(alice_v3f deg);
ALICE_API alice_v4f alice_torad_v4f(alice_v4f deg);

/* 4x4 float matrix */
typedef struct alice_m4f {
	float elements[4][4];
} alice_m4f;

ALICE_API alice_m4f alice_new_mf4(float diagonal);
ALICE_API alice_m4f alice_m4f_identity();

ALICE_API alice_m4f alice_m4f_multiply(alice_m4f a, alice_m4f b);

ALICE_API alice_m4f alice_m4f_translate(alice_m4f m, alice_v3f v);
ALICE_API alice_m4f alice_m4f_rotate(alice_m4f m, float a, alice_v3f v);
ALICE_API alice_m4f alice_m4f_scale(alice_m4f m, alice_v3f v);

ALICE_API alice_m4f alice_m4f_ortho(float left, float right,
	float bottom, float top, float near, float far);

ALICE_API alice_m4f alice_m4f_persp(float fov, float aspect, float near, float far);

ALICE_API alice_m4f alice_m4f_lookat(alice_v3f camera, alice_v3f object, alice_v3f up);

ALICE_API void alice_m4f_decompose(alice_m4f matrix, alice_v3f* translation, alice_v3f* rotation, alice_v3f* scale);

ALICE_API alice_v3f alice_v3f_transform(alice_v3f v, alice_m4f m);
ALICE_API alice_v4f alice_v4f_transform(alice_v4f v, alice_m4f m);

/* TODO: signed and unsigned integer matrices */

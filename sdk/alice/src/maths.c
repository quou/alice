#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "alice/maths.h"

/* ==== 2-D VECTOR ==== */

float alice_v2f_mag(alice_v2f v) {
	return sqrtf(v.x * v.x + v.y * v.y);
}

alice_v2f alice_v2f_normalise(alice_v2f v) {
	const float l = alice_v2f_mag(v);

	if (l == 0.0) {
		return (alice_v2f) {0.0, 0.0};
	}

	return (alice_v2f){v.x / l, v.y / l};
}

float alice_v2f_dist(alice_v2f a, alice_v2f b) {
	const float xd = a.x - b.x;
	const float yd = a.y - b.y;
	return sqrtf(xd * xd + yd * yd);
}

float alice_v2f_dot(alice_v2f a, alice_v2f b) {
	return a.x * b.x + a.y * b.y;
}

i32 alice_v2i_mag(alice_v2i v) {
	return sqrt(v.x * v.x + v.y * v.y);
}

alice_v2i alice_v2i_normalise(alice_v2i v) {
	i32 l = alice_v2i_mag(v);

	if (l == 0) {
		return (alice_v2i) {0, 0};
	}

	return (alice_v2i){v.x / l, v.y / l};
}

i32 alice_v2i_dist(alice_v2i a, alice_v2i b) {
	const i32 xd = a.x - b.x;
	const i32 yd = a.y - b.y;
	return sqrt(xd * xd + yd * yd);
}

i32 alice_v2i_dot(alice_v2i a, alice_v2i b) {
	return a.x * b.x + a.y * b.y;
}

u32 alice_v2u_mag(alice_v2u v) {
	return sqrt(v.x * v.x + v.y * v.y);
}

alice_v2u alice_v2u_normalise(alice_v2u v) {
	u32 l = alice_v2u_mag(v);

	if (l == 0) {
		return (alice_v2u) {0, 0};
	}

	return (alice_v2u){v.x / l, v.y / l};
}

u32 alice_v2u_dist(alice_v2u a, alice_v2u b) {
	const u32 xd = a.x - b.x;
	const u32 yd = a.y - b.y;
	return sqrt(xd * xd + yd * yd);
}

u32 alice_v2u_dot(alice_v2u a, alice_v2u b) {
	return a.x * b.x + a.y * b.y;
}

/* ==== 3-D VECTOR ==== */

float alice_v3f_mag(alice_v3f v) {
	return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

alice_v3f alice_v3f_normalise(alice_v3f v) {
	const float l = alice_v3f_mag(v);

	if (l == 0.0) {
		return (alice_v3f) {0.0, 0.0, 0.0};
	}

	return (alice_v3f){v.x / l, v.y / l, v.z / l};
}

float alice_v3f_dist(alice_v3f a, alice_v3f b) {
	const u32 xd = a.x - b.x;
	const u32 yd = a.y - b.y;
	const u32 zd = a.z - b.z;

	return sqrtf(xd * xd + yd * yd + zd * zd);
}

float alice_v3f_dot(alice_v3f a, alice_v3f b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

alice_v3f alice_v3f_cross(alice_v3f a, alice_v3f b) {
	return (alice_v3f){
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	};
}

i32 alice_v3i_mag(alice_v3i v) {
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

alice_v3i alice_v3i_normalise(alice_v3i v) {
	const i32 l = alice_v3i_mag(v);

	if (l == 0) {
		return (alice_v3i) {0, 0, 0};
	}

	return (alice_v3i){v.x / l, v.y / l, v.z / l};
}

i32 alice_v3i_dist(alice_v3i a, alice_v3i b) {
	const i32 xd = a.x - b.x;
	const i32 yd = a.y - b.y;
	const i32 zd = a.z - b.z;

	return sqrt(xd * xd + yd * yd + zd * zd);
}

i32 alice_v3i_dot(alice_v3i a, alice_v3i b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

u32 alice_v3u_mag(alice_v3u v) {
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

alice_v3u alice_v3u_normalise(alice_v3u v) {
	const u32 l = alice_v3u_mag(v);

	if (l == 0) {
		return (alice_v3u) {0, 0, 0};
	}

	return (alice_v3u){v.x / l, v.y / l, v.z / l};
}

u32 alice_v3u_dist(alice_v3u a, alice_v3u b) {
	const u32 xd = a.x - b.x;
	const u32 yd = a.y - b.y;
	const u32 zd = a.z - b.z;

	return sqrt(xd * xd + yd * yd + zd * zd);
}

u32 alice_v3u_dot(alice_v3u a, alice_v3u b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

/* ==== 4-D VECTOR ====*/
float alice_v4f_mag(alice_v4f v) {
	return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}

alice_v4f alice_v4f_normalise(alice_v4f v) {
	const float l = alice_v4f_mag(v);

	if (l == 0.0) {
		return (alice_v4f) {0.0, 0.0, 0.0, 0.0};
	}

	return (alice_v4f){v.x / l, v.y / l, v.z / l, v.w / l};
}

float alice_v4f_dist(alice_v4f a, alice_v4f b) {
	const float xd = a.x - b.x;
	const float yd = a.y - b.y;
	const float zd = a.z - b.z;
	const float wd = a.w - b.w;

	return sqrtf(xd * xd + yd * yd + zd * zd + wd * wd);
}

float alice_v4f_dot(alice_v4f a, alice_v4f b) {
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

i32 alice_v4i_mag(alice_v4i v) {
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}

alice_v4i alice_v4i_normalise(alice_v4i v) {
	const i32 l = alice_v4i_mag(v);

	if (l == 0) {
		return (alice_v4i) {0, 0, 0, 0};
	}

	return (alice_v4i){v.x / l, v.y / l, v.z / l, v.w / l};
}

i32 alice_v4i_dist(alice_v4i a, alice_v4i b) {
	const i32 xd = a.x - b.x;
	const i32 yd = a.y - b.y;
	const i32 zd = a.z - b.z;
	const i32 wd = a.w - b.w;

	return sqrt(xd * xd + yd * yd + zd * zd + wd * wd);
}

i32 alice_v4i_dot(alice_v4i a, alice_v4i b) {
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

u32 alice_v4u_mag(alice_v4u v) {
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}

alice_v4u alice_v4u_normalise(alice_v4u v) {
	const u32 l = alice_v4u_mag(v);

	if (l == 0) {
		return (alice_v4u) {0, 0, 0, 0};
	}

	return (alice_v4u){v.x / l, v.y / l, v.z / l, v.w / l};
}

u32 alice_v4u_dist(alice_v4u a, alice_v4u b) {
	const u32 xd = a.x - b.x;
	const u32 yd = a.y - b.y;
	const u32 zd = a.z - b.z;
	const u32 wd = a.w - b.w;

	return sqrt(xd * xd + yd * yd + zd * zd + wd * wd);
}

u32 alice_v4u_dot(alice_v4u a, alice_v4u b) {
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

/* ==== TO-DEGREES ==== */
float alice_todeg(float rad) {
	return (rad * (180.0 / alice_pi));
}

alice_v2f alice_todeg_v2f(alice_v2f rad) {
	return (alice_v2f){
		alice_todeg(rad.x),
		alice_todeg(rad.y)
	};
}

alice_v3f alice_todeg_v3f(alice_v3f rad) {
	return (alice_v3f){
		alice_todeg(rad.x),
		alice_todeg(rad.y),
		alice_todeg(rad.z)
	};
}

alice_v4f alice_todeg_v4f(alice_v4f rad) {
	return (alice_v4f){
		alice_todeg(rad.x),
		alice_todeg(rad.y),
		alice_todeg(rad.z),
		alice_todeg(rad.y)
	};
}

/* ==== TO-RADIANS ==== */
float alice_torad(float deg) {
	return (deg * (alice_pi / 180.0));
}

alice_v2f alice_torad_v2f(alice_v2f deg) {
	return (alice_v2f){
		alice_torad(deg.x),
		alice_todeg(deg.y)
	};
}

alice_v3f alice_torad_v3f(alice_v3f deg) {
	return (alice_v3f){
		alice_torad(deg.x),
		alice_torad(deg.y),
		alice_torad(deg.z)
	};
}

alice_v4f alice_torad_v4f(alice_v4f deg) {
	return (alice_v4f){
		alice_torad(deg.x),
		alice_torad(deg.y),
		alice_torad(deg.z),
		alice_torad(deg.y)
	};
}

/* ==== FOUR-BY-FOUR MATRIX ==== */

alice_m4f alice_new_mf4(float diagonal) {
	alice_m4f result;

	for (u32 x = 0; x < 4; x++) {
		for (u32 y = 0; y < 4; y++) {
			result.elements[x][y] = 0.0;
		}
	}

	result.elements[0][0] = diagonal;
	result.elements[1][1] = diagonal;
	result.elements[2][2] = diagonal;
	result.elements[3][3] = diagonal;

	return result;
}

alice_m4f alice_m4f_identity() {
	return alice_new_mf4(1.0);
}

alice_m4f alice_m4f_multiply(alice_m4f a, alice_m4f b) {
	alice_m4f result = alice_m4f_identity();

	for (u32 row = 0; row < 4; row++) {
		for (u32 col = 0; col < 4; col++) {
			float sum = 0.0;

			for (u32 e = 0; e < 4; e++) {
				sum += a.elements[e][row] * b.elements[col][e];
			}

			result.elements[col][row] = sum;
		}
	}

	return result;
}

alice_m4f alice_m4f_translate(alice_m4f m, alice_v3f v) {
	alice_m4f result = alice_m4f_identity();

	result.elements[3][0] = v.x;
	result.elements[3][1] = v.y;
	result.elements[3][2] = v.z;

	return alice_m4f_multiply(m, result);
}

alice_m4f alice_m4f_rotate(alice_m4f m, float a, alice_v3f v) {
	alice_m4f result = alice_m4f_identity();

	const float r = alice_torad(a);
	const float c = cosf(r);
	const float s = sinf(r);

	const float omc = 1.0 - c;

	float x = v.x;
	float y = v.y;
	float z = v.z;

	result.elements[0][0] = x * x * omc + c;
	result.elements[0][1] = y * x * omc + z * s;
	result.elements[0][2] = x * z * omc - y * s;

	result.elements[1][0] = x * y * omc - z * s;
	result.elements[1][1] = y * y * omc + c;
	result.elements[1][2] = y * z * omc + x * s;

	result.elements[2][0] = x * z * omc + y * s;
	result.elements[2][1] = y * z * omc - x * s;
	result.elements[2][2] = z * z * omc + c;

	return alice_m4f_multiply(m, result);
}

alice_m4f alice_m4f_scale(alice_m4f m, alice_v3f v) {
	alice_m4f result = alice_m4f_identity();

	result.elements[0][0] = v.x;
	result.elements[1][1] = v.y;
	result.elements[2][2] = v.z;

	return alice_m4f_multiply(m, result);
}

alice_m4f alice_m4f_ortho(float left, float right,
	float bottom, float top, float near, float far) {
	alice_m4f result = alice_m4f_identity();

	result.elements[0][0] = 2.0 / (right - left);
	result.elements[1][1] = 2.0 / (top - bottom);
	result.elements[2][2] = 2.0 / (near - far);

	result.elements[3][0] = (left + right) / (left - right);
	result.elements[3][1] = (bottom + top) / (bottom - top);
	result.elements[3][2] = (far + near) / (far - near);

	return result;
}

alice_m4f alice_m4f_persp(float fov, float aspect, float near, float far) {
	alice_m4f result = alice_m4f_identity();

	const float q = 1.0f / tanf(alice_torad(0.5 * fov));
	const float a = q / aspect;

	const float b = (near + far) / (near - far);
	const float c = (2.0 * near * far) / (near - far);

	result.elements[0][0] = a;
	result.elements[1][1] = q;
	result.elements[2][2] = b;
	result.elements[2][3] = -1.0;
	result.elements[3][2] = c;

	return result;
}

alice_m4f alice_m4f_lookat(alice_v3f camera, alice_v3f object, alice_v3f up) {
	alice_m4f result = alice_m4f_identity();

	alice_v3f f = alice_v3f_normalise((alice_v3f){
		object.x - camera.x,
		object.y - camera.y,
		object.z - camera.z
	});

	alice_v3f u = alice_v3f_normalise(up);
	alice_v3f s = alice_v3f_normalise(alice_v3f_cross(f, u));
	u = alice_v3f_cross(s, f);

	result.elements[0][0] = s.x;
	result.elements[1][0] = s.y;
	result.elements[2][0] = s.z;
	result.elements[0][1] = u.x;
	result.elements[1][1] = u.y;
	result.elements[2][1] = u.z;
	result.elements[0][2] = -f.x;
	result.elements[1][2] = -f.y;
	result.elements[2][2] = -f.z;

	result.elements[3][0] = -alice_v3f_dot(s, camera);
	result.elements[3][1] = -alice_v3f_dot(u, camera);
	result.elements[3][2] = alice_v3f_dot(f, camera);

	return result;
}
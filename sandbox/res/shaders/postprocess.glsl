#begin VERTEX

#version 430 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;

out VS_OUT {
	vec2 uv;
} vs_out;

void main() {
	vs_out.uv = uv;
	gl_Position = vec4(position, 0.0, 1.0);
}

#end VERTEX

#begin FRAGMENT

#version 430 core

in VS_OUT {
	vec2 uv;
} fs_in;

out vec4 color;

uniform bool use_antialiasing;
uniform bool use_bloom;

uniform sampler2D input_color;
uniform sampler2D bloom_texture;
uniform vec3 color_mod = vec3(1.0);

uniform float input_width;
uniform float input_height;

uniform float exposure = 1.0;
uniform float gamma = 2.2;

vec2 texel_size = vec2(1.0 / input_width, 1.0 / input_height);
vec3 luma = vec3(0.299, 0.587, 0.114);

vec4 get_antialiased_color() {
	float FXAA_SPAN_MAX = 4.0;
	float FXAA_REDUCE_MIN = 1.0/128.0;
	float FXAA_REDUCE_MUL = 1.0/80.0;

	float luma_TL = dot(luma, texture(input_color, fs_in.uv + (vec2(-1.0, -1.0) * texel_size)).rgb);
	float luma_TR = dot(luma, texture(input_color, fs_in.uv + (vec2(1.0, -1.0) * texel_size)).rgb);
	float luma_BL = dot(luma, texture(input_color, fs_in.uv + (vec2(-1.0, 1.0) * texel_size)).rgb);
	float luma_BR = dot(luma, texture(input_color, fs_in.uv + (vec2(1.0, 1.0) * texel_size)).rgb);
	float luma_M  = dot(luma, texture(input_color, fs_in.uv).rgb);

	vec2 dir;
	dir.x = -((luma_TL + luma_TR) - (luma_BL + luma_BR));
	dir.y = ((luma_TL + luma_BL) - (luma_TR + luma_BR));

	float dir_reduce = max((luma_TL + luma_TR + luma_BL + luma_BR)
		* (FXAA_REDUCE_MUL * 0.25), FXAA_REDUCE_MIN);
	float inverse_dir_adjustment = 1.0 / (min(abs(dir.x), abs(dir.y)) + dir_reduce);

	dir = min(vec2(FXAA_SPAN_MAX),
		max(vec2(-FXAA_SPAN_MAX), dir * inverse_dir_adjustment)) * texel_size;

	vec3 result1 = (1.0 / 2.0) * (
		texture(input_color, fs_in.uv + (dir * vec2(1.0 / 3.0 - 0.5))).rgb +
		texture(input_color, fs_in.uv + (dir * vec2(2.0 / 3.0 - 0.5))).rgb);

	vec3 result2 = result1 * (1.0 / 2.0 ) + (1.0 / 4.0) * (
		texture(input_color, fs_in.uv + (dir * vec2(0.0 / 3.0 - 0.5))).rgb +
		texture(input_color, fs_in.uv + (dir * vec2(3.0 / 3.0 - 0.5))).rgb);

	float luma_min = min(luma_M, min(min(luma_TL, luma_TR), min(luma_BL, luma_BR)));
	float luma_max = max(luma_M, max(max(luma_TL, luma_TR), max(luma_BL, luma_BR)));
	float luma_result2 = dot(luma, result2);

	if (luma_result2 < luma_min || luma_result2 > luma_max) {
		return vec4(result1, 1.0);
	}

	return vec4(result2, 1.0);
}

void main() {
	vec3 hdr_color = vec3(0.0);
	if (use_antialiasing) {
		hdr_color = color_mod * get_antialiased_color().rgb;
	} else {
		hdr_color = color_mod * texture(input_color, fs_in.uv).rgb;
	}

	vec3 bloom_color = vec3(0.0);

	if (use_bloom) {
		bloom_color = texture(bloom_texture, fs_in.uv).rgb;
	}

	hdr_color += bloom_color;
	
	vec3 final = vec3(1.0) - exp(-hdr_color * exposure);

	final = pow(final, vec3(1.0 / gamma));

	color = vec4(final, 1.0);
}

#end FRAGMENT

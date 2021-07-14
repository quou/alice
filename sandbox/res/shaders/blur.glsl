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

uniform sampler2D input_color;

uniform float input_width;
uniform float input_height;

uniform bool horizontal;

uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main() {
	vec2 tex_offset = 1.0 / textureSize(input_color, 0);
	vec3 result = texture(input_color, fs_in.uv).rgb * weight[0];
	if (horizontal) {
		for (uint i = 1; i < 5; i++) {
			result += texture(input_color, fs_in.uv + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
			result += texture(input_color, fs_in.uv - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
		}
	} else {
		for (uint i = 1; i < 5; i++) {
			result += texture(input_color, fs_in.uv + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
			result += texture(input_color, fs_in.uv - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
		}
	}

	color = vec4(result, 1.0);
}  

#end FRAGMENT

#begin VERTEX

#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 color;

uniform mat4 camera = mat4(1.0);

out VS_OUT {
	vec2 uv;
	vec4 color;
} vs_out;

void main() {
	vs_out.uv = uv;
	vs_out.color = color;

	gl_Position = camera * vec4(position, 0.0, 1.0);
}

#end VERTEX

#begin FRAGMENT

#version 330 core

out vec4 color;

in VS_OUT {
	vec2 uv;
	vec4 color;
} fs_in;

uniform sampler2D atlas;

void main() {
	color = vec4(texture(atlas, fs_in.uv).r) * fs_in.color;
}

#end FRAGMENT

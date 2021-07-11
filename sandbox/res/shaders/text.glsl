#begin VERTEX

#version 430 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;

uniform mat4 camera = mat4(1.0);

out VS_OUT {
	vec2 uv;
} vs_out;

void main() {
	vs_out.uv = uv;

	gl_Position = camera * vec4(position, 0.0, 1.0);
}

#end VERTEX

#begin FRAGMENT

#version 430 core

in VS_OUT {
	vec2 uv;
} fs_in;

out vec4 color;

uniform vec3 color_mod;
uniform sampler2D font;

void main() {
	vec4 texture_color = vec4(texture(font, fs_in.uv).r);

	color = vec4(color_mod, 1.0) * texture_color;
}

#end FRAGMENT

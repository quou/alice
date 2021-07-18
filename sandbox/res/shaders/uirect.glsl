#begin VERTEX

#version 430 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec3 color;

uniform mat4 camera = mat4(1.0);

out VS_OUT {
	vec3 color;
} vs_out;

void main() {
	vs_out.color = color;

	gl_Position = camera * vec4(position, 0.0, 1.0);
}

#end VERTEX

#begin FRAGMENT

#version 430 core

in VS_OUT {
	vec3 color;
} fs_in;

out vec4 color;

void main() {
	color = vec4(fs_in.color, 1.0);
}

#end FRAGMENT

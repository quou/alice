#begin VERTEX

#version 430 core

layout (location = 0) in vec3 position;

uniform mat4 camera = mat4(1.0);
uniform mat4 transform = mat4(1.0);

void main() {
	gl_Position = camera * transform * vec4(position, 1.0);
}

#end VERTEX

#begin FRAGMENT

#version 430 core

out vec4 color;

void main() {
	color = vec4(1.0, 0.0, 1.0, 1.0);
}

#end FRAGMENT

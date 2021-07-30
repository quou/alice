#begin VERTEX

#version 430 core

layout (location = 0) in vec3 position;

uniform mat4 light;
uniform mat4 transform = mat4(1.0);

void main() {
	gl_Position = light * transform * vec4(position, 1.0);
}

#end VERTEX

#begin FRAGMENT

#version 430 core

void main() {}

#end FRAGMENT

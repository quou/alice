#begin VERTEX

#version 330 core

layout (location = 0) in vec3 position;

uniform mat4 transform = mat4(1.0);
uniform mat4 camera = mat4(1.0);

void main() {
	gl_Position = camera * transform * vec4(position, 1.0);
}

#end VERTEX

#begin FRAGMENT

#version 330 core

out vec4 color;

uniform vec3 object;

void main() {
	color = vec4(object, 1.0);
}

#end FRAGMENT

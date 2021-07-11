#begin VERTEX
#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;

out vec2 out_uv;

uniform mat4 projection;
uniform mat4 transform;

void main() {
	gl_Position = projection * transform * vec4(position, 0.0, 1.0);
	out_uv = uv;
}

#end VERTEX

#begin FRAGMENT
#version 330 core

in vec2 out_uv;
out vec4 color;

uniform sampler2D image;

void main() {
	color = texture(image, out_uv);
}

#end FRAGMENT

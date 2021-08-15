#begin VERTEX

#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 color;
layout (location = 3) in float mode;

uniform mat4 camera = mat4(1.0);

out VS_OUT {
	vec2 uv;
	vec4 color;
	float mode;
} vs_out;

void main() {
	vs_out.uv = uv;
	vs_out.color = color;
	vs_out.mode = mode;

	gl_Position = camera * vec4(position, 0.0, 1.0);
}

#end VERTEX

#begin FRAGMENT

#version 330 core

out vec4 color;

in VS_OUT {
	vec2 uv;
	vec4 color;
	float mode;
} fs_in;

uniform sampler2D atlas;
uniform sampler2D font;

void main() {
	vec4 texture_color = vec4(1.0);

	int mode = int(fs_in.mode);

	if (mode == 1) {
		texture_color = vec4(texture(atlas, fs_in.uv).r);
	} else if (mode == 2) {
		texture_color = vec4(vec3(1.0), texture(font, fs_in.uv).r); 
	}

	color = texture_color * fs_in.color;
}

#end FRAGMENT

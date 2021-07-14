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

void main() {
	vec4 texture_color = texture(input_color, fs_in.uv);

	float pixel_brightness = dot(texture_color.rgb, vec3(0.2126, 0.7152, 0.0722));
	if (pixel_brightness > 1.0) {
		color = texture_color;
	} else {
		color = vec4(0.0, 0.0, 0.0, 1.0);
	}
}

#end FRAGMENT

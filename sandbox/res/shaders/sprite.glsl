#begin VERTEX

#version 430 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 source_rect;
layout (location = 3) in vec2 texture_size;
layout (location = 4) in float texture_index;

uniform mat4 camera = mat4(1.0);

out VS_OUT {
	vec2 uv;
	float texture_index;
} vs_out;

void main() {
	float width_pixel = 1.0 / texture_size.x;
	float height_pixel = 1.0 / texture_size.y;

	float start_x = source_rect.x;
	float start_y = source_rect.y;
	float width = source_rect.z;
	float height = source_rect.w;

	vs_out.uv = vec2(
        	width_pixel * start_x + width * width_pixel * uv.x,
        	height_pixel * start_y + height * height_pixel * uv.y
	);

	vs_out.texture_index = texture_index;

	gl_Position = camera * vec4(position, 0.0, 1.0);
}

#end VERTEX

#begin FRAGMENT

#version 430 core

in VS_OUT {
	vec2 uv;
	float texture_index;
} fs_in;

out vec4 color;

uniform sampler2D textures[32];

void main() {
	vec4 texture_color;

	switch(int(fs_in.texture_index))
	{
		case 0: texture_color = texture(textures[0], fs_in.uv); break;
		case 1: texture_color = texture(textures[1], fs_in.uv); break;
		case 2: texture_color = texture(textures[2], fs_in.uv); break;
		case 3: texture_color = texture(textures[3], fs_in.uv); break;
		case 4: texture_color = texture(textures[4], fs_in.uv); break;
		case 5: texture_color = texture(textures[5], fs_in.uv); break;
		case 6: texture_color = texture(textures[6], fs_in.uv); break;
		case 7: texture_color = texture(textures[7], fs_in.uv); break;
		case 8: texture_color = texture(textures[8], fs_in.uv); break;
		case 9: texture_color = texture(textures[9], fs_in.uv); break;
		case 10: texture_color = texture(textures[10], fs_in.uv); break;
		case 11: texture_color = texture(textures[11], fs_in.uv); break;
		case 12: texture_color = texture(textures[12], fs_in.uv); break;
		case 13: texture_color = texture(textures[13], fs_in.uv); break;
		case 14: texture_color = texture(textures[14], fs_in.uv); break;
		case 15: texture_color = texture(textures[15], fs_in.uv); break;
		case 16: texture_color = texture(textures[16], fs_in.uv); break;
		case 17: texture_color = texture(textures[17], fs_in.uv); break;
		case 18: texture_color = texture(textures[18], fs_in.uv); break;
		case 19: texture_color = texture(textures[19], fs_in.uv); break;
		case 20: texture_color = texture(textures[20], fs_in.uv); break;
		case 21: texture_color = texture(textures[21], fs_in.uv); break;
		case 22: texture_color = texture(textures[22], fs_in.uv); break;
		case 23: texture_color = texture(textures[23], fs_in.uv); break;
		case 24: texture_color = texture(textures[24], fs_in.uv); break;
		case 25: texture_color = texture(textures[25], fs_in.uv); break;
		case 26: texture_color = texture(textures[26], fs_in.uv); break;
		case 27: texture_color = texture(textures[27], fs_in.uv); break;
		case 28: texture_color = texture(textures[28], fs_in.uv); break;
		case 29: texture_color = texture(textures[29], fs_in.uv); break;
		case 30: texture_color = texture(textures[30], fs_in.uv); break;
		case 31: texture_color = texture(textures[31], fs_in.uv); break;
	}

	color = texture_color;
	//color = vec4(1.0);
}

#end FRAGMENT

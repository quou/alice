#begin VERTEX

#version 430 core

layout (location = 0) in vec3 position;

uniform mat4 transform = mat4(1.0);

void main() {
	gl_Position = transform * vec4(position, 1.0);
}

#end VERTEX

#begin GEOMETRY

#version 430 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

uniform mat4 shadow_matrices[6];

out vec4 frag_pos;

void main() {
	for (int face = 0; face < 6; face++) {
		gl_Layer = face;
		for (int i = 0; i < 3; i++) {
			frag_pos = gl_in[i].gl_Position;
			gl_Position = shadow_matrices[face] * frag_pos;
			EmitVertex();
		}

		EndPrimitive();
	}
}

#end GEOMETRY

#begin FRAGMENT

#version 430 core

in vec4 frag_pos;

uniform vec3 light_position;
uniform float far;

void main() {
	float light_distance = length(frag_pos.xyz - light_position);

	light_distance = light_distance / far;

	gl_FragDepth = light_distance;
}

#end FRAGMENT

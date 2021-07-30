#begin VERTEX

#version 430 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

uniform mat4 camera = mat4(1.0);
uniform mat4 transform = mat4(1.0);

out VS_OUT {
	vec3 normal;
	vec2 uv;
	vec3 world_pos;
} vs_out;

void main() {
	vs_out.uv = uv;
	vs_out.normal = mat3(transform) * normal;
	vs_out.world_pos = vec3(transform * vec4(position, 1.0));

	gl_Position = camera * vec4(vs_out.world_pos, 1.0);
}

#end VERTEX

#begin FRAGMENT

#version 430 core

in VS_OUT {
	vec3 normal;
	vec2 uv;
	vec3 world_pos;
} fs_in;

struct Material {
	vec3 diffuse;
	vec3 specular;
	vec3 ambient;

	float shininess;
	float emissive;

	sampler2D diffuse_map;
	bool use_diffuse_map;
};

struct PointLight {
	vec3 position;
	vec3 color;
	float intensity;
	float range;
};

struct DirectionalLight {
	vec3 direction;
	vec3 color;
	float intensity;

	mat4 transform;
};

uniform PointLight point_lights[100];
uniform uint point_light_count = 0;

uniform DirectionalLight directional_lights[100];
uniform uint directional_light_count = 0;

uniform sampler2D shadowmap;

out vec4 color;

uniform vec3 camera_position;

uniform Material material;
uniform float ambient_intensity;
uniform vec3 ambient_color;

const float PI = 3.14159265359;

uniform float gamma = 2.2;

/*vec3 get_normal_from_map() {
	vec3 tangent_normal = texture(material.normal_map, fs_in.uv).xyz * 2.0 - 1.0;

	vec3 Q1  = dFdx(fs_in.world_pos);
	vec3 Q2  = dFdy(fs_in.world_pos);
	vec2 st1 = dFdx(fs_in.uv);
	vec2 st2 = dFdy(fs_in.uv);

	vec3 N   = normalize(fs_in.normal);
	vec3 T  = normalize(Q1 * st2.t - Q2 * st1.t);
	vec3 B  = -normalize(cross(N, T));
	mat3 TBN = mat3(T, B, N);

	return normalize(TBN * tangent_normal);
}*/

float calculate_directional_shadow(DirectionalLight light, vec3 normal, vec3 light_dir) {
	vec4 light_space_pos = light.transform * vec4(fs_in.world_pos, 1.0);
	vec3 proj_coords = light_space_pos.xyz / light_space_pos.w;
	proj_coords = (proj_coords * 0.5) + 0.5;

	if (proj_coords.z > 1.0) {
		return 0.0;
	}

	float closest_depth = texture(shadowmap, proj_coords.xy).r;
	float current_depth = proj_coords.z;
	
	float bias = max(0.05 * (1.0 - dot(normal, light_dir)), 0.005);

	float shadow = current_depth - bias > closest_depth ? 1.0 : 0.0;

	return shadow;
}

vec3 calculate_directional_light(DirectionalLight light, vec3 normal, vec3 view_dir) {
	vec3 light_dir = normalize(-light.direction);
	vec3 halfway_dir = normalize(light_dir + view_dir);

	float diff = max(dot(normal, light_dir), 0.0);

	vec3 reflect_dir = reflect(-light_dir, normal);
	float spec = pow(max(dot(normal, halfway_dir), 0.0), material.shininess);

	vec3 diffuse = light.color * light.intensity * diff * material.diffuse;
	vec3 specular = light.color * light.intensity * spec * material.specular;

	return (1.0 - calculate_directional_shadow(light, normal, view_dir)) * (diffuse + specular);
}

vec3 calculate_point_light(PointLight light, vec3 normal, vec3 view_dir) {
	vec3 light_dir = normalize(light.position - fs_in.world_pos);
	vec3 halfway_dir = normalize(light_dir + view_dir);

	float diff = max(dot(normal, light_dir), 0.0);

	vec3 reflect_dir = reflect(-light_dir, normal);
	float spec = pow(max(dot(normal, halfway_dir), 0.0), material.shininess);

	float dist = length(light.position - fs_in.world_pos);
	float attenuation = 1.0 / (pow((dist / light.range) * 5.0, 2.0) + 1.0);

	vec3 diffuse = light.color * light.intensity * diff * material.diffuse;
	vec3 specular = light.color * light.intensity * spec * material.specular;

	diffuse *= attenuation;
	specular *= attenuation;

	return (diffuse + specular);
}

void main() {
	vec3 texture_color = vec3(1.0);

	vec3 normal = normalize(fs_in.normal);
	vec3 view_dir = normalize(camera_position - fs_in.world_pos);

	if (material.use_diffuse_map) {
		texture_color = texture(material.diffuse_map, fs_in.uv).rgb;
	}

	vec3 lighting_result = material.ambient * ambient_intensity * ambient_color;
	lighting_result += material.emissive;

	for (uint i = 0; i < directional_light_count; i++) {
		lighting_result += calculate_directional_light(directional_lights[i], normal, view_dir);
	}

	for (uint i = 0; i < point_light_count; i++) {
		lighting_result += calculate_point_light(point_lights[i], normal, view_dir);
	}

	color = vec4(lighting_result * texture_color, 1.0);
}

#end FRAGMENT

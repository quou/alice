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
	vec3 albedo;
	float roughness;
	float metallic;

	float emissive;

	bool use_albedo_map;
	sampler2D albedo_map;

	bool use_metallic_map;
	sampler2D metallic_map;

	bool use_roughness_map;
	sampler2D roughness_map;

	bool use_normal_map;
	sampler2D normal_map;

	bool use_ambient_occlusion_map;
	sampler2D ambient_occlusion_map;
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
};

uniform PointLight point_lights[100];
uniform uint point_light_count = 0;

uniform DirectionalLight directional_lights[100];
uniform uint directional_light_count = 0;

out vec4 color;

uniform vec3 camera_position;

uniform Material material;

const float PI = 3.14159265359;

vec3 albedo = vec3(0.0);
vec3 normal = vec3(0.0);
float metallic = 0.0;
float roughness = 0.0;

vec3 get_normal_from_map() {
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
}

float distribution_ggx(vec3 N, vec3 H, float roughness) {
	float a = roughness * roughness;
	float a2 = a*a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH*NdotH;

	float nom   = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return nom / max(denom, 0.0000001);
}

float geometry_schlick_ggx(float NdotV, float roughness) {
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;

	float nom   = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}

float geometry_smith(vec3 N, vec3 V, vec3 L, float roughness) {
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = geometry_schlick_ggx(NdotV, roughness);
	float ggx1 = geometry_schlick_ggx(NdotL, roughness);

	return ggx1 * ggx2;
}

vec3 fresnel_schlick(float cosTheta, vec3 F0) {
	return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

vec3 calculate_point_light(PointLight light, vec3 N, vec3 V, vec3 F0) {
	vec3 L = normalize(light.position - fs_in.world_pos);
	vec3 H = normalize(V + L);
	float distance = length(light.position - fs_in.world_pos);
	float attenuation = (1.0 / (distance * distance)) * light.range;
	vec3 radiance = light.color * light.intensity * attenuation;

	float NDF = distribution_ggx(N, H, roughness);
	float G = geometry_smith(N, V, L, roughness);
	vec3 F = fresnel_schlick(clamp(dot(H, V), 0.0, 1.0), F0);

	vec3 numerator	= NDF * G * F;
	float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
	vec3 specular = numerator / max(denominator, 0.001);

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;

	kD *= 1.0 - metallic;

	float NdotL = max(dot(N, L), 0.0);

	return (kD * albedo / PI + specular) * radiance * NdotL;
}

vec3 calculate_directional_light(DirectionalLight light, vec3 N, vec3 V, vec3 F0) {
	vec3 L = normalize(normalize(-light.direction) - fs_in.world_pos);
	vec3 H = normalize(V + L);
	vec3 radiance = light.color * light.intensity;

	float NDF = distribution_ggx(N, H, roughness);
	float G = geometry_smith(N, V, L, roughness);
	vec3 F = fresnel_schlick(clamp(dot(H, V), 0.0, 1.0), F0);

	vec3 numerator	= NDF * G * F;
	float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
	vec3 specular = numerator / max(denominator, 0.001);

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;

	kD *= 1.0 - metallic;

	float NdotL = max(dot(N, L), 0.0);

	return (kD * albedo / PI + specular) * radiance * NdotL;
}

void main() {
	normal = normalize(fs_in.normal);
	vec3 view_dir = normalize(camera_position - fs_in.world_pos);

	albedo = material.albedo;
	metallic = material.metallic;
	roughness = material.roughness;

	if (material.use_albedo_map) {
		albedo = material.albedo * texture2D(material.albedo_map, fs_in.uv).rgb;
	}

	if (material.use_normal_map) {
		normal = get_normal_from_map();
	}

	if (material.use_roughness_map) {
		roughness = material.roughness * texture2D(material.roughness_map, fs_in.uv).r;
	}

	if (material.use_metallic_map) {
		metallic = material.metallic * texture2D(material.metallic_map, fs_in.uv).r;
	}

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, material.metallic);

	vec3 lighting_result = vec3(0.0);

	for (uint i = 0; i < point_light_count; i++) {
		lighting_result += calculate_point_light(point_lights[i], normal, view_dir, F0);
	}

	for (uint i = 0; i < directional_light_count; i++) {
		lighting_result += calculate_directional_light(directional_lights[i],
				normal, view_dir, F0);
	}

	float ao = 1.0;
	if (material.use_ambient_occlusion_map) {
		ao = texture(material.ambient_occlusion_map, fs_in.uv).r;
	}

	vec3 ambient = vec3(0.003 + material.emissive) * albedo * ao;

	color = vec4(ambient + lighting_result, 1.0);
}

#end FRAGMENT

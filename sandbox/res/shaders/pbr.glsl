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

	mat4 transform;
};

uniform PointLight point_lights[100];
uniform uint point_light_count = 0;

uniform DirectionalLight directional_lights[100];
uniform uint directional_light_count = 0;

uniform sampler2DShadow shadowmap;
uniform bool use_shadows = false;

out vec4 color;

uniform vec3 camera_position;

uniform Material material;

const float PI = 3.14159265359;

vec3 albedo = vec3(0.0);
vec3 normal = vec3(0.0);
float metallic = 0.0;
float roughness = 0.0;

uniform float gamma = 2.2;

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

vec2 poisson_disk[64] = vec2[]( 
	vec2(-0.5119625f, -0.4827938f),
	vec2(-0.2171264f, -0.4768726f),
	vec2(-0.7552931f, -0.2426507f),
	vec2(-0.7136765f, -0.4496614f),
	vec2(-0.5938849f, -0.6895654f),
	vec2(-0.3148003f, -0.7047654f),
	vec2(-0.42215f, -0.2024607f),
	vec2(-0.9466816f, -0.2014508f),
	vec2(-0.8409063f, -0.03465778f),
	vec2(-0.6517572f, -0.07476326f),
	vec2(-0.1041822f, -0.02521214f),
	vec2(-0.3042712f, -0.02195431f),
	vec2(-0.5082307f, 0.1079806f),
	vec2(-0.08429877f, -0.2316298f),
	vec2(-0.9879128f, 0.1113683f),
	vec2(-0.3859636f, 0.3363545f),
	vec2(-0.1925334f, 0.1787288f),
	vec2(0.003256182f, 0.138135f),
	vec2(-0.8706837f, 0.3010679f),
	vec2(-0.6982038f, 0.1904326f),
	vec2(0.1975043f, 0.2221317f),
	vec2(0.1507788f, 0.4204168f),
	vec2(0.3514056f, 0.09865579f),
	vec2(0.1558783f, -0.08460935f),
	vec2(-0.0684978f, 0.4461993f),
	vec2(0.3780522f, 0.3478679f),
	vec2(0.3956799f, -0.1469177f),
	vec2(0.5838975f, 0.1054943f),
	vec2(0.6155105f, 0.3245716f),
	vec2(0.3928624f, -0.4417621f),
	vec2(0.1749884f, -0.4202175f),
	vec2(0.6813727f, -0.2424808f),
	vec2(-0.6707711f, 0.4912741f),
	vec2(0.0005130528f, -0.8058334f),
	vec2(0.02703013f, -0.6010728f),
	vec2(-0.1658188f, -0.9695674f),
	vec2(0.4060591f, -0.7100726f),
	vec2(0.7713396f, -0.4713659f),
	vec2(0.573212f, -0.51544f),
	vec2(-0.3448896f, -0.9046497f),
	vec2(0.1268544f, -0.9874692f),
	vec2(0.7418533f, -0.6667366f),
	vec2(0.3492522f, 0.5924662f),
	vec2(0.5679897f, 0.5343465f),
	vec2(0.5663417f, 0.7708698f),
	vec2(0.7375497f, 0.6691415f),
	vec2(0.2271994f, -0.6163502f),
	vec2(0.2312844f, 0.8725659f),
	vec2(0.4216993f, 0.9002838f),
	vec2(0.4262091f, -0.9013284f),
	vec2(0.2001408f, -0.808381f),
	vec2(0.149394f, 0.6650763f),
	vec2(-0.09640376f, 0.9843736f),
	vec2(0.7682328f, -0.07273844f),
	vec2(0.04146584f, 0.8313184f),
	vec2(0.9705266f, -0.1143304f),
	vec2(0.9670017f, 0.1293385f),
	vec2(0.9015037f, -0.3306949f),
	vec2(-0.5085648f, 0.7534177f),
	vec2(0.9055501f, 0.3758393f),
	vec2(0.7599946f, 0.1809109f),
	vec2(-0.2483695f, 0.7942952f),
	vec2(-0.4241052f, 0.5581087f),
	vec2(-0.1020106f, 0.6724468f)
);

float random(vec3 seed, int i) {
	vec4 seed4 = vec4(seed, i);
	float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
	return fract(sin(dot_product) * 43758.5453);
}

float calculate_directional_shadow(DirectionalLight light, vec3 normal, vec3 light_dir) {
	if (!use_shadows) { return 0.0; }

	vec4 light_space_pos = light.transform * vec4(fs_in.world_pos, 1.0);
	vec3 proj_coords = light_space_pos.xyz / light_space_pos.w;
	proj_coords = (proj_coords * 0.5) + 0.5;

	float bias = 0.008;

	float shadow = 0.0f;

	vec2 texel_size = 1.0 / textureSize(shadowmap, 0);

	for (int i = 0; i < 4; i++) {
		int index = int(64.0 * random(floor(fs_in.world_pos.xyz * 1000.0), i)) % 64;

		shadow += texture(shadowmap,
				vec3(proj_coords.xy + poisson_disk[index] / 700.0f,
					proj_coords.z - bias)).r;
	}

	shadow /= 4.0;

	if (proj_coords.z > 1.0) {
		shadow = 1.0;
	}

	return shadow;
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
	vec3 L = normalize(-light.direction);
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

	return calculate_directional_shadow(light, N, V) * ((kD * albedo / PI + specular) * radiance * NdotL);
}

void main() {
	normal = normalize(fs_in.normal);
	vec3 view_dir = normalize(camera_position - fs_in.world_pos);

	albedo = material.albedo;
	metallic = material.metallic;
	roughness = material.roughness;

	if (material.use_albedo_map) {
		albedo = material.albedo * pow(texture2D(material.albedo_map, fs_in.uv).rgb, vec3(gamma));
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
	F0 = mix(F0, albedo, metallic);

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

	vec3 ambient = vec3(0.03 + material.emissive) * albedo * ao;

	color = vec4(ambient + lighting_result, 1.0);
}

#end FRAGMENT

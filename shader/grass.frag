#version 460

layout (location = 0) in vec3 p;
layout (location = 1) in vec3 n;
layout (location = 2) in vec2 uv;

layout (set = 0, binding = 1) uniform sampler2D tex;

layout (location = 0) out vec4 fragcolor;

struct point {
	vec3 p;
	vec3 color;
};

vec3 phong(in point l, in vec3 c) {
	vec3 ldir = l.p - p;
	
	float dist = length(ldir);

	const vec3 cf = vec3(1.0, 0.0, 0.0);
	float falloff = cf.x + (cf.y / dist) + (cf.z / (dist * dist));
	ldir /= dist;

	vec3 nn = normalize(n);

	float diff = clamp(dot(ldir, nn), 0.0, 1.0);

	vec3 amb = 0.15 * c;
	vec3 diffc = mix(amb, c * l.color, diff);

	/*
	vec3 eyedir = normalize(eye - p);
	float spec = clamp(dot(reflect(-ldir, nn), eyedir), 0.0, 1.0);
	spec = pow(spec, 150);

	vec3 specc = l.color * spec;

	return (diffc + specc) * falloff;
	*/
	
	return diffc * falloff;
}

void main() {
	vec4 raw = texture(tex, uv);
	// discarding if not 1.0 leads to aliasing issues on the edge of the texture
	// discarding if not 0.0 leads to transparency problems
	if (raw.a <= 0.9) {
		discard;
	}

	const point l = point(vec3(0.0, 10.0, -12.0), vec3(0.5));

	vec3 c = phong(l, raw.rgb);

	fragcolor = vec4(c, raw.a);
}

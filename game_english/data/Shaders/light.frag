#version 150 core
uniform vec4 colour;
in vec2 texture_coordinates;
out vec4 fragment;
void main()
{
	vec2 pos = texture_coordinates * 2.0 - 1.0;
	float dist = length(pos);
	float intensity = 1.0 - clamp(dist, 0.0, 1.0);
	intensity = smoothstep(0.0, 1.0, intensity);
	fragment = vec4(colour.rgb * intensity * colour.a, 1.0);
}

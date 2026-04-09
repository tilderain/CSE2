#version 150 core
uniform sampler2D tex;
uniform sampler2D lightmap;
in vec2 texture_coordinates;
out vec4 fragment;
void main()
{
	vec4 scene = texture(tex, texture_coordinates);
	vec4 light = texture(lightmap, texture_coordinates);
	fragment = vec4(scene.rgb * light.rgb, scene.a);
}

#version 150 core
uniform sampler2D tex;
uniform sampler2D lightmap;
in vec2 texture_coordinates;
out vec4 fragment;
void main()
{
	vec4 scene = texture(tex, texture_coordinates);
	vec4 light = texture(lightmap, texture_coordinates);
    
	// Multiply by 2.0 to allow overbrightening
	fragment = vec4(scene.rgb * light.rgb * 2.0, scene.a);
}

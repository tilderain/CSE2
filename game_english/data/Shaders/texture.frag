#version 150 core
uniform sampler2D tex;
in vec2 texture_coordinates;
out vec4 fragment;
void main()
{
	fragment = texture(tex, texture_coordinates);
}

#version 150 core
in vec2 input_vertex_coordinates;
in vec2 input_texture_coordinates;
out vec2 texture_coordinates;
void main()
{
	texture_coordinates = input_texture_coordinates;
	gl_Position = vec4(input_vertex_coordinates.xy, 0.0, 1.0);
}

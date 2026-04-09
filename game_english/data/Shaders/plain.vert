#version 150 core
in vec2 input_vertex_coordinates;
void main()
{
	gl_Position = vec4(input_vertex_coordinates.xy, 0.0, 1.0);
}

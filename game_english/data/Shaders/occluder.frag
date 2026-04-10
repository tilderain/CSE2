// occluder.frag
#version 150 core
uniform sampler2D tex;
in vec2 texture_coordinates;
out vec4 fragment;

void main()
{
    vec4 texColor = texture(tex, texture_coordinates);
    
    // Discard transparent parts of the tile so shadows follow the sprite shape
    if (texColor.a < 0.5) discard; 
    
    // Output white (1.0) into the occlusion map to cast a shadow
    fragment = vec4(1.0, 1.0, 1.0, 1.0);
}

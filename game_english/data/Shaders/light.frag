#version 150 core

uniform vec4 colour;
uniform vec2 lightPos;
uniform sampler2D occlusionMap;
uniform vec2 texSize;

in vec2 texture_coordinates;
out vec4 fragment;

void main()
{
    vec2 pos = texture_coordinates * 2.0 - 1.0;
    float dist = length(pos);
    if (dist > 1.0) discard;

    float intensity = 1.0 - clamp(dist, 0.0, 1.0);
    intensity = smoothstep(0.0, 1.0, intensity);

    // --- RAYTRACING ---
float shadow = 1.0;
const int steps = 30;
const float occlusionStrength = 0.05; // how much each fully-opaque sample costs

vec2 currentPixel = gl_FragCoord.xy;
vec2 lightDir = lightPos - currentPixel;

for (int i = 2; i < steps; i++)
{
    float t = float(i) / float(steps);
    vec2 samplePos = currentPixel + (lightDir * t);
    vec2 sampleUV = samplePos / texSize;

    float occ = texture(occlusionMap, sampleUV).r;
    shadow -= occ * occlusionStrength;
    if (shadow <= 0.0) { shadow = 0.0; break; }
}

    fragment = vec4(colour.rgb * intensity * shadow * colour.a, 1.0);
}

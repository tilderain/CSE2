#version 150 core

uniform vec4 colour;         // rgb = color, a = brightness/intensity
uniform vec2 lightPos;       // Light center in screen pixels
uniform float lightRadius;    
uniform sampler2D occlusionMap; 
uniform sampler2D normalMap;    

in vec2 texture_coordinates;
out vec4 fragment;

void main()
{
    // 1. Radial Falloff Calculation
    vec2 pos = texture_coordinates * 2.0 - 1.0;
    float dist = length(pos);
    if (dist > 1.0) discard;

    float baseFalloff = 1.0 - dist;
    
    // Radial Falloff for the "ambient" part (so the quad edges are invisible)
    float radialAlpha = smoothstep(1.0, 0.0, dist);

    // Overbright "hot" core
    float glow = pow(baseFalloff, 6.0) * 3.0; 
    float intensity = (baseFalloff + glow) * colour.a;

    // 2. Bump Mapping
    vec2 screenUV = gl_FragCoord.xy / textureSize(normalMap, 0);
    vec3 normal = texture(normalMap, screenUV).rgb * 2.0 - 1.0;
    
    // FIX: Increase Z height significantly (30.0 to 60.0). 
    // This allows light to "rain down" on the flat tops of sprites, 
    // not just hit the side edges.
    vec3 dirToLight = normalize(vec3(lightPos - gl_FragCoord.xy, 40.0));
    
    // Lambertian Dot Product
    float diff = max(dot(normal, dirToLight), 0.0);
    
    // FIX: The Normal Map effect (diff) is mixed with a base level (0.3).
    // Then the ENTIRE result is multiplied by intensity.
    // This ensures that highlights fall off as distance increases.
    float lamb = mix(0.3, 1.0, diff);
    float finalLighting = lamb * intensity;

    // 3. 2D Raytracing (Fading Shadows)
    float shadow = 1.0;
    const int steps = 32; 
    const float occlusionStrength = 0.08; 
    vec2 currentPixel = gl_FragCoord.xy;
    vec2 rayDir = (lightPos - currentPixel);

    for(int i = 2; i < steps; i++)
    {
        float t = float(i) / float(steps);
        vec2 sampleUV = (currentPixel + (rayDir * t)) / textureSize(occlusionMap, 0);
        
        float occ = texture(occlusionMap, sampleUV).r;
        shadow -= occ * occlusionStrength;
        if (shadow <= 0.0) { shadow = 0.0; break; }
    }

    // 4. Final Combine
    // We multiply by 2.0 to allow for HDR-like saturation
    vec3 finalRGB = colour.rgb * finalLighting * shadow * 2.0;
    
    fragment = vec4(finalRGB, 1.0);
}

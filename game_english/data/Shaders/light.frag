#version 150 core
uniform vec4 colour;
uniform vec2 lightPos;
uniform sampler2D occlusionMap;
uniform sampler2D normalMap;
uniform vec2 texSize;
in vec2 texture_coordinates;
out vec4 fragment;

void main()
{
    // 1. Radial Falloff
    vec2 pos = texture_coordinates * 2.0 - 1.0;
    float dist = length(pos);
    if (dist > 1.0) discard;

    float intensity = 1.0 - clamp(dist, 0.0, 1.0);
    intensity = smoothstep(0.0, 1.0, intensity);

    // 2. Normal Map Lighting
    vec2 screenUV = gl_FragCoord.xy / texSize;
    vec4 normSample = texture(normalMap, screenUV);

    vec3 normal = normSample.rgb * 2.0 - 1.0;

    const float lightZ = 60.0;
    const float heightScale = 20.0;
    float currentHeight = normSample.a * heightScale;

    vec2 toLight = lightPos - gl_FragCoord.xy;

    const float flipX = 1.0;
    const float flipY = -1.0;
    vec3 dirToLight = normalize(vec3(toLight.x * flipX, toLight.y * flipY, lightZ - currentHeight));
    float diff = max(dot(normal, dirToLight), 0.0);

    // 3. Raytracing
    float occAccum = 0.0;
    float maxBlockerSlope = -1000.0;
    bool blocked = false;

    const int steps = 48;
    const float occlusionStrength = 0.03;
    const float shadowSensitivity = 0.5;

    vec2 currentPixel = gl_FragCoord.xy;
    vec2 lightDir = lightPos - currentPixel;
    float rayLength = length(lightDir);

    float distanceFactor = clamp(rayLength / 300.0, 0.0, 1.0);
    float lightSlope = (lightZ - currentHeight) / (rayLength + 0.001);

    for (int i = steps - 1; i >= 1; i--)
    {
        float t = mix(0.08, 1.0, float(i) / float(steps));
        vec2 samplePos = currentPixel + (lightDir * t);
        vec2 sampleUV = samplePos / texSize;

        // --- Wall occlusion (soft when close, hard when far) ---
        float occ = texture(occlusionMap, sampleUV).r;
        float weight = 1.0 - t;
        occAccum += occ * occlusionStrength * (1.0 + weight * 2.0) * (0.2 + distanceFactor * 0.8);

        // --- Height self-shadowing ---
        float sampleHeight = texture(normalMap, sampleUV).a * heightScale;
        float distToSample = t * rayLength;
        float blockerSlope = (sampleHeight - currentHeight) / (distToSample + 0.001);
        float weightedSlope = blockerSlope * (1.0 - t);
        maxBlockerSlope = max(maxBlockerSlope, weightedSlope);

        if (weightedSlope >= lightSlope * shadowSensitivity) blocked = true;
    }

    occAccum = clamp(occAccum, 0.0, 1.0);

    float penumbra = clamp(lightSlope * 0.4, 0.01, 0.3);
    float heightShadow = blocked ? 0.0 : 1.0 - smoothstep(lightSlope - penumbra, lightSlope + penumbra, maxBlockerSlope);

    float shadow = (1.0 - occAccum) * heightShadow;

// 4. Final Combine
    const float ambient = 0.05;
    
    // FIX 1: Multiply shadow ONLY by the diff component. 
    // This allows the ambient floor to survive inside shadowed areas.
    float lighting = ambient + (1.0 - ambient) * (diff * shadow);

    // The radial falloff (intensity) and the alpha still fade the overall light out at the edges
    float baseLighting = intensity * colour.a * lighting;
    
    // Multiply by 2.0 to allow overbright on lit faces
    vec3 litColour = colour.rgb * baseLighting * 2.0;

    fragment = vec4(litColour, 1.0);
}

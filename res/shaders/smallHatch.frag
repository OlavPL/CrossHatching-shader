#version 430 core

in layout(location = 0) vec3 normal_in;
in layout(location = 1) vec2 textureCoordinates;
in layout(location = 2) vec3 fragPos;

uniform layout(location = 8) float shinyness;

out vec4 color;

// Voronoi function
float voronoi(vec2 uv) {
    float minDist = 1.0;
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            vec2 gridPos = vec2(x, y);
            vec2 randomPos = fract(sin(vec2(dot(uv + gridPos, vec2(123.4, 748.6)), dot(uv + gridPos, vec2(547.3, 659.3)))) * 43758.5453);
            vec2 pos = gridPos + randomPos - uv;
            float dist = dot(pos, pos);
            minDist = min(minDist, dist);
        }
    }
    return sqrt(minDist);
}

void main()
{
    float waveFrequency = 7.0; // Adjust this to change the frequency of the waves
    float waveAmplitude = 5.0; // Adjust this to change the amplitude of the waves

    float noiseFrequency = 1.3; // Adjust this to change the frequency of the noise
    float noiseAmplitude = 0.02; // Adjust this to change the amplitude of the noise

    // Project the fragment position onto the plane defined by the normal
    vec3 proj = fragPos - dot(fragPos, normal_in) * normal_in;

    // Apply Voronoi function to the rotation
    float voronoiValue = voronoi(proj.xy);
    float rotation = voronoiValue * 2.0 * 3.14159; // Convert Voronoi value to rotation angle in radians

    // Rotate the coordinates
    vec2 rotatedPos = vec2(proj.x * cos(rotation) - proj.y * sin(rotation), proj.x * sin(rotation) + proj.y * cos(rotation));

    // Use a sine function to create the waves, adding noise to the argument
    float wave = 0.5 + 0.5 * sin(waveFrequency * cos(rotatedPos.x) * noiseFrequency * sin(rotatedPos.x * noiseAmplitude * (rotatedPos.x *0.335) )*0.2 + waveAmplitude * rotatedPos.y);

    color = vec4(vec3(wave), 1.0);
}
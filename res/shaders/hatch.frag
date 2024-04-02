#version 430 core

in layout(location = 0) vec3 normal_in;
in layout(location = 1) vec2 textureCoordinates;
in layout(location = 2) vec3 fragPos;
in layout(location = 3) vec3 cameraPos;
in layout(location = 14) mat4 projection_in;
in layout(location = 5) mat4 cameraTransform_in;
in layout(location = 10) mat4 modelMatrix_in;

uniform layout(location = 8) float shinyness;

out vec4 color;

float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453); }
float dither(vec2 uv) { return (rand(uv)*2.0-1.0) / 256.0; }

 vec3 reject(vec3 from, vec3 onto) {
    return from - onto*dot(from, onto)/dot(onto, onto);
}

struct LightStruct
{
    vec3 position;
    vec3 color;
};

uniform LightStruct lightArray[3]; 

void main()
{

vec3 normal = normalize(normal_in);
    vec3 viewDir = normalize(cameraPos - fragPos);

    //Shadows
    vec3 fragToLight0 = lightArray[0].position - fragPos;
//    vec3 fragToLight1 = lightArray[1].position - fragPos;
//    vec3 fragToLight2 = lightArray[2].position - fragPos;

    // attenuation
    float d1 = length(fragPos - lightArray[0].position);
    float d2 = length(fragPos - lightArray[1].position);
    float d3 = length(fragPos - lightArray[2].position);
    float la = 0.0001;
    float lb = 0.0002;
    float lc = 0.00075;

    float L1 = 1 / (la + d1 * lb + pow(d1, 2) * lc); 
    float L3 = 1 / (la + d3 * lb + pow(d3, 2) * lc); 
    float L2 = 1 / (la + d2 * lb + pow(d2, 2) * lc); 

    // Diffuse lighting
    vec3 vecToLight0 = normalize(lightArray[0].position - fragPos);
    vec3 vecToLight1 = normalize(lightArray[1].position - fragPos);
    vec3 vecToLight2 = normalize(lightArray[2].position - fragPos);
//    vec3 light1Color = max(0, dot(vecToLight0, normal)) * lightArray[0].color;
//    vec3 light2Color = max(0, dot(vecToLight1, normal)) * lightArray[1].color;
//    vec3 light3Color = max(0, dot(vecToLight2, normal)) * lightArray[2].color

    vec3 light1Color = max(0, dot(vecToLight0, normal)) * lightArray[0].color;
    vec3 light2Color = max(0, dot(vecToLight1, normal)) * lightArray[1].color;
    vec3 light3Color = max(0, dot(vecToLight2, normal)) * lightArray[2].color;


    vec3 diffuse = light1Color * L1 + light2Color * L2 + light3Color * L3;



    //Specular lighting
    vec3 specular = pow(max(dot(viewDir, reflect(-vecToLight0, normal)),  0.0), shinyness) * lightArray[0].color * L1
                 + pow(max(dot(viewDir, reflect(-vecToLight1, normal)),  0.0), shinyness) * lightArray[1].color * L2
                 + pow(max(dot(viewDir, reflect(-vecToLight2, normal)),  0.0), shinyness) * lightArray[2].color * L3;

    float dither = dither(textureCoordinates);

    float waveFrequency = 30.0; // Adjust this to change the frequency of the waves
    float waveAmplitude = 0.2; // Adjust this to change the amplitude of the waves

    float noiseFrequency = 1.3; // Adjust this to change the frequency of the noise
    float noiseAmplitude = 0.02; // Adjust this to change the amplitude of the noise

// convert normal to cameraspace normal
    vec3 normalCameraSpace = mat3(transpose(inverse(cameraTransform_in))) * normal;
    vec3 posModelSpace = vec3(inverse(modelMatrix_in) * vec4(fragPos, 1.0));
    vec3 normalModelSpace = vec3(inverse(modelMatrix_in) * vec4(normal, 0.0));
    // make circular things go -1 to 1 from left around top, right, bottom back to left and rotate the position by this
    float arctan = atan(normalModelSpace.y, normalModelSpace.x) / 3.14159;
    arctan = (arctan + 1) / 2;

    arctan *= 8.0;
    arctan = trunc(arctan);
    arctan /= 8.0;
    arctan *= -32;

    float cosTheta = cos(arctan);
    float sinTheta = sin(arctan);

    mat3 rotationMatrix = mat3(
        cosTheta, -sinTheta, 0,
        sinTheta, cosTheta, 0,
        0, 0, 1
    );
    vec4 fragPosCameraSpace = vec4(fragPos, 1.0f); // Assuming fragPos is a glm::vec3
    mat4 viewMatrixInverse = inverse(cameraTransform_in); // Assuming cameraTransform is your view matrix
    vec3 fragPosWorldSpace = vec3(viewMatrixInverse * fragPosCameraSpace);
                
    vec3 proj = fragPos;
    proj *= rotationMatrix;
//

    float tileWidth = 0.2; // Adjust this to change the width of the tiles
    float wave = mod(proj.y, tileWidth) / tileWidth; // x is now in the range [0, 1] and repeats every tileWidth units
    wave += 0.5;
    wave = floor(wave);
   
    vec3 lightColors = vec3(diffuse + specular + dither);
    float lightIntensity = lightColors.x * 0.2989 + lightColors.y * 0.5870 + lightColors.z * 0.1140;
    color =  vec4(2 * lightIntensity - 0.6 * vec3(wave), 1.0);
//    color = vec4(vec3(arctan),1);
}
#version 430 core

in layout(location = 0) vec3 normal_in;
in layout(location = 1) vec2 textureCoordinates;
in layout(location = 2) vec3 fragPos;
in layout(location = 3) vec3 cameraPos;
in layout(location = 14) mat4 projection_in;
in layout(location = 5) mat4 cameraTransform_in;
//in layout(location = 10) mat4 modelMatrix_in;

uniform sampler2D voronoiTexture;
uniform sampler2D signalTexture;
uniform layout(location = 8) float shinyness;
uniform layout(location = 9) vec2 u_resolution;

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



vec2 hash2(vec2 p) {
    p = vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)));
    return -1.0 + 2.0*fract(sin(p)*43758.5453123);
}

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

    vec3 light1Color = max(0, dot(vecToLight0, normal)) * lightArray[0].color;
    vec3 light2Color = max(0, dot(vecToLight1, normal)) * lightArray[1].color;
    vec3 light3Color = max(0, dot(vecToLight2, normal)) * lightArray[2].color;


    vec3 diffuse = light1Color * L1 + light2Color * L2 + light3Color * L3;



    //Specular lighting
    float angle = degrees(acos(dot(normal, viewDir)));

    // Calculate the specular lighting
    vec3 specular;
    if (angle <= 90.0) 
        specular = pow(max(dot(viewDir, reflect(-vecToLight0, normal)),  0.0), shinyness) * lightArray[0].color * L1
                    + pow(max(dot(viewDir, reflect(-vecToLight1, normal)),  0.0), shinyness) * lightArray[1].color * L2
                    + pow(max(dot(viewDir, reflect(-vecToLight2, normal)),  0.0), shinyness) * lightArray[2].color * L3;

    float dither = dither(textureCoordinates);

    float waveFrequency = 30.0; // Adjust this to change the frequency of the waves
    float waveAmplitude = 0.2; // Adjust this to change the amplitude of the waves

    float noiseFrequency = 1.3; // Adjust this to change the frequency of the noise
    float noiseAmplitude = 0.02; // Adjust this to change the amplitude of the noise

    // voronoi cells for irregular line rotation of waves
    vec4 voronoiColor = texture(voronoiTexture, textureCoordinates.xy*3);
    voronoiColor *= 9;
    voronoiColor = floor(voronoiColor);
    voronoiColor /= 9;
    float voronoiIntensity = voronoiColor.x * 0.2989 + voronoiColor.y * 0.5870 + voronoiColor.z * 0.1140;


    float cosTheta = cos(voronoiIntensity);
    float sinTheta = sin(voronoiIntensity);
    float cosTheta45 = cos(voronoiIntensity-45);
    float sinTheta45 = sin(voronoiIntensity-45);

    mat3 rotationMatrix = mat3(
        cosTheta, -sinTheta, 0,
        sinTheta, cosTheta, 0,
        0, 0, 1
    );
    mat3 rotationMatrix45 = mat3(
        cosTheta45, -sinTheta45, 0,
        sinTheta45, cosTheta45, 0,
        0, 0, 1
    );
               
    vec3 proj = fragPos;
    vec3 proj45 = proj * rotationMatrix;
    vec3 proj45n = proj * rotationMatrix45;


    float tileWidth = 0.25; // Adjust this to change the width of the tiles
    float wave1 = mod(proj45.x, tileWidth) / tileWidth;
    float wave2 = mod(fragPos.y, tileWidth) /tileWidth;
    float wave3 = mod(proj45n.x, 0.2) /0.2;
        
    // adjust the ratio of white-black between the waves
    wave1 += 0.5;
    wave1 = floor(wave1);

    wave2 += 0.3;
    wave2 = floor(wave2);

    wave3 += 0.7;
    wave3 = floor(wave3);

    //Convert the light colors to grayscale for intensity
    vec3 lightColors = vec3(diffuse + specular + dither);
    float lightIntensity = lightColors.x * 0.2989 + lightColors.y * 0.5870 + lightColors.z * 0.1140;
    float wave;

    // Determine which combination of waves to use in the final intentity 
    // based on lightIntensity thresholds
    if(lightIntensity > 0.5)
		wave = 1;
    else if (lightIntensity >= 0.35)
        wave = wave1 ;
    else if(lightIntensity >= 0.20)
		wave =  wave1-wave2 ;
    else if(lightIntensity >= 0.10)
	    wave =  wave1-wave2 - wave3;
    else 
        wave =  0;
           
    color = vec4(vec3(wave), 1.0);
}
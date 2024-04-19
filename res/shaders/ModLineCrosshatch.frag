#version 430 core
in layout(location = 0) vec3 normal_in;
in layout(location = 1) vec2 textureCoordinates;
in layout(location = 2) vec3 fragPos;
in layout(location = 3) vec3 cameraPos;

uniform sampler2D voronoiTexture;
uniform layout(location = 8) float shinyness;

out vec4 color;

struct LightStruct
{
    vec3 position;
    vec3 color;
};

uniform LightStruct lightArray[3]; 


vec2 random(vec2 st) {
    st = vec2( dot(st,vec2(127.1,311.7)),
               dot(st,vec2(269.5,183.3)) );
    return -1.0 + 2.0*fract(sin(st)*43758.5453123);
}

float noise(vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    vec2 u = f*f*(3.0-2.0*f);

    return mix( mix( dot(random(i + vec2(0.0,0.0)), f - vec2(0.0,0.0)),
                     dot(random(i + vec2(1.0,0.0)), f - vec2(1.0,0.0)), u.x),
                mix( dot(random(i + vec2(0.0,1.0)), f - vec2(0.0,1.0)),
                     dot(random(i + vec2(1.0,1.0)), f - vec2(1.0,1.0)), u.x), u.y);
}

// returns a mod line wave rotated about a given angle, with a black-white ratio of thickness and xy direction of 0 or 1.
float makeWave(float angle, float thickness, float frequency, int direction)
{
    float cosTheta = cos(angle);
    float sinTheta = sin(angle);

    mat3 rotationMatrix = mat3(
        cosTheta, -sinTheta, 0,
        sinTheta, cosTheta, 0,
        0, 0, 1
    );
    vec3 rotatedPos = fragPos * rotationMatrix;

    float wave;
    if(direction == 0)
        wave = mod(rotatedPos.x , frequency) /frequency;
    else
        wave = mod(rotatedPos.y, frequency) /frequency;


    wave += thickness;
    wave = floor(wave);
    wave += abs(noise(fragPos.xy*50))*0.5; // noise to simulate graphite pencil texture

    return wave;
}

void main()
{
vec3 normal = normalize(normal_in);
vec3 viewDir = normalize(cameraPos - fragPos);

    // Phong light model to find light intensity for the fragment
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
    vec3 specular;
    if (angle <= 90.0) 
        specular = pow(max(dot(viewDir, reflect(-vecToLight0, normal)),  0.0), shinyness) * lightArray[0].color * L1
                    + pow(max(dot(viewDir, reflect(-vecToLight1, normal)),  0.0), shinyness) * lightArray[1].color * L2
                    + pow(max(dot(viewDir, reflect(-vecToLight2, normal)),  0.0), shinyness) * lightArray[2].color * L3;

    //Convert the light colors to grayscale for intensity
    vec3 lightColors = vec3(diffuse + specular);
    float lightIntensity = lightColors.x * 0.2989 + lightColors.y * 0.5870 + lightColors.z * 0.1140;

    // voronoi cells for irregular line rotation of waves. 
    vec4 voronoiColor = texture(voronoiTexture, textureCoordinates.xy*0.025);
    float voronoiIntensity = voronoiColor.x * 0.2989 + voronoiColor.y * 0.5870 + voronoiColor.z * 0.1140;


    // Determine which combination of waves to use in the final intentity 
    // based on lightIntensity thresholds
    float wave = 0;
    if(lightIntensity > 0.5)
		wave = 1;
    else if (lightIntensity >= 0.35)
        wave = makeWave(voronoiIntensity+0.25, 0.6, 0.15, 0);
    else if(lightIntensity >= 0.20)
		wave = makeWave(voronoiIntensity+0.25, 0.6, 0.15, 0) *
        makeWave(voronoiIntensity, 0.65, 0.2, 1);
    else if(lightIntensity >= 0.10)
    {
	    wave = makeWave(voronoiIntensity+0.25, 0.6, 0.15, 0) *
        makeWave(voronoiIntensity, 0.65, 0.2, 1) *
        makeWave(voronoiIntensity, 0.4, 0.2, 0);
    }
           
    color = vec4(vec3(wave), 1.0);
}
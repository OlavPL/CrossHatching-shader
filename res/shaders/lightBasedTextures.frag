#version 430 core
in layout(location = 0) vec3 normal_in;
in layout(location = 1) vec2 textureCoordinates;
in layout(location = 2) vec3 fragPos;
in layout(location = 3) vec3 cameraPos;
in layout(location = 5) mat4 cameraTransform_in;

uniform sampler2D hatch_light_1;
uniform sampler2D hatch_light_2;
uniform sampler2D hatch_dense;
uniform layout(location = 8) float shinyness;

out vec4 color;

struct LightStruct
{
    vec3 position;
    vec3 color;
};

uniform LightStruct lightArray[3]; 

vec3 getTexture(float angle, float scale, sampler2D tex)
{
    return vec3(texture(tex, fragPos.xy/scale));
}

float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453); }
float dither(vec2 uv) { return (rand(uv)*2.0-1.0) / 256.0; }

void main()
{

    vec3 normal = normalize(normal_in);
    vec3 viewDir = normalize(cameraPos - fragPos);
    vec3 cameraspacePosition = (cameraTransform_in * vec4(fragPos, 1.0)).xyz;


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

    // Determine which combination of waves to use in the final intentity 
    // based on lightIntensity thresholds
    vec3 wave;
    if(lightIntensity > 0.6)
		wave = vec3(1);        
    else if(lightIntensity < 0.10)
        wave =  vec3(0); 

    else 
    {
        wave = getTexture(0.25, 8, hatch_light_1);

        if(lightIntensity <= 0.40)
        {
		    wave *= getTexture(0.25, 8, hatch_light_2);
            wave -= 0.15 - dither(textureCoordinates)*2;
        }

        if(lightIntensity <= 0.25)
        {
            wave *= getTexture(0.2, 8, hatch_dense);
            wave -= 0.5 - dither(textureCoordinates);
        }
    }

    color = vec4(wave, 1.0);
}
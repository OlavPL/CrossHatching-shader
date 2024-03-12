#version 430 core

in layout(location = 0) vec3 normal_in;
in layout(location = 1) vec2 textureCoordinates;
in layout(location = 2) vec3 fragPos;

uniform layout(location = 5) vec3 cameraPos;
uniform layout(location = 8) float shinyness;
uniform layout(location = 13) float ballRadius;

 uniform layout(location = 12) vec3 ballPos;

out vec4 color;

float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453); }
float dither(vec2 uv) { return (rand(uv)*2.0-1.0) / 256.0; }

 vec3 ambientColor = vec3(.1650, .2420, .2430); // nice icy blue

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
    vec3 fragToLight1 = lightArray[1].position - fragPos;
    vec3 fragToLight2 = lightArray[2].position - fragPos;

    vec3 fragToBall = ballPos - fragPos;

    float shadow1 = 1;
    float shadow2 = 1;
    float shadow3 = 1;
    vec3 normalFragToBall = normalize(fragToBall);

    if ( length(reject(fragToBall, fragToLight0)) < ballRadius)
        if( length(fragToBall) < (length(fragToLight0)) || dot(normalFragToBall, normalize(fragToLight0)) < 0.0)
            shadow1 = 0;

    if ( length(reject(fragToBall, fragToLight1)) < ballRadius)
        if( length(fragToBall) < (length(fragToLight1)) || dot(normalFragToBall, normalize(fragToLight1)) < 0.0)
            shadow2 = 0;

    if ( length(reject(fragToBall, fragToLight2)) < ballRadius)
        if( length(fragToBall) < (length(fragToLight2)) || dot(normalFragToBall, normalize(fragToLight2)) < 0.0)
            shadow3 = 0;

    // Soft shadow
    float softShadow1 = 1;
    float softShadow2 = 1;
    float softShadow3 = 1;
    float softShadowRadius = ballRadius*1.4;
    float softShadowWidth = softShadowRadius - ballRadius;

    float softRejectLen0 = length(reject(fragToBall, fragToLight0)); 
    if ( softShadowRadius > softRejectLen0 && softRejectLen0 > ballRadius)
        if( length(fragToBall) < (length(fragToLight0)) || dot(normalFragToBall, normalize(fragToLight0)) < 0.0)
            softShadow1 = (softRejectLen0 - ballRadius) / softShadowWidth;
            
    float softRejectLen1 = length(reject(fragToBall, fragToLight1)); 
    if ( softShadowRadius > softRejectLen1 && softRejectLen1 > ballRadius)
        if( length(fragToBall) < (length(fragToLight1)) || dot(normalFragToBall, normalize(fragToLight1)) < 0.0)
            softShadow2 = (softRejectLen1 - ballRadius) / softShadowWidth;
            
    float softRejectLen2 = length(reject(fragToBall, fragToLight2)); 
    if ( softShadowRadius > softRejectLen2 && softRejectLen2 > ballRadius)
        if( length(fragToBall) < (length(fragToLight2)) || dot(normalFragToBall, normalize(fragToLight2)) < 0.0)
            softShadow3 = (softRejectLen2 - ballRadius) / softShadowWidth;
        

    // Ambient lighting
    vec3 ambient = ambientColor;

    // attenuation
    float d1 = length(fragPos - lightArray[0].position);
    float d2 = length(fragPos - lightArray[1].position);
    float d3 = length(fragPos - lightArray[2].position);
    float la = 0.0001;
    float lb = 0.002;
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

    vec3 diffuse = light1Color * L1 * shadow1 * softShadow1 + light2Color * L2 * shadow2  * softShadow2 + light3Color * L3 * shadow3 * softShadow3;



    //Specular lighting
    vec3 specular = pow(max(dot(viewDir, reflect(-vecToLight0, normal)),  0.0), shinyness) * lightArray[0].color * L1 * shadow1
                 + pow(max(dot(viewDir, reflect(-vecToLight1, normal)),  0.0), shinyness) * lightArray[1].color * L2 * shadow2
                 + pow(max(dot(viewDir, reflect(-vecToLight2, normal)),  0.0), shinyness) * lightArray[2].color * L3 * shadow3;

    float dither = dither(textureCoordinates);
    color = vec4(ambient + diffuse + specular + dither, 1.0f);
}
#version 430 core

in layout(location = 0) vec3 normal_in;
in layout(location = 1) vec2 textureCoordinates;
in layout(location = 2) vec3 fragPos;
in layout(location = 3) vec3 cameraPos;
in layout(location = 14) mat4 projection_in;
in layout(location = 5) mat4 cameraTransform_in;
in layout(location = 10) mat4 modelMatrix_in;

uniform layout(location = 15) sampler2D voronoiTexture;
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

vec2 voronoi(vec2 x) {
    vec2 n = floor(x);
    vec2 f = fract(x);

    //----------------------------------
    // Four corners pattern: 
    //----------------------------------
    vec2 mg, mr;

    float md = 8.0;
    for (int j=-1; j<=1; j++)
    for (int i=-1; i<=1; i++) {
        vec2 g = vec2(float(i),float(j));
        vec2 o = hash2(n + g);
        vec2 r = g - f + o;
        float d = dot(r,r);

        if (d<md) {
            md = d;
            mr = r;
            mg = g;
        }
    }

    return vec2(sqrt(md), mr.x + mr.y * 0.1);
}

vec3 hash3(vec2 p) {
    vec2 pp = p * vec2(p.y, p.y + 1.0) * vec2(1.6, 1.7);
    vec3 q = vec3(dot(pp, vec2(127.1, 311.7)), 
                  dot(pp, vec2(269.5, 183.3)), 
                  dot(pp, vec2(419.2, 371.9)));
    return fract(sin(q) * 43758.5453);
}

vec3 permute(vec3 x) { return mod(((x*34.0)+1.0)*x, 289.0); }

float snoise(vec2 v) {
    const vec4 C = vec4(0.211324865405187, 0.366025403784439, -0.577350269189626, 0.024390243902439);
    vec2 i  = floor(v + dot(v, C.yy));
    vec2 x0 = v - i + dot(i, C.xx);
    vec2 i1;
    i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;
    i = mod(i, 289.0);
    vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0)) + i.x + vec3(0.0, i1.x, 1.0));
    vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
    m = m*m;
    m = m*m;
    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;
    m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );
    vec3 g;
    g.x  = a0.x  * x0.x  + h.x  * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    return 130.0 * dot(m, g);
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

//    vec2 voronoi = voronoi(fragPos.xy);
//    voronoi *= 3.0;

    // Generate Voronoi diagram
    vec2 voronoiResult = voronoi(vec2(fragPos.x* 0.00000005,fragPos.y* 0.000000189)); // Multiply by 10.0 to scale the Voronoi cells

    // Use the cell index to create a color
    vec3 voronoiColor = hash3(voronoiResult);
    // convert voronoiColor to grayscale 
    float voronoiIntensity = voronoiColor.x * 0.2989 + voronoiColor.y * 0.5870 + voronoiColor.z * 0.1140;
    voronoiColor = vec3(voronoiIntensity);

//    voronoiColor *= 4;
//    voronoiColor = floor(voronoiColor);
//    voronoiColor /= 4;

    // Output the color

    float cosTheta = cos(voronoiIntensity);
    float sinTheta = sin(voronoiIntensity);

    mat3 rotationMatrix = mat3(
        cosTheta, -sinTheta, 0,
        sinTheta, cosTheta, 0,
        0, 0, 1
    );
    vec4 fragPosCameraSpace = vec4(fragPos, 1.0f); // Assuming fragPos is a glm::vec3
    mat4 viewMatrixInverse = inverse(cameraTransform_in); // Assuming cameraTransform is your view matrix
    vec3 fragPosWorldSpace = vec3(viewMatrixInverse * fragPosCameraSpace);
                
    vec3 proj = fragPos;
//    proj *= rotationMatrix;
//
    float noise = (voronoi(fragPos.xy).y);
    float tileWidth = 0.2 + (noise * 0.007); // Adjust this to change the width of the tiles
    float wave = mod(proj.x, tileWidth) / tileWidth; // y is now in the range [0, 1] and repeats every tileWidth units
    
    // TODO: trying to add width noise to the black lines. Or just make em more dense,
    // and only shade by changing the intensity of the line.
    bool addNoise = false;
    if(wave > 0.3 || wave < 0.95) {
		addNoise = true;
	}
//    wave += 0.25 + snoise(fragPos.xy) * 0.05;
    wave += 0.25;
    wave = floor(wave);

    vec3 lightColors = vec3(diffuse + specular + dither);
    float lightIntensity = lightColors.x * 0.2989 + lightColors.y * 0.5870 + lightColors.z * 0.1140;
    
    color =  vec4(2 * lightIntensity - 0.6 * vec3(wave), 1.0);
    
//    color = vec4(vec3(voronoiColor), 1.0);
    //color from voronoiTexture
    
    color = vec4(vec3(voronoiColor), 1.0);
    color = vec4(vec3(texture(voronoiTexture, textureCoordinates*5)), 1.0);
}
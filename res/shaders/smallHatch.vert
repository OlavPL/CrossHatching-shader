#version 430 core

in layout(location = 0) vec3 position;
in layout(location = 1) vec3 normal_in;
in layout(location = 2) vec2 textureCoordinates_in;


uniform layout(location = 3) mat4 modelMatrix;
uniform layout(location = 7) mat3 normalMatrix;

uniform layout(location = 4) mat4 projection;

out layout(location = 0) vec3 normal_out;
out layout(location = 1) vec2 textureCoordinates_out;
out layout(location = 2) vec4 vertexPos_in;

void main()
{
    normal_out = normalize( normalMatrix * normal_in);

    textureCoordinates_out = textureCoordinates_in;

    vertexPos_in = modelMatrix * vec4(position, 1.0f);
    gl_Position = projection * modelMatrix * vec4(position, 1.0f);

    
}
